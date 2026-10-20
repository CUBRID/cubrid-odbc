#!/bin/bash
#
#  Copyright 2016 CUBRID Corporation
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
# Install CUBRID into a freshly imported WSL2 distribution and start it, so
# that windows-odbc-test.yml has the same Linux server the Linux workflow
# runs against.  Runs as root inside the distribution:
#
#   wsl -d <distro> -u root -- bash <this script> <engine tarball url> [db] [md5]
#
# The distribution is a plain Rocky Linux 8 container rootfs, the same base
# the cubrid-docker images are built FROM, and the steps below are the ones
# cubrid-wsl-installer performs after `wsl --import`: lay down the cubrid
# user, unpack the engine, write ~/.cubrid.sh and create demodb.
#
set -eux

CUBRID_URL="$1"
DB_NAME="${2:-demodb}"
CUBRID_MD5="${3:--}"

echo "========== packages =========="
# the engine needs ncurses and libnsl; the rest is what the steps below use
dnf -y install --setopt=install_weak_deps=False \
    tar gzip ncurses libnsl iproute procps-ng 2>&1 | tail -5

echo "========== cubrid user =========="
getent group cubrid  >/dev/null || groupadd -r -g 1000 cubrid
getent passwd cubrid >/dev/null || useradd -r -u 1000 -g cubrid -d /home/cubrid -m cubrid
# WSL mounts /tmp/.X11-unix read only, so this cannot be the recursive chmod
# the Dockerfiles use
chmod 1777 /tmp

echo "========== download $CUBRID_URL =========="
curl -fsSL -o /tmp/cubrid.tar.gz "$CUBRID_URL"

if [ "$CUBRID_MD5" != "-" ]; then
    echo "========== verify md5 =========="
    command -v md5sum >/dev/null || dnf -y install --setopt=install_weak_deps=False coreutils-single 2>&1 | tail -5
    echo "$CUBRID_MD5  /tmp/cubrid.tar.gz" | md5sum -c -
else
    echo "========== no md5 published for this package, skipping =========="
fi

tar -xzf /tmp/cubrid.tar.gz -C /home/cubrid
rm -f /tmp/cubrid.tar.gz

# the environment the cubrid-wsl-installer image overlay writes, so that
# every `wsl -u cubrid` call finds CUBRID the same way the installer does
cat > /home/cubrid/.cubrid.sh <<'EOF'
export CUBRID=/home/cubrid/CUBRID
export CUBRID_DATABASES=$CUBRID/databases

LD_LIBRARY_PATH=$CUBRID/lib:$CUBRID/cci/lib:$LD_LIBRARY_PATH
PATH=$CUBRID/bin:/usr/sbin:$PATH
export LD_LIBRARY_PATH SHLIB_PATH LIBPATH PATH
EOF
chmod 755 /home/cubrid/.cubrid.sh

if ! grep -q 'cubrid.sh' /home/cubrid/.bash_profile 2>/dev/null; then
    echo '. /home/cubrid/.cubrid.sh' >> /home/cubrid/.bash_profile
fi

mkdir -p /home/cubrid/CUBRID/databases
chown -R cubrid:cubrid /home/cubrid

su - cubrid -c '. ~/.cubrid.sh; cubrid_rel'

# the engine tarball ships no database, so demodb is created here with the
# DEMO data the testcases need (event, game, ...)
echo "========== create $DB_NAME =========="
su - cubrid -c ". ~/.cubrid.sh; cd \$CUBRID_DATABASES && \$CUBRID/demo/make_cubrid_demo.sh"
su - cubrid -c '. ~/.cubrid.sh; cat $CUBRID_DATABASES/databases.txt'
su - cubrid -c ". ~/.cubrid.sh; grep -qw '^$DB_NAME' \$CUBRID_DATABASES/databases.txt"

echo "========== start the server and the broker =========="
su - cubrid -c ". ~/.cubrid.sh; cubrid server start $DB_NAME"
su - cubrid -c '. ~/.cubrid.sh; cubrid broker start'
su - cubrid -c '. ~/.cubrid.sh; cubrid service status' || true

# the brokers listen on 0.0.0.0, which is what lets WSL forward the Windows
# side of localhost to them
echo "========== listening ports =========="
ss -ltn

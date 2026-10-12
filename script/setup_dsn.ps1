#
# Register the ODBC data sources required by the CUBRID ODBC unit tests.
#
# Creates four 64bit DSNs (System scope by default):
#   CUBRID Driver Unicode      AUTOCOMMIT=on
#   cubrid_autocommit_on       AUTOCOMMIT=on
#   cubrid_autocommit_off      AUTOCOMMIT=off
#   cubrid_autocommit_unknown  AUTOCOMMIT=     (empty: driver falls back to ON)
#
# A DSN that is already registered is left untouched unless -Force is given.
#
# -DriverDll names the driver build to test.  A DSN carries that path and loads
# it whoever runs this, but a DRIVER=<name> connection string resolves the name
# through HKLM\SOFTWARE\ODBC\ODBCINST.INI, which ODBC keeps machine wide:
#
#   driver not registered yet  the build is registered there  (needs elevation)
#   registered, same path      nothing to do
#   registered, other path     elevated: re-registered to the build
#                              otherwise: left alone, and the run warns that
#                              DRIVER= strings still use the installed driver
#
# Usage:
#   powershell -ExecutionPolicy Bypass -File script\setup_dsn.ps1
#   powershell -ExecutionPolicy Bypass -File script\setup_dsn.ps1 -Force
#   powershell -ExecutionPolicy Bypass -File script\setup_dsn.ps1 -Scope User
#   powershell -ExecutionPolicy Bypass -File script\setup_dsn.ps1 -Server 192.168.0.10
#   powershell -ExecutionPolicy Bypass -File script\setup_dsn.ps1 -DriverOnly `
#       -DriverName 'CUBRID Driver' -DriverDll <path>\cubrid_odbc64.dll
#
[CmdletBinding()]
param(
    [ValidateSet('System', 'User')][string] $Scope = 'System',
    [string] $DriverName = 'CUBRID Driver Unicode',
    [string] $DriverDll  = '',
    [string] $Server     = 'test-db-server',
    [string] $Port       = '33000',
    [string] $DbName     = 'demodb',
    [string] $User       = 'dba',
    [string] $Password   = '',
    [string] $Charset    = 'utf8',
    [string] $FetchSize  = '100',
    [string] $OmitSchema = 'no',
    [switch] $Force,
    # register the 64bit driver only, for the ANSI driver which the tests reach
    # through a DRIVER= connection string instead of a DSN
    [switch] $DriverOnly,
    # write what this run is about to change into this file, so that a test run
    # can hand the machine back the way it found it
    [string] $Backup = '',
    # put such a file back and do nothing else
    [string] $Restore = ''
)

$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------- definitions

$DSN_LIST = @(
    @{ Name = 'CUBRID Driver Unicode';     AutoCommit = 'on';  Description = 'CUBRID Unicode' },
    @{ Name = 'cubrid_autocommit_on';      AutoCommit = 'on';  Description = 'cubrid_autocommit_on' },
    @{ Name = 'cubrid_autocommit_off';     AutoCommit = 'off'; Description = 'cubrid_autocommit_off' },
    @{ Name = 'cubrid_autocommit_unknown'; AutoCommit = '';    Description = 'cubrid_autocommit_unknown' }
)

$ODBCINST_PATH = 'SOFTWARE\ODBC\ODBCINST.INI'
$ODBCINI_PATH  = 'SOFTWARE\ODBC\ODBC.INI'
$SOURCES_KEY   = "$ODBCINI_PATH\ODBC Data Sources"

# ------------------------------------------------------------------- registry
#
# Always use the 64bit registry view so that the DSNs land in
# HKLM\SOFTWARE\ODBC (64bit) and never under WOW6432Node, even when this
# script happens to be run by a 32bit powershell.exe.

function Get-BaseKey
{
    param([Microsoft.Win32.RegistryHive] $Hive)
    return [Microsoft.Win32.RegistryKey]::OpenBaseKey($Hive, [Microsoft.Win32.RegistryView]::Registry64)
}

function Get-RegValue
{
    param([Microsoft.Win32.RegistryKey] $Base, [string] $Path, [string] $Name)

    $key = $Base.OpenSubKey($Path)
    if ($null -eq $key) { return $null }
    try { return $key.GetValue($Name) } finally { $key.Close() }
}

function Set-RegValue
{
    param(
        [Microsoft.Win32.RegistryKey] $Base,
        [string] $Path,
        [string] $Name,
        $Value,
        [Microsoft.Win32.RegistryValueKind] $Kind = [Microsoft.Win32.RegistryValueKind]::String
    )

    $key = $Base.CreateSubKey($Path)
    if ($null -eq $key) { throw "cannot open or create registry key: $Path" }
    try { $key.SetValue($Name, $Value, $Kind) } finally { $key.Close() }
}

function Test-Administrator
{
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    return (New-Object Security.Principal.WindowsPrincipal($id)).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Remove-RegValueIfAny
{
    param([Microsoft.Win32.RegistryKey] $Base, [string] $Path, [string] $Name)

    if ($null -eq (Get-RegValue $Base $Path $Name)) { return }
    $key = $Base.OpenSubKey($Path, $true)
    if ($null -eq $key) { return }
    try { $key.DeleteValue($Name, $false) } finally { $key.Close() }
}

function Remove-RegKeyIfAny
{
    param([Microsoft.Win32.RegistryKey] $Base, [string] $Path)

    $key = $Base.OpenSubKey($Path)
    if ($null -eq $key) { return }
    $key.Close()
    $Base.DeleteSubKeyTree($Path)
}

function Test-RegKey
{
    param([Microsoft.Win32.RegistryKey] $Base, [string] $Path)

    $key = $Base.OpenSubKey($Path)
    if ($null -eq $key) { return $false }
    $key.Close()
    return $true
}

# ----------------------------------------------------------- saved state
#
# -Backup records what a run is about to change and -Restore puts it back, so
# that test.bat can point the DSNs at the build it just made without leaving
# the machine that way.

function Save-OdbcState
{
    param([string] $File)

    $driverKey = "$ODBCINST_PATH\$DriverName"
    $driverDll = Get-RegValue $hklm $driverKey 'Driver'

    $dsn = [ordered]@{}
    if (-not $DriverOnly) {
        foreach ($d in $DSN_LIST) {
            $path = "$ODBCINI_PATH\$($d.Name)"
            $dsn[$d.Name] = [ordered]@{
                Existed = (Test-RegKey $base $path)
                Driver  = [string] (Get-RegValue $base $path 'Driver')
            }
        }
    }

    $state = [ordered]@{
        Scope      = $Scope
        DriverName = $DriverName
        Driver     = [ordered]@{
            Existed = ($null -ne $driverDll)
            Driver  = [string] $driverDll
            Setup   = [string] (Get-RegValue $hklm $driverKey 'Setup')
        }
        Dsn        = $dsn
    }

    $state | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $File -Encoding utf8
    Write-Host "[backup]  previous ODBC registration saved to $File"
}

function Restore-OdbcState
{
    param([string] $File)

    if (-not (Test-Path -LiteralPath $File)) {
        Write-Host "[restore] nothing to put back, $File does not exist"
        return
    }

    $state = Get-Content -LiteralPath $File -Raw | ConvertFrom-Json
    $hive = if ($state.Scope -eq 'System') { [Microsoft.Win32.RegistryHive]::LocalMachine }
            else { [Microsoft.Win32.RegistryHive]::CurrentUser }

    $dsnBase = Get-BaseKey $hive
    $machine = Get-BaseKey ([Microsoft.Win32.RegistryHive]::LocalMachine)

    try {
        $p = "$ODBCINST_PATH\$($state.DriverName)"
        if ($state.Driver.Existed) {
            # only written back when this run actually moved it, so that a run
            # without elevation does not try to write HKLM at all
            if ((Get-RegValue $machine $p 'Driver') -ne $state.Driver.Driver) {
                Set-RegValue $machine $p 'Driver' $state.Driver.Driver
                if (-not [string]::IsNullOrEmpty($state.Driver.Setup)) {
                    Set-RegValue $machine $p 'Setup' $state.Driver.Setup
                }
                Write-Host ("[restore] '{0}' -> {1}" -f $state.DriverName, $state.Driver.Driver)
            }
        }
        else {
            if (Test-RegKey $machine $p) {
                Remove-RegKeyIfAny $machine $p
                Remove-RegValueIfAny $machine "$ODBCINST_PATH\ODBC Drivers" $state.DriverName
                Write-Host ("[restore] '{0}' unregistered again" -f $state.DriverName)
            }
        }

        foreach ($entry in $state.Dsn.PSObject.Properties) {
            $name = $entry.Name
            $saved = $entry.Value
            $path = "$ODBCINI_PATH\$name"

            if ($saved.Existed) {
                if ((Get-RegValue $dsnBase $path 'Driver') -ne $saved.Driver) {
                    Set-RegValue $dsnBase $path 'Driver' $saved.Driver
                    Write-Host ("[restore] DSN {0} driver -> {1}" -f $name, $saved.Driver)
                }
            }
            elseif (Test-RegKey $dsnBase $path) {
                Remove-RegKeyIfAny $dsnBase $path
                Remove-RegValueIfAny $dsnBase $SOURCES_KEY $name
                Write-Host ("[restore] DSN {0} removed again" -f $name)
            }
        }
    }
    finally {
        $dsnBase.Close()
        $machine.Close()
    }

    Remove-Item -LiteralPath $File -Force
}

# ---------------------------------------------------------------------- start

if (-not [string]::IsNullOrEmpty($Restore)) {
    Restore-OdbcState $Restore
    exit 0
}

if ($Scope -eq 'System') {
    $hive = [Microsoft.Win32.RegistryHive]::LocalMachine
    if (-not (Test-Administrator)) {
        Write-Error "System DSNs are written to HKLM; run this script from an elevated prompt, or use -Scope User."
        exit 1
    }
}
else {
    $hive = [Microsoft.Win32.RegistryHive]::CurrentUser
}

$hklm = Get-BaseKey ([Microsoft.Win32.RegistryHive]::LocalMachine)
$base = Get-BaseKey $hive

try {
    # -- 0. record what is there now, before anything changes -----------------

    if (-not [string]::IsNullOrEmpty($Backup)) {
        Save-OdbcState $Backup
    }

    # -- 1. make sure the 64bit ODBC driver itself is registered --------------

    $registeredDll = Get-RegValue $hklm "$ODBCINST_PATH\$DriverName" 'Driver'

    # what the caller asked for, before falling back to whatever is registered:
    # a driver given by hand is a request to test that build, not the installed one
    $requestedDll = $DriverDll

    if ([string]::IsNullOrEmpty($DriverDll)) {
        $DriverDll = $registeredDll
    }

    if ([string]::IsNullOrEmpty($DriverDll)) {
        # not registered yet: look for an installed / freshly built 64bit dll
        $candidates = @(
            "${env:ProgramFiles(x86)}\cubrid-odbc-unicode\cubrid_odbc64.dll",
            "$env:ProgramFiles\cubrid-odbc-unicode\cubrid_odbc64.dll",
            (Join-Path $PSScriptRoot '..\output_unicode\cubrid_odbc64.dll')
        )
        foreach ($c in $candidates) {
            if (Test-Path -LiteralPath $c) { $DriverDll = (Resolve-Path -LiteralPath $c).Path; break }
        }
    }

    if ([string]::IsNullOrEmpty($DriverDll)) {
        Write-Error "ODBC driver '$DriverName' is not registered and cubrid_odbc64.dll could not be found. Install the driver, or pass -DriverDll <path>."
        exit 1
    }

    if (-not (Test-Path -LiteralPath $DriverDll)) {
        Write-Error "driver dll does not exist: $DriverDll"
        exit 1
    }

    # compare and store one spelling of the path, so that a relative -DriverDll
    # does not look like a different driver than the registered one
    $DriverDll = [System.IO.Path]::GetFullPath((Resolve-Path -LiteralPath $DriverDll).Path)

    if ($null -eq $registeredDll) {
        if (-not (Test-Administrator)) {
            Write-Error "ODBC driver '$DriverName' is not registered; registering it requires an elevated prompt."
            exit 1
        }
        Write-Host "[driver] registering 64bit driver '$DriverName' -> $DriverDll"
        $p = "$ODBCINST_PATH\$DriverName"
        Set-RegValue $hklm $p 'Driver'           $DriverDll
        Set-RegValue $hklm $p 'Setup'            $DriverDll
        Set-RegValue $hklm $p 'APILevel'         '2'
        Set-RegValue $hklm $p 'DriverODBCVer'    '10.0.0(64)'
        Set-RegValue $hklm $p 'ConnectFunctions' 'YYN'
        Set-RegValue $hklm $p 'CreateDSN'        'CUBRID Driver 64'
        Set-RegValue $hklm $p 'SQLLevel'         '0'
        # keep the value name used by the NSIS installer (installer-unicode.nsi)
        Set-RegValue $hklm $p 'UsageCoun'        1 ([Microsoft.Win32.RegistryValueKind]::DWord)
        Set-RegValue $hklm "$ODBCINST_PATH\ODBC Drivers" $DriverName 'Installed'
    }
    elseif (-not [string]::IsNullOrEmpty($requestedDll) -and $registeredDll -ne $DriverDll) {
        # a different driver was asked for than the one that is registered.  The
        # DSNs below carry their own path and will load it, but a DRIVER=<name>
        # connection string resolves the name through HKLM instead, which only
        # an elevated run can change.
        if (Test-Administrator) {
            Write-Host "[driver] re-registering '$DriverName' -> $DriverDll"
            Write-Host "         (was $registeredDll)"
            $p = "$ODBCINST_PATH\$DriverName"
            Set-RegValue $hklm $p 'Driver' $DriverDll
            Set-RegValue $hklm $p 'Setup'  $DriverDll
        }
        else {
            Write-Host "[driver] '$DriverName' is registered as $registeredDll"
            Write-Warning @"
  the DSNs will load $DriverDll,
  but DRIVER=$DriverName connection strings keep using the registered one.
  Run this from an elevated prompt to test the build end to end.
"@
        }
    }
    else {
        Write-Host "[driver] '$DriverName' already registered -> $registeredDll"
        if ($registeredDll -notmatch '64\.dll$') {
            Write-Warning "registered driver does not look like the 64bit build; the x64 unit tests may fail to load it."
        }
    }

    if ($DriverOnly) {
        Write-Host "[dsn]    skipped, -DriverOnly is given"
        exit 0
    }

    # -- 2. create the DSNs that are missing ---------------------------------

    $created = 0
    $skipped = 0
    $updated = 0

    foreach ($dsn in $DSN_LIST) {
        $name = $dsn.Name
        $path = "$ODBCINI_PATH\$name"

        $values = [ordered]@{
            'Driver'      = $DriverDll
            'DB_NAME'     = $DbName
            'DESCRIPTION' = $dsn.Description
            'UID'         = $User
            'PWD'         = $Password
            'SERVER'      = $Server
            'PORT'        = $Port
            'FETCH_SIZE'  = $FetchSize
            'CHARSET'     = $Charset
            'AUTOCOMMIT'  = $dsn.AutoCommit
            'OMIT_SCHEMA' = $OmitSchema
        }

        $exists = ($null -ne $base.OpenSubKey($path))

        if ($exists -and -not $Force) {
            # already registered: leave it alone, but report anything that
            # differs from what the unit tests expect
            Write-Host ("[dsn]    {0} : already registered, skipped" -f $name)
            $diff = @()
            foreach ($k in $values.Keys) {
                # Driver is not a setting of this DSN to keep, it is which build
                # the DSN loads, and it is repaired below
                if ($k -eq 'Driver') { continue }
                $cur = Get-RegValue $base $path $k
                if ($null -eq $cur) { $cur = '' }
                if ([string]$cur -ne [string]$values[$k]) {
                    $diff += ("{0}='{1}' (expected '{2}')" -f $k, $cur, $values[$k])
                }
            }
            if ($diff.Count -gt 0) {
                Write-Warning ("  {0} differs: {1}" -f $name, ($diff -join ', '))
            }
            $skipped++
        }
        else {
            foreach ($k in $values.Keys) {
                Set-RegValue $base $path $k $values[$k]
            }
            if ($exists) {
                Write-Host ("[dsn]    {0} : updated (-Force)" -f $name)
                $updated++
            }
            else {
                Write-Host ("[dsn]    {0} : created" -f $name)
                $created++
            }
        }

        # the index entry is cheap to repair, so always make sure it is correct
        $indexed = Get-RegValue $base $SOURCES_KEY $name
        if ($indexed -ne $DriverName) {
            Set-RegValue $base $SOURCES_KEY $name $DriverName
        }

        # whatever else a DSN keeps, it has to load the driver under test, so
        # this one value is repaired even for a DSN that was left alone
        if ((Get-RegValue $base $path 'Driver') -ne $DriverDll) {
            Set-RegValue $base $path 'Driver' $DriverDll
            Write-Host ("           driver -> {0}" -f $DriverDll)
        }
    }

    Write-Host ""
    Write-Host ("{0} DSN(s): {1} created, {2} updated, {3} skipped  [scope={4}, 64bit]" -f
        $DSN_LIST.Count, $created, $updated, $skipped, $Scope)
    exit 0
}
finally {
    $base.Close()
    $hklm.Close()
}

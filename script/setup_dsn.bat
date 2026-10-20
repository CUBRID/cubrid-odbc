@echo off
rem Register the ODBC data sources used by the unit tests (see setup_dsn.ps1).
rem Any argument is forwarded to the script, e.g. "setup_dsn.bat -Force".
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0setup_dsn.ps1" %*
exit /b %ERRORLEVEL%

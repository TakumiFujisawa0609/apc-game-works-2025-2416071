@echo off
setlocal

set VERSION_FILE=version.txt

if not exist %VERSION_FILE% (
    echo 0.0.0 > %VERSION_FILE%
)

set /p VERSION=<%VERSION_FILE%
for /f "tokens=1,2,3 delims=." %%a in ("%VERSION%") do (
    set MAJOR=%%a
    set MINOR=%%b
    set BUILD=%%c
)

set /a BUILD=%BUILD% + 1
set NEW_VERSION=%MAJOR%.%MINOR%.%BUILD%

echo %NEW_VERSION% > %VERSION_FILE%
echo Version updated: %VERSION% → %NEW_VERSION%

git tag v%NEW_VERSION%
git push origin v%NEW_VERSION%

endlocal
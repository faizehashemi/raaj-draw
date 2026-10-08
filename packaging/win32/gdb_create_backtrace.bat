@echo off
rem Execute this to create a debug backtrace of a Raaj Draw crash.

set TRACEFILE="%USERPROFILE%\raajdraw_backtrace.txt"

echo Thanks for creating a debug backtrace!
echo.
echo After Raaj Draw starts, try to force the crash.
echo The backtrace will be recorded automatically.
echo.
echo Gathering system info...

echo --- RAAJ DRAW VERSION --- > %TRACEFILE%
raajdraw.com --debug-info >> %TRACEFILE%
echo. >> %TRACEFILE%
echo --- SYSTEM INFO --- >> %TRACEFILE%
systeminfo >> %TRACEFILE%

echo.
echo Launching Raaj Draw, please wait...

echo. >> %TRACEFILE%
echo --- BACKTRACE --- >> %TRACEFILE%
gdb.exe -batch -ex "run --app-id-tag gdbbt" -ex "bt" raajdraw.exe >> %TRACEFILE%

echo.
echo Backtrace written to %TRACEFILE%
echo Please attach this file when reporting the issue at https://raajsoftware.com/contact
echo (remove personal information you do not want to share, e.g. your user name)
echo.

pause

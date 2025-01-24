@echo on
SET LOG_LEVEL_CONSOLE=info
SET LOG_CATEGORY_DISABLED=
SET LOG_MAX_FILES=14d
SET PORT=6969

REM Launch HITMAN3.exe in the background (no visible CMD window)
echo Starting HITMAN3.exe...
start /min "" .\Retail\HITMAN3.exe %*
IF ERRORLEVEL 1 (
    echo [ERROR] Failed to start HITMAN3.exe.
    PAUSE
    EXIT /B 1
)

REM Launch PeacockPatcher.exe in the background (no visible CMD window)
echo Starting PeacockPatcher.exe...
start /min "" PeacockPatcher.exe
IF ERRORLEVEL 1 (
    echo [ERROR] Failed to start PeacockPatcher.exe.
    PAUSE
    EXIT /B 1
)

REM Launch server.cmd in a separate window (visible window)
echo Starting server.cmd...
start "Server" cmd /k server.cmd
IF ERRORLEVEL 1 (
    echo [ERROR] Failed to start server.cmd.
    PAUSE
    EXIT /B 1
)

REM Keep the console window open for user interaction
echo All tasks completed. Press any key to exit.
PAUSE

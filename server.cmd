@echo off

SET LOG_LEVEL_CONSOLE=info
SET LOG_CATEGORY_DISABLED=
SET LOG_MAX_FILES=14d
start Launcher.exe %*
start PeacockPatcher.exe 
pushd "%~dp0"
.\nodedist\node.exe chunk0.js
PAUSE

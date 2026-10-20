@echo off
setlocal
cd /d "%~dp0"
call build_embedded.bat
if errorlevel 1 exit /b 1
if not defined NOICE_EXE set "NOICE_EXE=C:\Program Files (x86)\NoICE\bin\NoICE12.exe"
if not exist "%NOICE_EXE%" (
  echo [ERROR] NoICE not found. Set NOICE_EXE to the path of NoICE12.exe.
  exit /b 1
)
if not exist "build\9s12\buttonbox_debug.noi" exit /b 1
start "" "%NOICE_EXE%" "%CD%\build\9s12\buttonbox_debug.noi"
exit /b 0

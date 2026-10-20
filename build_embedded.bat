@echo off
setlocal
cd /d "%~dp0"
python tools\build_9s12.py
exit /b %errorlevel%

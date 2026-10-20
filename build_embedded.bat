@echo off
setlocal
cd /d "%~dp0"
python Tools\build_9s12.py
exit /b %errorlevel%

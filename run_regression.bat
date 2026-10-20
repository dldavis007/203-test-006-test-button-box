@echo off
setlocal
cd /d "%~dp0"
python tests\run_regression.py
exit /b %errorlevel%

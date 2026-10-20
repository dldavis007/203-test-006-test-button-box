@echo off
setlocal
cd /d "%~dp0"
python Tests\run_regression.py
exit /b %errorlevel%

@echo off

REM Check if python is installed
python --version >nul 2>&1
if %errorlevel% NEQ 0 (
    echo Python is not installed. Please install Python to continue.
    exit /b 1
)

REM Check if at least one argument is provided
if "%~1"=="" (
    echo Usage: build.bat clean^|generate^|build^|clang_format [configuration]
    exit /b 1
)

REM Prepare command line arguments for python script
set ACTION=%~1
set CONFIGURATION=%~2

set CMD=python builder_scripts/builder_script.py %ACTION%

if not "%CONFIGURATION%"=="" (
    set CMD=%CMD% --config %CONFIGURATION%
)

REM Execute the command
%CMD%

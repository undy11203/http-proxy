@echo off

REM This script only calls build.bat with predefined arguments for generation

call builder_scripts/build.bat generate Debug
call builder_scripts/build.bat generate Release

@echo off

REM This script calls build.bat with predefined arguments for building the project

call builder_scripts/build.bat build Debug
call builder_scripts/build.bat build Release

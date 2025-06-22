@echo off

REM Проверка наличия аргумента
if "%~1"=="" (
    echo Usage: build.bat own^|conan-io
    exit /b 1
)

REM Выбор режима
if /i "%~1"=="own" (
    call builder_scripts/build.bat generate Debug
    call builder_scripts/build.bat generate Release
) else if /i "%~1"=="conan-io" (
    cmake -B build -S . -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="conan_provider.cmake" -DCMAKE_BUILD_TYPE=Release
) else (
    echo Invalid argument. Use "own" or "conan-io".
    exit /b 1
)
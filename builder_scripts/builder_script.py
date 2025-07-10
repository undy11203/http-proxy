import argparse
import os
import shutil
import subprocess
from enum import Enum

class Platform(Enum):
    X64 = "x64"
    WIN32 = "Win32"

class Configuration(Enum):
    Debug = "Debug"
    Release = "Release"

class Action(Enum):
    CLEAN = "clean"
    GENERATE = "generate"
    BUILD = "build"
    CLANG_FORMAT = "clang_format"

##################### manual configuration ####################

class Config:
    BUILD_FOLDER = "build"
    CMAKE_GENERATOR = "Visual Studio 17 2022" # https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html
    PLATFORM = Platform.X64  # Platform.WIN32
    FRESH = True
    CLEAN = True
    VERBOSE = False
    SOURCE_DIR = "."

###############################################################

FRESH_ARG = "--fresh" if Config.FRESH else ""
CLEAN_ARG = "--clean-first" if Config.CLEAN else ""
VERBOSE_ARG = "--verbose" if Config.VERBOSE else ""

def run_command(command):
    print(f"Running command: {' '.join(command)}")
    result = subprocess.run(command)
    if result.returncode != 0:
        raise RuntimeError(f"Command {' '.join(command)} failed with exit code {result.returncode}")

def clean_build_folder():
    if os.path.exists(Config.BUILD_FOLDER):
        if Config.VERBOSE:
            print(f"Removing build folder: {Config.BUILD_FOLDER}")
        shutil.rmtree(Config.BUILD_FOLDER)
    else:
        if Config.VERBOSE:
            print(f"Build folder {Config.BUILD_FOLDER} does not exist, nothing to clean.")

def generate_build_files():
    os.makedirs(Config.BUILD_FOLDER, exist_ok=True)
    conan_provider_path = os.path.abspath("conan_provider.cmake")
    cmake_command = [
        "cmake",
        "-S", Config.SOURCE_DIR,
        "-B", Config.BUILD_FOLDER,
        "-G", Config.CMAKE_GENERATOR,
        "-A", Config.PLATFORM.value,
        f'-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES={conan_provider_path}'
    ]
    if Config.VERBOSE:
        print(f"Generating build files with command: {' '.join(cmake_command)}")
    run_command(cmake_command)

def build_project():
    cmake_build_command = [
        "cmake",
        "--build", Config.BUILD_FOLDER,
        "--config", Configuration.Release.value
    ]
    if Config.VERBOSE:
        print(f"Building project with command: {' '.join(cmake_build_command)}")
    run_command(cmake_build_command)

def run_clang_format():
    # Find all source files in SOURCE_DIR with extensions .cpp, .h, .hpp, .c
    extensions = [".cpp", ".h", ".hpp", ".c"]
    files_to_format = []
    for root, _, files in os.walk(Config.SOURCE_DIR):
        for file in files:
            if any(file.endswith(ext) for ext in extensions):
                files_to_format.append(os.path.join(root, file))
    if not files_to_format:
        if Config.VERBOSE:
            print("No source files found for clang-format.")
        return
    clang_format_command = ["clang-format", "-i"] + files_to_format
    if Config.VERBOSE:
        print(f"Running clang-format on {len(files_to_format)} files.")
    run_command(clang_format_command, Config.VERBOSE)

def main():
    parser = argparse.ArgumentParser(description="Build script for the project.")
    parser.add_argument("action", choices=[action.value for action in Action], help="Action to perform")
    parser.add_argument("--config", choices=[config.value for config in Configuration], default=Configuration.Release.value, help="Build configuration")
    parser.add_argument("--platform", choices=[platform.value for platform in Platform], default=Config.PLATFORM.value, help="Target platform")
    parser.add_argument("--verbose", action="store_true", help="Enable verbose output")
    parser.add_argument("--fresh", action="store_true", help="Force fresh build (clean before build)")
    args = parser.parse_args()

    # Update config based on args
    Config.VERBOSE = args.verbose
    Config.FRESH = args.fresh
    Config.PLATFORM = Platform(args.platform)
    build_config = Configuration(args.config)

    if Config.FRESH or (args.action == Action.CLEAN.value):
        clean_build_folder()
        if args.action == Action.CLEAN.value:
            if Config.VERBOSE:
                print("Clean action completed.")
            return

    if args.action == Action.GENERATE.value:
        generate_build_files()
    elif args.action == Action.BUILD.value:
        # Ensure build files are generated before building
        if not os.path.exists(Config.BUILD_FOLDER):
            if Config.VERBOSE:
                print("Build folder does not exist, generating build files first.")
            generate_build_files()
        build_project()
    elif args.action == Action.CLANG_FORMAT.value:
        run_clang_format()
    else:
        print(f"Unknown action: {args.action}")

if __name__ == "__main__":
    main()

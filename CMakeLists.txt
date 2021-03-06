cmake_minimum_required(VERSION 3.5)

include("3rd/HunterGate.cmake")
HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.251.tar.gz"
    SHA1 "5659b15dc0884d4b03dbd95710e6a1fa0fc3258d"
)

project(angelscript-llvm LANGUAGES CXX)

hunter_add_package(fmt)
find_package(fmt CONFIG REQUIRED)

if(DEFINED ENABLE_TESTING)
    hunter_add_package(Catch)
    find_package(Catch2 CONFIG REQUIRED)
endif()

add_library(angelscript-llvm
    src/asllvm/detail/ashelper.cpp
    src/asllvm/detail/assert.cpp
    src/asllvm/detail/builder.cpp
    src/asllvm/detail/debuginfo.cpp
    src/asllvm/detail/functionbuilder.cpp
    src/asllvm/detail/jitcompiler.cpp
    src/asllvm/detail/llvmglobals.cpp
    src/asllvm/detail/modulebuilder.cpp
    src/asllvm/detail/modulecommon.cpp
    src/asllvm/detail/modulemap.cpp
    src/asllvm/detail/runtime.cpp
    src/asllvm/detail/stackframe.cpp
    src/asllvm/jit.cpp
)

if(NOT DEFINED ANGELSCRIPT_SDK_DIRECTORY)
    message(
        FATAL_ERROR
        "CMake variable ANGELSCRIPT_SDK_DIRECTORY must be specified to the sdk/ directory of your AS installation"
    )
endif()

if(NOT IS_DIRECTORY "${ANGELSCRIPT_SDK_DIRECTORY}/angelscript/source")
    message(
        FATAL_ERROR
        "CMake variable ANGELSCRIPT_SDK_DIRECTORY does not appear to be valid: (path)/angelscript/source is not a directory"
    )
endif()

target_compile_features(angelscript-llvm PUBLIC cxx_std_17)
target_include_directories(angelscript-llvm PRIVATE include/
	${ANGELSCRIPT_SDK_DIRECTORY}/angelscript/source
	${ANGELSCRIPT_SDK_DIRECTORY}/angelscript/include
)
target_link_libraries(angelscript-llvm
    angelscript pthread
    fmt::fmt
    LLVM
)

# TODO: enable only on gcc AND clang
target_compile_options(angelscript-llvm PUBLIC
    "-fno-strict-aliasing"
    "-Wall"
    "-Wextra"
)

add_subdirectory(3rd)

if(DEFINED ENABLE_TESTING)
    add_subdirectory(tests/)
endif()

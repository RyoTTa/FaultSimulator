cmake_minimum_required(VERSION 3.1.0)
project(dramfaultsim LANGUAGES CXX VERSION 0.1)

set(default_build_type "Release")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

add_library(inih INTERFACE)
target_include_directories(inih INTERFACE ext/headers)

add_library(args INTERFACE)
target_include_directories(args INTERFACE ext/headers)

add_library(json INTERFACE)
target_include_directories(json INTERFACE ext/headers)

# Main DRAMSim Lib
add_library(dramfaultsim SHARED
        src/common.cc
        src/configuration.cc
        src/generator.cc
        src/memory_system.cc
        src/faultmodel.cc
        src/faultmap.cc
        src/stat.cc
        src/faultresult.cc src/faulttrace.cc src/faulttrace.h src/sighandler.cc src/sighandler.h)

if (DEBUG_TRACE)
    target_compile_options(dramfaultsim PRIVATE -DDEBUG_TRACE)
endif (DEBUG_TRACE)
if (TEST_MODE)
    target_compile_options(dramfaultsim PRIVATE -DTEST_MODE)
endif (TEST_MODE)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_include_directories(dramfaultsim INTERFACE src)
target_compile_options(dramfaultsim PRIVATE -Wall)
target_compile_options(dramfaultsim PUBLIC -pthread -O3)
target_link_libraries(dramfaultsim PRIVATE inih Threads::Threads)
set_target_properties(dramfaultsim PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

add_executable(dramfaultsimmain src/main.cc)
target_link_libraries(dramfaultsimmain PRIVATE dramfaultsim args Threads::Threads)
target_compile_options(dramfaultsimmain PRIVATE)
target_compile_options(dramfaultsimmain PUBLIC -pthread -O3)
if (TEST_MODE)
    target_compile_options(dramfaultsimmain PRIVATE -DTEST_MODE)
endif (TEST_MODE)
set_target_properties(dramfaultsimmain PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )


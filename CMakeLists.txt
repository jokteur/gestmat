cmake_minimum_required(VERSION 3.14)
project(gestmat)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXE_LINKER_FLAGS -static)
set(GCC_COVERAGE_COMPILE_FLAGS "-s -O3 -fvisibility=hidden")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/build/install/0.1)


# Python and TOML11 do not like each other
# This flag avoids interopabilities problems
add_compile_definitions(HAVE_SNPRINTF)

set(APP_LOG_DEBUG OFF CACHE BOOL "Collect log for debugging")
set(APP_PRINT_DEBUG OFF CACHE BOOL "If LOG_DEBUG is on, print the log to the std")

if (${APP_LOG_DEBUG})    
    add_compile_definitions(LOG_DEBUG)
endif()
if (${APP_PRINT_DEBUG})    
    add_compile_definitions(PRINT_DEBUG)
endif()

# --------------------------------
# Python installation and pybind11
# --------------------------------

add_definitions(-DPy_ENABLE_SHARED)

# set(Python_ROOT_DIR "" CACHE PATH "Python conda path")

set(Python_ROOT_DIR "C:\\Users\\jokte\\.conda\\envs\\python_app")
# set(Python_NUMPY_INCLUDE "" CACHE PATH "Numpy include dir path")

find_package(Python COMPONENTS Interpreter Development)


if((NOT ${Python_FOUND}))
    message(FATAL_ERROR
            "Missing Python installation\n"
            "Make sure that Python is installed on the system "
            "or to set the cache entry 'Python_ROOT_DIR' to point "
            "to a valid python installation"
            )
endif()
# Ensure that the python header is found
if(NOT EXISTS "${Python_INCLUDE_DIRS}/Python.h")
    message(FATAL_ERROR
            "Missing: \"${Python_INCLUDE_DIRS}/Python.h\",\n"
            "Set the cache entry 'Python_INCLUDE_DIRS' to point "
            "to a valid python include path. Containing "
            "Python.h for python version \"${Python_VERSION}\""
            )
endif()

message(${Python_INCLUDE_DIRS})
include_directories(${Python_INCLUDE_DIRS})
# include_directories(${Python_NUMPY_INCLUDE})

add_subdirectory(src/python)

add_subdirectory(vendors/pybind11)
include_directories(vendors/pybind11/include)

# ------
# TOML11
# ------
include_directories(vendors/toml11)

# --------
# TempoApp
# --------

add_subdirectory(vendors/TempoApp)
include_directories(vendors/TempoApp/include)

# ----
# Zlib
# ----
add_subdirectory(vendors/zlib)
include_directories(vendors/zlib)
# Hack because zconf.h keeps disappearing when building
target_include_directories(zlib PUBLIC "${PROJECT_SOURCE_DIR}/vendors/zconf")


# -----------
# Main source
# -----------
include_directories(src)
file (GLOB source_list RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        src/*.cpp src/*.h
        src/python/*.h
        src/python/*.cpp
        src/core/*.h src/core/*.cpp
        src/ui/*.cpp src/ui/*.h
    )

add_executable(${PROJECT_NAME} ${source_list} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE zlib Tempo ${Python_LIBRARIES})
target_include_directories(Tempo PUBLIC "${PROJECT_SOURCE_DIR}/vendors/TempoApp/include")
# target_include_directories(zlib PUBLIC "${PROJECT_SOURCE_DIR}/vendors/zlib")

# Copy the data (such as fonts) in build directory
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:${PROJECT_NAME}>) 

# -- Testing --

# add_library(${PROJECT_NAME}_lib ${source_list})
# add_subdirectory(tests)

# Set compiler options
if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY data DESTINATION ${CMAKE_INSTALL_PREFIX})

cmake_minimum_required(VERSION 3.14)
project(app)

set(CMAKE_CXX_STANDARD 17)

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

# -----
# glad
# -----

add_subdirectory(vendors/glad)
include_directories(vendors/glad/include)

# -----
# glfw3
# -----
# find_package(OpenGL REQUIRED)

set(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
set(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
set(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
set(GLFW_INSTALL "Generate installation target" OFF)

add_subdirectory(vendors/glfw)
include_directories(vendors/glfw/include)



# ---------------
# STB image write
# ---------------
include_directories(vendors/stb_image)

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

# ----------
# Dear Imgui
# ----------

include_directories(vendors/imgui/misc/cpp)
set(imgui_DIR "vendors/imgui")
set(imgui_src
        ${imgui_DIR}/imgui.cpp
        ${imgui_DIR}/imgui_demo.cpp
        ${imgui_DIR}/imgui_draw.cpp
        ${imgui_DIR}/imgui_widgets.cpp
        ${imgui_DIR}/imgui_tables.cpp
        ${imgui_DIR}/misc/cpp/imgui_stdlib.cpp
        ${imgui_DIR}/misc/cpp/imgui_stdlib.h)
include_directories(vendors/imgui)

# For GLFWwindowHandler
include_directories(src/rendering)
set(window_handler src/rendering/GLFWwindow_handler.h)

# --- Imgui bindings ---
set(imgui_bindings_DIR "vendors/imgui_bindings")
set(imgui_binding
        ${imgui_bindings_DIR}/imgui_impl_glfw.cpp
        ${imgui_bindings_DIR}/imgui_impl_glfw.h
        ${imgui_bindings_DIR}/imgui_impl_opengl3.cpp
        ${imgui_bindings_DIR}/imgui_impl_opengl3.h)
include_directories(vendors/imgui_bindings)

# ---------------
# opencv to numpy
# ---------------
# set(cv2np vendors/opencv2np/cv2np.h vendors/opencv2np/cv2np.cpp)
# include_directories(vendors/opencv2np)

# ----------------
# nativefiledialog
# ----------------
add_subdirectory(vendors/nativefiledialog-extended)
if (WIN32)
    target_link_libraries(nfd comctl32.lib uuid.lib)
endif()
# TODO implement for other OS
include_directories(vendors/nativefiledialog-extended/src/include)

# ------
# TOML11
# ------
include_directories(vendors/toml11)

# -----------
# Main source
# -----------
include_directories(src)
file (GLOB _project_src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        src/*.cpp src/*.h
        src/app/*.cpp src/app/*.h
        src/python/*.h
        src/python/*.cpp
        src/core/*.cpp src/core/*.h
        src/rendering/*.cpp src/rendering/*.h
        src/ui/*.cpp src/ui/*.h
        src/ui/panels/*.h src/ui/panels/*.cpp
        src/ui/modales/*.h src/ui/modales/*.cpp
        src/ui/views/*.cpp src/ui/views/*.h)

set(all_sources ${_project_src} ${imgui_src} ${imgui_binding})
list(REMOVE_ITEM all_sources "src/main.cpp")

add_library(${PROJECT_NAME}_lib STATIC ${all_sources})

target_link_libraries(${PROJECT_NAME}_lib glfw ${GLFW_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_lib glad)
target_link_libraries(${PROJECT_NAME}_lib ${OPENGL_gl_LIBRARY})
target_link_libraries(${PROJECT_NAME}_lib nfd)
target_link_libraries(${PROJECT_NAME}_lib ${Python_LIBRARIES})

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY assets DESTINATION ${CMAKE_INSTALL_PREFIX})

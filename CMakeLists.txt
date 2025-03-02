cmake_minimum_required(VERSION 3.10)
project(S28 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable threading
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Add option to select the build type: DLL or GUI
option(BUILD_DLL "Build the shared DLL" ON)
option(BUILD_GUI "Build the SFML GUI application" OFF)

# Define the source files for DLL and GUI
set(DLL_SOURCES
    DLL.cpp
    model/health.cpp
)

set(GUI_SOURCES
    app.cpp  # GUI application source file
    #model/health.cpp  # Uncomment if needed
)

# Build DLL
if(BUILD_DLL)
    add_library(S28 SHARED ${DLL_SOURCES})

    # Include directories
    include_directories(model)

    # Link with pthreads (for std::thread support)
    target_link_libraries(S28 PRIVATE Threads::Threads)

    # Set output name for DLL
    set_target_properties(S28 PROPERTIES OUTPUT_NAME "S28")

endif()

# Build GUI Application
if(BUILD_GUI)
    add_executable(S28_GUI ${GUI_SOURCES})

    # Include directories
    include_directories(model)

    # Link SFML
    find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
    target_link_libraries(S28_GUI PRIVATE sfml-graphics sfml-window sfml-system)

    # Link with pthreads
    target_link_libraries(S28_GUI PRIVATE Threads::Threads)

    # Set output name for the application
    set_target_properties(S28_GUI PROPERTIES OUTPUT_NAME "S28_GUI")

    # Post-build step to copy necessary SFML DLLs into the build directory
    add_custom_command(TARGET S28_GUI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "D:/Application/Executable/MinGW/bin/sfml-graphics-2.dll" $<TARGET_FILE_DIR:S28_GUI>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "D:/Application/Executable/MinGW/bin/sfml-window-2.dll" $<TARGET_FILE_DIR:S28_GUI>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "D:/Application/Executable/MinGW/bin/sfml-system-2.dll" $<TARGET_FILE_DIR:S28_GUI>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "D:/Application/Executable/MinGW/bin/sfml-graphics-d-2.dll" $<TARGET_FILE_DIR:S28_GUI>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "D:/Application/Executable/MinGW/bin/sfml-window-d-2.dll" $<TARGET_FILE_DIR:S28_GUI>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "D:/Application/Executable/MinGW/bin/sfml-system-d-2.dll" $<TARGET_FILE_DIR:S28_GUI>
    )

endif()

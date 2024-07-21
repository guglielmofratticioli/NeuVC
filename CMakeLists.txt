cmake_minimum_required(VERSION 3.16)
project(NeuVCPlugin VERSION 0.1.0)

# C++ settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable verbose makefile
set(CMAKE_VERBOSE_MAKEFILE ON)

# Set minimum MacOS target
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum OS X deployment version" FORCE)

# Enable universal binary on Mac if needed
option(UniversalBinary "Build universal binary for mac" OFF)
if (UniversalBinary)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE INTERNAL "")
    message(STATUS "Building Universal Binary")
endif()

# Set the location of libtorch
#set(Torch_DIR "C:/Program\ Files/libtorch-win-shared-with-deps-debug-2.3.1+cpu/libtorch/share/cmake/Torch")
set(Torch_DIR "/Users/guglielmofratticioli/Library/NeuVC/libs/libtorch/share/cmake/Torch")
find_package(Torch REQUIRED)


# Compile commands for IDEs like VS-Code
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Static linking on Windows
if (WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Add JUCE
add_subdirectory(ThirdParty/JUCE)

set(BaseTargetName NeuVC)

# Set COPY_PLUGIN_AFTER_BUILD based on OS
if (APPLE)
    set(COPY_PLUGIN_AFTER_BUILD TRUE)
else()
    set(COPY_PLUGIN_AFTER_BUILD FALSE)
endif()

juce_add_plugin("${BaseTargetName}"
    ICON_BIG "${CMAKE_CURRENT_LIST_DIR}/NeuVC/Assets/logo.png"
    ICON_SMALL "${CMAKE_CURRENT_LIST_DIR}/NeuVC/Assets/logo.png"

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${COPY_PLUGIN_AFTER_BUILD}
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Need access to Microphone"
    HARDENED_RUNTIME_ENABLED TRUE
    PLUGIN_MANUFACTURER_CODE DrAu
    PLUGIN_CODE NRNT
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "NeuVC"
)

juce_generate_juce_header(${BaseTargetName})

# Source files
file(GLOB_RECURSE SOURCES_PLUGIN ${CMAKE_CURRENT_LIST_DIR}/NeuVC/*.cpp ${CMAKE_CURRENT_LIST_DIR}/Lib/*.cpp)
file(GLOB_RECURSE HEADERS_PLUGIN ${CMAKE_CURRENT_LIST_DIR}/NeuVC/*.h ${CMAKE_CURRENT_LIST_DIR}/Lib/*.h)
target_sources(${BaseTargetName} PRIVATE ${SOURCES_PLUGIN} ${HEADERS_PLUGIN})

# Include directories
file(GLOB_RECURSE lib_dirs LIST_DIRECTORIES true ${CMAKE_CURRENT_LIST_DIR}/Lib/*)
foreach (dir ${lib_dirs})
    if (IS_DIRECTORY ${dir})
        target_include_directories(${BaseTargetName} PRIVATE ${dir})
    endif()
endforeach()

file(GLOB_RECURSE source_dirs LIST_DIRECTORIES true ${CMAKE_CURRENT_LIST_DIR}/NeuVC/*)
foreach (dir ${source_dirs})
    if (IS_DIRECTORY ${dir})
        target_include_directories(${BaseTargetName} PRIVATE ${dir})
    endif()
endforeach()

# Binary data
file(GLOB RESOURCES_FILES
    ${CMAKE_CURRENT_LIST_DIR}/Lib/ModelData/*.json
    ${CMAKE_CURRENT_LIST_DIR}/Lib/ModelData/*.ort
    ${CMAKE_CURRENT_LIST_DIR}/NeuVC/Assets/*.ttf
    ${CMAKE_CURRENT_LIST_DIR}/NeuVC/Assets/*.png
    ${CMAKE_CURRENT_LIST_DIR}/NeuVC/Assets/*.svg
)
juce_add_binary_data(bin_data SOURCES ${RESOURCES_FILES})

target_include_directories(${BaseTargetName} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/ThirdParty/minimp3)

target_compile_definitions(${BaseTargetName}
    PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(${BaseTargetName}
    PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    bin_data
    ${TORCH_LIBRARIES}
    PUBLIC
    juce_recommended_config_flags
    juce_recommended_lto_flags
)

# Add executable for test_rvc
add_executable(test_rvc NeuVC/Source/RVC/rvc.cpp)

# Include Torch directories
target_include_directories(test_rvc PRIVATE ${TORCH_INCLUDE_DIRS})

# Link Torch libraries
target_link_libraries(test_rvc PRIVATE ${TORCH_LIBRARIES})

# Set C++ standard for test_rvc
set_property(TARGET test_rvc PROPERTY CXX_STANDARD 17)

# Copy Torch DLLs on Windows
if (MSVC)
    file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
    add_custom_command(TARGET ${BaseTargetName}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${TORCH_DLLS}
        $<TARGET_FILE_DIR:${BaseTargetName}>
    )
    add_custom_command(TARGET test_rvc
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${TORCH_DLLS}
        $<TARGET_FILE_DIR:test_rvc>
    )
endif()

# Set runtime paths for development
set(CMAKE_BUILD_RPATH "${CMAKE_SOURCE_DIR}/path/to/your/libs")
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# Use the rpath settings in your target properties
set_target_properties(${BaseTargetName} PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
)

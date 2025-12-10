cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) # C++ minimum language version

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # output directory, self explanatory

include(FetchContent)

# Download GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4                                     # library version 3.4
    GIT_SHALLOW ON                                  # exclude version history
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)    # turn off examples
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)       # turn off tests         <- validates that GLFW is working, not needed since we're not developing GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)        # turn off documentation <- this is available offline
FetchContent_MakeAvailable(glfw)

# Download SFML
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1   # defines project version, SFML 3.0.1 in this case
    GIT_SHALLOW ON  # only download the latest version, and not full version history
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)

add_executable(
    main src/main.cpp
    "src/Core/Program.cpp"
    "src/Core/ConsoleColours.h"  # <- no .cpp file
    "src/Core/Services/WindowManager.cpp"
    "src/Core/Services.h" # <- no .cpp file
    "src/Core/Services/CommandConsole.cpp"
    "src/Core/Services/LocalisedTextService.h"
    "src/Core/Services/IService.h" # <- no .cpp file
    "src/Core/Window.cpp"
    "src/Core/Services/ObjectManager.cpp"
    "src/Core/Objects/IBaseObject.cpp"
    "src/Core/Pages/WinTile.cpp"
    "src/Core/Pages/UiPanel.cpp"
 )

target_compile_features(
    main PRIVATE
    cxx_std_17
)

target_link_libraries(
    main PRIVATE
    glfw
    SFML::Graphics
)
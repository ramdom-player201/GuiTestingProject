cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(
    main src/main.cpp
    "src/Core/Program.cpp"
    "src/Core/Program.h"
    "src/Core/ConsoleColours.h" 
    "src/Core/Services/WindowManager.h"
    "src/Core/Services/WindowManager.cpp"
    "src/Core/Services.h"
    "src/Core/Services/CommandConsole.cpp"
    "src/Core/Services/CommandConsole.h"
    "src/Core/Services/LocalisedTextService.h"
    "src/Core/Services/IService.h"
    "src/Core/Window.h"
    "src/Core/Window.cpp"
    "src/Core/Services/ObjectManager.cpp"
    "src/Core/Services/ObjectManager.h"
    "src/Core/Objects/IBaseObject.cpp"
    "src/Core/Objects/IBaseObject.h"
    "src/Core/Pages/WinTile.cpp"
    "src/Core/Pages/WinTile.h"
 "src/Core/Pages/UiPanel.h" "src/Core/Pages/UiPanel.cpp")
target_compile_features(main PRIVATE cxx_std_17)
target_link_libraries(main PRIVATE SFML::Graphics)

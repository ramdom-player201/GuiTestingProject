cmake_minimum_required(VERSION 3.31)
project(CMakeSFMLProject LANGUAGES C CXX)

# NOTE: all libraries need to be imported using a repository link or have the files stored in the repository to make compilation seamless for other users.

if(APPLE)
    enable_language(OBJC) # This is required to compile on MacOS for some reason...
endif()

set(CMAKE_CXX_STANDARD 23) # C++ minimum language version

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # output directory, self explanatory

include(FetchContent)

# Note, ensure [[Git for Windows]] is enabled if using Visual Studio, to allow CMake to import required dependencies

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

# Import VULKAN -- SDK needs to be installed manually :: https://vulkan.lunarg.com/sdk/home
#find_package(Vulkan QUIET)
#if (NOT Vulkan_FOUND)
#    message(FATAL_ERROR
#        "Vulkan SDK not found.\n"
#        "Please install it manually from https://vulkan.lunarg.com/sdk/home and try again.\n"
#        "Note that you may have to close and re-open your IDE after install."
#    )
#endif()

# Added additonal debug notes based on: https://stackoverflow.com/questions/77840729/why-is-cmake-find-packagevulkan-not-outputting-any-environment-variable
message("Attempting to find vulkan")
find_package(Vulkan QUIET) # fallback to default error message.
message("Vulkan install status: [" ${VULKAN_FOUND} "]")
message("Vulkan location: [" ${Vulkan_LIBRARIES} "]")
message("Vulkan include directory: [" ${Vulkan_INCLUDE_DIRS} "]")
if (NOT Vulkan_FOUND)
    message(
        FATAL_ERROR
        "Vulkan SDK not found.\n"
        "Please install it manually from https://vulkan.lunarg.com/sdk/home and try again.\n"
        "Note that you may have to close and re-open your IDE after install."
    )
endif()
#find_package(Vulkan REQUIRED COMPONENTS glslc glslangValidator) # Not used right now, do more research

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

# Download GLM          #https://github.com/g-truc/glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
)
FetchContent_MakeAvailable(glm)

add_executable(
    main src/main.cpp
    "src/Core/Program.cpp"
    "src/Core/Services/WindowManager.cpp"
    "src/Core/Services/CommandConsole.cpp"
   #"src/Core/Window.cpp"
    "src/Core/Services/ObjectManager.cpp"
    "src/Core/Objects/IBaseObject.cpp"
    "src/Core/Pages/WinTile.cpp"
    "src/Core/Pages/UiPanel.cpp"
    "src/Core/Services/LogService.cpp"
    "src/Core/Services/FileManager.cpp"
    "src/Core/Services/VulkanHandler.cpp"
    "src/Core/WindowClasses/BaseWindow.cpp"
    "src/Core/WindowClasses/TiledWindow.cpp"
    "src/Core/WindowClasses/TooltipWindow.cpp"
    "src/Core/WindowClasses/ContextWindow.cpp"
    "src/Core/WindowClasses/TestWindow.cpp"
    "src/Core/WindowClasses/DedicatedWindow.cpp"

    "src/Core/ConsoleColours.h"  # <- no .cpp file
    "src/Core/Services.h" # <- no .cpp file
    "src/Core/Services/IService.h" # <- no .cpp file
    "src/Core/Services/LocalisedTextService.h"
)

add_library(
    glad STATIC
    external/glad/src/glad.c
)

target_include_directories(
    glad PUBLIC
    external/glad/include
)

target_link_libraries(
    main PRIVATE
    SFML::Graphics
    Vulkan::Vulkan
    glm::glm
    glfw
    glad
)

target_compile_features(
    main PRIVATE
    cxx_std_23
)
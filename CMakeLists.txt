cmake_minimum_required(VERSION 3.22)

project(snake VERSION 0.1 LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

##########################################################################################
# Add dependencies with FetchContent
##########################################################################################

function(add_git_dependency libName gitURL gitTag)
    FetchContent_Declare(${libName}
        GIT_REPOSITORY ${gitURL}
        GIT_TAG        ${gitTag}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(${libName})
    target_compile_options(${libName} PRIVATE "-w")
endfunction()

# Add Raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
add_git_dependency(
    raylib
    https://github.com/raysan5/raylib.git
    5.0
)


##########################################################################################
# Project executable setup
##########################################################################################

add_executable(${PROJECT_NAME} main.c)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

##########################################################################################
# Project build settings
##########################################################################################

if (CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE -g -O0)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)

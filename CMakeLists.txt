cmake_minimum_required(VERSION 3.8)

project(Ark CXX)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_CLANG ON)
endif()

# setting up compilations options

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg -g -no-pie")
    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -s")
elseif (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/DWIN32 /D_WINDOWS /W3 /GR /EHa /MDd" CACHE STRING "Debug compilation options" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "/DNDEBUG /DWIN32 /D_WINDOWS /W3 /GR /EHa /Ox /Ob2 /Oi /Ot /Oy /MD" CACHE STRING "Release compilation options" FORCE)
    add_compile_options(/wd4267)  # disable warning about data loss (size_t -> int)
    add_compile_options(/wd4244)  # disable warning about data loss (size_t -> char)
elseif (CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# submodules
add_subdirectory("${Ark_SOURCE_DIR}/submodules/String")
# replxx
add_subdirectory("${Ark_SOURCE_DIR}/submodules/replxx")
set(REPLXX_INCLUDE_DIR "${Ark_SOURCE_DIR}/submodules/replxx/include")

# configuring Constants.hpp

# VERSION
set(ARK_VERSION_MAJOR 3)
set(ARK_VERSION_MINOR 0)
set(ARK_VERSION_PATCH 11)

message(STATUS "ArkScript version ${ARK_VERSION_MAJOR}.${ARK_VERSION_MINOR}.${ARK_VERSION_PATCH}")

# COMPILATION RELATED
set(ARK_COMPILATION_OPTIONS ${CMAKE_CXX_FLAGS})
set(ARK_COMPILER ${CMAKE_CXX_COMPILER_ID})
set(ARK_ENABLE_SYSTEM 1)  # enable use of (system "command here")

configure_file(
    ${Ark_SOURCE_DIR}/include/Ark/Constants.hpp.in
    ${Ark_SOURCE_DIR}/include/Ark/Constants.hpp
)

# files needed for the library ArkReactor
file(GLOB_RECURSE SOURCE_FILES
    ${Ark_SOURCE_DIR}/src/Builtins/*.cpp
    ${Ark_SOURCE_DIR}/src/Compiler/*.cpp
    ${Ark_SOURCE_DIR}/src/Parser/*.cpp
    ${Ark_SOURCE_DIR}/src/VM/*.cpp

    ${Ark_SOURCE_DIR}/thirdparty/*.cpp
)
list(APPEND SOURCE_FILES
    ${Ark_SOURCE_DIR}/src/Ark.cpp
    ${Ark_SOURCE_DIR}/src/Log.cpp
    ${Ark_SOURCE_DIR}/src/Utils.cpp
)

add_library(ArkReactor
    STATIC
        ${SOURCE_FILES}
        ${STRING_SOURCES}
)

# including content of project

target_include_directories(ArkReactor
    PUBLIC
        ${Ark_SOURCE_DIR}/include
        ${Ark_SOURCE_DIR}/thirdparty
        ${STRING_INCLUDES}
)

# linking libraries

find_package(Threads)

if (UNIX OR LINUX)
    if (CMAKE_COMPILER_IS_GNUCXX)
        target_link_libraries(ArkReactor PUBLIC stdc++fs)
    elseif (CMAKE_COMPILER_IS_CLANG)
        target_link_libraries(ArkReactor PUBLIC c++fs)
    endif()

    target_link_libraries(ArkReactor PUBLIC ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

# setting up project properties

set_target_properties(
    ArkReactor
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

# building exe

if (ARK_BUILD_EXE)
    # additional files needed for the exe (repl, command line and stuff)
    set(EXE_SOURCES
        ${Ark_SOURCE_DIR}/src/REPL/replxx/Util.cpp
        ${Ark_SOURCE_DIR}/src/REPL/Repl.cpp
        ${Ark_SOURCE_DIR}/src/main.cpp
    )

    include_directories(${REPLXX_INCLUDE_DIR})
    add_executable(Ark ${EXE_SOURCES})
    target_include_directories(Ark PUBLIC
        ${Ark_SOURCE_DIR}/include
        ${Ark_SOURCE_DIR}/thirdparty
    )
    target_link_libraries(Ark PUBLIC ArkReactor replxx)

    set_target_properties(
        Ark
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )

    # setting up installation directory

    install(TARGETS Ark
            DESTINATION bin
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                        GROUP_EXECUTE GROUP_READ
    )
    install(DIRECTORY ${Ark_SOURCE_DIR}/lib/
            DESTINATION lib
            PATTERN "lib/ext" EXCLUDE
            REGEX ".+\.md$" EXCLUDE
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                        GROUP_EXECUTE GROUP_READ
    )
endif()

# submodules

if (ARK_BUILD_MODULES OR ARK_BUILD_BASE_MODULES)
    add_subdirectory(${Ark_SOURCE_DIR}/lib/ext)
endif()

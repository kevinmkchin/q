cmake_minimum_required(VERSION 3.20)
set (CMAKE_CXX_STANDARD 20)

#########################################################################
######### Project setup                                         #########
#########################################################################

project(Game)

set(SOURCE_FILES
    code/Common.h
    code/Engine.h
    code/Engine.cpp
    code/Timer.h
    code/Timer.cpp
    code/Input.h
    code/Input.cpp
    code/GameRenderer.h
    code/GameRenderer.cpp
    code/MessageLogger.h
    code/MessageLogger.cpp
    code/Shader.h
    code/Shader.cpp
    code/FileSystem.h
    code/FileSystem.cpp
    code/Utils.h
    code/Utils.cpp
    code/Resources.h
    code/Resources.cpp
    code/Console.h
    code/Console.cpp
    code/Camera.h
    code/Camera.cpp
    code/Collision.h
    code/Collision.cpp
    code/GameRuntime.h
    code/GameRuntime.cpp
    code/GUI.h
    code/GUI.cpp
    code/EditorRuntime.h
    code/EditorRuntime.cpp
    code/LevelGeometry.h
    code/LevelGeometry.cpp
    code/ConstructiveSolidGeometry.h
    code/ConstructiveSolidGeometry.cpp
)

# Even tho these are added as target sources, CMake won't try to compile them if they don't have source file extension.
set(STUFF_TO_SHOW_IN_IDE_PROJECT
    lib
    wd
    readme.md
)

#source_group(code FILES ${SOURCE_FILES})

set(IS_WIN32PROGRAM 0)

# Create executable target
if (IS_WIN32PROGRAM)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES} ${STUFF_TO_SHOW_IN_IDE_PROJECT})
else()
    add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${STUFF_TO_SHOW_IN_IDE_PROJECT})
endif()

# Generate the shader folder location to the header
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/lib/project_path.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/lib/project_path.h")

set(MY_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/wd")

target_include_directories(${PROJECT_NAME} PUBLIC code/)
target_include_directories(${PROJECT_NAME} PUBLIC lib/single_headers/)

# My custom preprocessor defines
add_compile_definitions(
    INTERNAL_BUILD=1
    SLOW_BUILD=1
)

#########################################################################
######### Build options, additional include directories,        #########
######### library linking, copying DLLs to executable directory #########
#########################################################################

if(WIN32)
    # Set working directory for Visual Studio
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${MY_WORKING_DIRECTORY})

    if(MSVC)
      add_definitions(/MP)
    endif()

    target_compile_options(${PROJECT_NAME} PUBLIC
        # increase warning level
        "/W3"

        # Turn warning "not all control paths return a value" into an error
        "/we4715"

        # use sane exception handling, rather than trying to catch segfaults and allowing resource
        # leaks and UB. Yup... See "Default exception handling behavior" at
        # https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=vs-2019
        "/EHsc"

        # turn warning C4239 (non-standard extension that allows temporaries to be bound to
        # non-const references, yay microsoft) into an error
        "/we4239"
    )

    set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL/include")
    set(SDL2_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL/lib/x64/SDL2.lib;${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL/lib/x64/SDL2main.lib;")
    set(SDL_DLL "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL/lib/x64/SDL2.dll")

    set(GLEW_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/lib/GLEW/include/GL")
    set(GLEW_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/lib/GLEW/lib/Release/x64/glew32.lib")
    set(GLEW_DLL "${CMAKE_CURRENT_SOURCE_DIR}/lib/GLEW/lib/Release/x64/glew32.dll")

    set(ASSIMP_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/lib/ASSIMP/include;")
    set(ASSIMP_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/lib/ASSIMP/lib/assimp-vc142-mt.lib")
    set(ASSIMP_DLL "${CMAKE_CURRENT_SOURCE_DIR}/lib/ASSIMP/lib/assimp-vc142-mt.dll")

    target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS})

    target_link_libraries(${PROJECT_NAME} PUBLIC Shell32.lib opengl32.lib ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} ${ASSIMP_LIBRARIES})

    # Copy and rename dlls
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL_DLL}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLEW_DLL}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/glew32.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ASSIMP_DLL}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assimp-vc142-mt.dll")

else(APPLE)

    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_FRAMEWORK_IDENTIFIER org.cmake.${PROJECT_NAME}
            XCODE_GENERATE_SCHEME TRUE
            XCODE_SCHEME_WORKING_DIRECTORY ${MY_WORKING_DIRECTORY})

    find_package(SDL2 REQUIRED)
    if(SDL2_FOUND)
        message(STATUS "SDL2 found.")
        include_directories(${SDL2_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
    endif()

    find_package(OpenGL REQUIRED)
    if(OpenGL_FOUND)
        message(STATUS "OpenGL found.")
        target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
    else()
        message(FATAL_ERROR "CMake couldn't find OpenGL on MacOSX.")
    endif()

    find_package(assimp REQUIRED)
    if(assimp_FOUND)
        message(STATUS "Assimp found.")
        include_directories(${ASSIMP_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARIES})
    endif()

endif()

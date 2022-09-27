cmake_minimum_required(VERSION 3.20)
set (CMAKE_CXX_STANDARD 20)

project(Game)

set(IS_WIN32PROGRAM 0)

add_compile_definitions(
        INTERNAL_BUILD=1
        SLOW_BUILD=1
)

set(SOURCE_FILES 
    "code/Engine.cpp"
    "code/Timer.cpp"
    "code/Input.cpp" 
    "code/GameRenderer.cpp" 
    "code/MessageLogger.cpp" 
    "code/Shader.cpp"
    "code/FileSystem.cpp"
    "code/Utils.cpp" 
    "code/Resources.cpp" 
    "code/Console.cpp" 
    "code/Camera.cpp" 
    "code/Collision.cpp" 
    "code/GameRuntime.cpp"
    "code/GUI.cpp"
    "code/EditorRuntime.cpp")



# Create executable target
if (IS_WIN32PROGRAM)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})
else()
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})
endif()

if(MSVC)
  add_definitions(/MP)
endif()

# Generate the shader folder location to the header
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/lib/project_path.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/lib/project_path.h")

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

target_include_directories(${PROJECT_NAME} PUBLIC code/)
target_include_directories(${PROJECT_NAME} PUBLIC lib/single_headers/)

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

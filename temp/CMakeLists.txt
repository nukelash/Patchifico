cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

if (EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif ()

if (EMSCRIPTEN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1 --preload-file /Users/lukenash/Documents/Github/synth/resources")
  set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif ()


add_executable(basic main.cpp)

if(APPLE)
    set(FRAMEWORKS "-framework CoreVideo"
                   "-framework IOKit" 
                   "-framework Cocoa"
                   "-framework GLUT" 
                   "-framework OpenGL")
    target_link_libraries(basic PUBLIC ${FRAMEWORKS})
endif()

# Emscripten
set(RAYLIB_INCLUDE ${CMAKE_SOURCE_DIR}/../dep/raylib/wasm/include)
set(RAYLIB_LIB ${CMAKE_SOURCE_DIR}/../dep/raylib/wasm/lib/libraylib.a)

# Normal
# set(RAYLIB_INCLUDE ${CMAKE_SOURCE_DIR}/dep/raylib/include)
# set(RAYLIB_LIB ${CMAKE_SOURCE_DIR}/dep/raylib/lib/libraylib.a)

include_directories(basic ${RAYLIB_INCLUDE})
target_link_libraries(basic PUBLIC ${RAYLIB_LIB})
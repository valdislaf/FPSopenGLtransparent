
cmake_minimum_required(VERSION 3.22.3)
set(name "textopenglfps")
project(${name})
if(UNIX AND NOT APPLE)
    # Здесь указываем команды, которые должны быть выполнены только на Linux.
    set(GLUT_LIBRARY "/usr/lib/x86_64-linux-gnu/libglut.so")
    find_package(OpenGL REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    find_package(Freetype REQUIRED)
    # Add this line to link against GLUT
    find_package(GLUT REQUIRED)
    #find_package(glm REQUIRED)

    include_directories(${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})
    link_directories(${GTK3_LIBRARY_DIRS})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    link_directories(${CMAKE_CURRENT_SOURCE_DIR})
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib-mingw-w64)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib-vc2022)
    add_library(glad "${CMAKE_CURRENT_SOURCE_DIR}/glad.c")
    add_executable(${name} main.cpp)
    target_link_libraries(${name} ${OPENGL_LIBRARIES} glfw GLEW ${GTK3_LIBRARIES})
    target_link_libraries(${name} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLFW_LIBRARIES} ${GTK3_LIBRARIES} ${GLUT_LIBRARY})
    target_link_libraries(${name} ${OPENGL_LIBRARIES} glfw GLEW ${GTK3_LIBRARIES} ${GLUT_LIBRARY})
    target_link_libraries(${name} glfw3dll)
    target_link_libraries(${name} glad)
    #target_link_libraries(${name}  glm)
    target_link_libraries(${name}  GLEW)
    target_link_libraries(${name}  freetype)
elseif(WIN32)
    # Здесь указываем команды, которые должны быть выполнены только на Windows.
    set(CMAKE_CXX_STANDARD 17)
    # Добавление пути до папки glm
    #include_directories("C:/glm-0.9.9.8")

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    link_directories(${CMAKE_CURRENT_SOURCE_DIR})
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib-mingw-w64)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib-vc2022)
    add_library(glad "${CMAKE_CURRENT_SOURCE_DIR}/glad.c")

    set(FREEGLUT_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/freeglut")
    set(FREEGLUT_INCLUDE_DIR "${FREEGLUT_ROOT_PATH}/include")
    set(FREEGLUT_LIB_DIR "${FREEGLUT_ROOT_PATH}/build/lib/Debug")

    include_directories(${FREEGLUT_INCLUDE_DIR})
    link_directories(${FREEGLUT_LIB_DIR})

    set(FREETYPE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/freetype")
    set(FREETYPE_INCLUDE_DIRS "${FREETYPE_DIR}/include")
    set(FREETYPE_LIBRARY_DIRS "${FREETYPE_DIR}/objs/x64/Debug")
    set(FREETYPE_LIBRARIES "freetype")

    include_directories(${FREETYPE_INCLUDE_DIRS})
    link_directories(${FREETYPE_LIBRARY_DIRS})

    #link_directories("C:/glut/lib")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/glew/include")
    set(GLEW_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glew/lib/Release/x64")
    #include_directories("C:/freetype/include")
    add_executable(${name} main.cpp)
    target_link_libraries(${name} ${FREEGLUT_LIB_DIR}/freeglutd.lib)
    target_link_libraries(${name} ${FREEGLUT_LIB_DIR}/freeglut_staticd.lib)
    #target_link_libraries(${name} Freetype)
            #"C:/freetype/objs"
    target_link_libraries(${PROJECT_NAME} ${FREETYPE_LIBRARIES})
    #target_link_libraries(${name} glut)
    target_link_libraries(${name} glfw3dll)
    target_link_libraries(${name} glad)
    target_link_libraries(${name} "${GLEW_LIB_DIR}/glew32.lib")
    # target_link_libraries(${name} "${GLEW_LIB_DIR}/glew32.lib")
    #  target_link_libraries(${name} "${GLEW_LIB_DIR}/glew32s.lib")
    # target_link_libraries(${name} glfw libglad.a C:/glew-2.1.0/lib/Release/x64/glew32s.lib C:/glew-2.1.0/lib/Release/x64/glew32.lib)
    target_link_libraries(${name} opengl32)

    if (CMAKE_GENERATOR MATCHES "Visual Studio")
        # Копирование файлов для Visual Studio
        add_custom_command(TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/freeglutd.dll"
                "${CMAKE_CURRENT_SOURCE_DIR}/freeglutd.pdb"
                "${CMAKE_CURRENT_SOURCE_DIR}/glew32.dll"
                "${CMAKE_CURRENT_SOURCE_DIR}/freetype.dll"
                "$<TARGET_FILE_DIR:${name}>"
                )
        add_custom_command(TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
                "$<TARGET_FILE_DIR:${name}>/fonts"
                )
        # Определите путь к шрифту относительно директории исходного кода
        set(FONT_PATH "fonts/arial.ttf")

        # Копирование шрифта в директорию сборки
        configure_file(
                ${CMAKE_CURRENT_SOURCE_DIR}/${FONT_PATH}
                ${CMAKE_CURRENT_BINARY_DIR}/${FONT_PATH}
                COPYONLY
        )
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    else()
        # Копирование файлов для MinGW
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/freeglutd.dll ${CMAKE_CURRENT_BINARY_DIR}/freeglutd.dll COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/freeglutd.pdb ${CMAKE_CURRENT_BINARY_DIR}/freeglutd.pdb COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/glew32.dll ${CMAKE_CURRENT_BINARY_DIR}/glew32.dll COPYONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/freetype.dll ${CMAKE_CURRENT_BINARY_DIR}/freetype.dll COPYONLY)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    endif()
endif()

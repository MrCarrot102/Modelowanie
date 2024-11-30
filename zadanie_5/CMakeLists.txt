cmake_minimum_required(VERSION 3.10)

# Nazwa projektu
project(ParticleSystem3D)

# Standard C++
set(CMAKE_CXX_STANDARD 17)

# Szukamy bibliotek OpenGL i GLEW
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SFML 2.6 REQUIRED COMPONENTS window system)

# Dodajemy odpowiednie katalogi z nagłówkami
include_directories(${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${SFML_INCLUDE_DIRS})

# Tworzymy plik wykonywalny
add_executable(ParticleSystem3D main.cpp)

# Linkujemy odpowiednie biblioteki
target_link_libraries(ParticleSystem3D ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${SFML_LIBRARIES})

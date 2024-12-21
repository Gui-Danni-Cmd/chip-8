cmake_minimum_required(VERSION 3.10)

project(Chip-8)

set(SRC_DIR src)
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.c")

set(CMAKE_C_STANDARD 11)

# Executável principal
add_executable(Chip-8 ${SOURCES})

if(WIN32)
    set(SDL2_INCLUDE_DIRS "C:/libs/SDL2/include")
    set(SDL2_LIBRARIES "C:/libs/SDL2/lib/SDL2.lib")
    include_directories(${SDL2_INCLUDE_DIRS})
    target_link_libraries(Chip-8 PRIVATE ${SDL2_LIBRARIES})
elseif(UNIX AND NOT APPLE)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    target_link_libraries(Chip-8 PRIVATE ${SDL2_LIBRARIES})
elseif(APPLE)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    target_link_libraries(Chip-8 PRIVATE ${SDL2_LIBRARIES})
else()
    message(FATAL_ERROR "Sistema operacional não suportado!")
endif()

# Opções de compilação
option(DEBUG_MODE "Ativar o modo de depuração" OFF)
if(DEBUG_MODE)
    target_compile_definitions(Chip-8 PRIVATE -DDEBUG_MODE)
endif()



# Comandos para ativar modos:
# - Modo Debug: cmake -B build . -DDEBUG_MODE=ON
# - Modo Teste: cmake -B build . -DTEST_MODE=ON
# - Ambos: cmake -B build . -DDEBUG_MODE=ON -DTEST_MODE=ON
# Compilar: cmake --build build

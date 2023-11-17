# specify minimum required
cmake_minimum_required(VERSION 3.16.3)

# specify project name
project(coffee)

# specify the toolchain file
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/tiva_tm4c123_toolchain.cmake")

# specify the startup file
set(STARTUP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/startup_gcc.c)

# specify which file will be built
add_executable(${PROJECT_NAME} cof.c ${STARTUP_FILE})

# provide cmake the path to the include directories we want
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TivaWare_C_Series-2.2.0.295/)

# tell cmake where to find the library (path to .a file)
target_link_libraries(${PROJECT_NAME} PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/TivaWare_C_Series-2.2.0.295/driverlib/gcc/libdriver.a)

# set additional properties for the executable
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")

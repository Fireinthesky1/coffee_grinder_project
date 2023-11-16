# specify minimum required
cmake_minimum_required(VERSION 3.16.3)

# specify project name
project(coffee)

# specify which file will be built
add_executable(${PROJECT_NAME} cof.c)

# provide cmake the path to the include directories we want
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TivaWare_C_Series-2.2.0.295/)

# tell cmake where to find the library (path to .a file)
target_link_libraries(${PROJECT_NAME} PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/TivaWare_C_Series-2.2.0.295/driverlib/gcc/libdriver.a)

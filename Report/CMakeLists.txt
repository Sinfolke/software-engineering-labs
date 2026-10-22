cmake_minimum_required(VERSION 3.16)
project(Lab_2_Melnyk LANGUAGES CXX)

add_executable(Lab_2_Melnyk src/algorithm.cpp src/main.cpp src/interface.cpp src/global.cpp)
target_compile_features(Lab_2_Melnyk PRIVATE cxx_std_20)
configure_file(input.txt input.txt COPYONLY)

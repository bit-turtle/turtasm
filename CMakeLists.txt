cmake_minimum_required( VERSION 3.5 )

# TurtASM Compiler
project( turtasm )
add_executable( turtasm WIN32 turtasm.cpp )

# TurtASM Linker
project( turtlink )
add_executable( turtlink WIN32 turtlink.cpp )

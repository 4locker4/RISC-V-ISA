cmake_minimum_required(VERSION 3.11)
project("interpretator")

# Указываем исходные файлы
SET(SRCS 
	./CppInterp/src/Interpr.cpp
)

# Указываем заголовочные файлы
set(HEADERS
    ./CppInterp/inc/Interpr.hpp
)

# Создаем библиотеку
add_library(lclib STATIC ${SRCS})

# Указываем пути к заголовочным файлам
target_include_directories(lclib PUBLIC 
    ./CppInterp/inc
)

add_executable(InterprProg.a ./CppInterp/src/Interpr.cpp)

target_link_libraries(InterprProg.a lclib)
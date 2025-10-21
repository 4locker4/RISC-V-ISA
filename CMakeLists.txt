cmake_minimum_required(VERSION 3.11)
project("interpretator")

# Указываем исходные файлы
SET(SRCS 
	./CppInterp/src/Execute.cpp
	./CppInterp/src/Decode.cpp
	./CppInterp/src/CPU.cpp

)

# Указываем заголовочные файлы
set(HEADERS
    ./CppInterp/inc/CPU.hpp
    ./CppInterp/inc/Decode.hpp
    ./CppInterp/inc/Execote.hpp
)

# Создаем библиотеку
add_library(lclib STATIC ${SRCS})

# Указываем пути к заголовочным файлам
target_include_directories(lclib PUBLIC 
    ./CppInterp/incinc
)

add_executable(InterprProg.a ./CppInterp/src/CPU.cpp)

target_link_libraries(InterprProg.a lclib)
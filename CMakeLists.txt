cmake_minimum_required(VERSION 3.11)
project("interpretator")

# Указываем исходные файлы
SET(SRCS 
	./CppInterp/src/Execute.cpp
	./CppInterp/src/Decode.cpp
	./CppInterp/src/Machine.cpp

)

# Указываем заголовочные файлы
set(HEADERS
    ./CppInterp/inc/Machine.hpp
    ./CppInterp/inc/Decode.hpp
    ./CppInterp/inc/Execote.hpp
    ./CppInterp/inc/Enums.hpp
)

# Создаем библиотеку
add_library(lclib STATIC ${SRCS})

# Указываем пути к заголовочным файлам
target_include_directories(lclib PUBLIC 
    ./CppInterp/incinc
)

add_executable(InterprProg.a ./CppInterp/src/Machine.cpp)

target_link_libraries(InterprProg.a lclib)
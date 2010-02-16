cmake_minimum_required(VERSION 2.6)
project (libpthread++)
set(SRCS PThread++.cpp)
add_library(pthread++ STATIC ${SRCS})

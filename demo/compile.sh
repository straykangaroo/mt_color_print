#!/bin/sh

g++ --std=c++17 -Wall -Wextra -Wpedantic -Weffc++ -I.. -pthread -O3 -o mt_color_print_demo mt_color_print_demo.cpp

#!/usr/bin/env tclsh

puts [pwd]
set CC "gcc"
set debug "-ggdb -Wall -Wextra -fsanitize=address"
set build "build"

file mkdir "./$build/"
eval "exec $CC $debug ./main.c -o ./$build/tcl.out"

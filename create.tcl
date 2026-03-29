#!/usr/bin/env tclsh

puts [pwd]
set CC "gcc"
set debug "-ggdb -Wall -Wextra -fsanitize=address"
set build "build"

file mkdir "./$build/"
if { [catch "exec $CC $debug ./main.c -o ./$build/tcl.out" fid] } {
	set comb ""
	set cnt 30
	foreach {pt} [split $fid "\n"] {
		append comb "$pt\n"
		incr cnt -1
		if {$cnt <= 0} break
	}
	puts "$comb"
	exit 1
}



proc testing {a b} {

	puts "$a $b"
	puts "+ [expr {$a + $b}]"
	puts "* [expr {$a * $b}]"
	set g [expr {$a + $b * $a}]
	set h [expr {$a + $b / $a}]
	puts "// $h"
}

testing 4 5
testing 2 5

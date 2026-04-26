

proc testing {a b} {

	puts "$a $b"
	puts "+ [expr {$a + $b}]"
	puts "* [expr {$a * $b}]"
}

testing 4 5
testing 2 5

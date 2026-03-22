
set n 5
if {$n > 0} {
    puts "countdown:"
    while {$n > 0} {
        puts $n
        incr n -1
    }
} else {
    puts "nothing to do"
}



set a 4
set b 7

puts "a $a"
puts "b $b"
puts ""
puts "a + b [expr {$a + $b}]"
puts "a - b [expr {$a - $b}]"
puts "a / b [expr {$a / $b}]"
puts "a * b [expr {$a * $b}]"
puts "a % b [expr {$a % $b}]"
puts ""

puts "a >  b [expr {$a > $b}]"
puts "a >= b [expr {$a >= $b}]"
puts "a <  b [expr {$a < $b}]"
puts "a <= b [expr {$a <= $b}]"
puts "a == b [expr {$a == $b}]"
puts "a != b [expr {$a != $b}]"
puts ""
puts "a + b * a [expr {$a + $b * $a}]"

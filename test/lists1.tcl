
set items {apple banana cherry}
lappend items date
puts "items: [join $items , ]"
puts "first: [lindex $items 0] last: [lindex $items end]"
puts "contains banana? [expr {[lsearch -exact $items banana] >= 0}]"



set usingExe "./build/tcl-i"
if {![file exists $usingExe]} {
	puts "$usingExe seems to not exists"
	#error
	exit 1
}
set files [glob test/*]
set fCount 0
set eCount 0
set eFiles ""
#puts $files

proc run-tcl {usingExe run} {
	puts "Running $run"
	set output1 [exec tclsh $run]
	set output2 [exec $usingExe $run]

	if {$output1 != $output2} {
		puts "Error in $run"
		puts "1===="
		puts $output1
		puts "2===="
		puts $output2
		puts "====="
		return 1
	}
	return 0
}

foreach {fl} "$files" {
	#upvar 0 run-tcl
	if {[run-tcl $usingExe $fl]} {
		incr eCount
		lappend eFiles $fl
	}
	incr fCount
}


if {$eCount == 0} {
	puts "Everything fine! $fCount Tested"
} else {
	puts "$eCount out of $fCount failed"
	puts "$eFiles"
}

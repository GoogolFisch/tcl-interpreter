

set usingExe "./build/tcl.out"
if {![file exists $usingExe]} {
	puts "$usingExe seems to not exists"
	#error
	exit 1
}
set files ""
catch {set files "$files [glob test/*]"} errno
catch {set files "$files [glob hide/*]"} errno
set fCount 0
set eCount 0
set eFiles ""
#puts $files

proc run-tcl {usingExe run} {
	puts "Running $run"
	set runOk 0
	if "[catch {set output1 [exec tclsh $run]} errno]" {
		puts "error in tclsh!!\n$errno"
		exit 1
	}
	if "[catch {set output2 [exec $usingExe $run]} errno]" {
		puts "error in $usingExe:\n$errno"
		set output2 $errno
		set runOk 1
	}

	if {"@$output1" == "@$output2"} {
		puts "Error in $run"
		puts "<<<<<"
		puts $output1
		puts "====="
		puts $output2
		puts ">>>>>"
		set runOk 1
	}
	return $runOk
}

foreach {fl} "$files" {
	#upvar 0 run-tcl
	if {[run-tcl $usingExe $fl]} {
		incr eCount
		set eFiles "$eFiles$fl\n"
	}
	incr fCount
}


if {$eCount == 0} {
	puts "Everything fine! $fCount Tested"
} else {
	puts "$eCount out of $fCount failed"
	puts "$eFiles"
}

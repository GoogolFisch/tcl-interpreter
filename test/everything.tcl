# parse_tests.tcl
# Tcl parsing-only comprehensive test-suite
# Exit 0 on success; non-zero on any detected parse failure.
# By design this file exercises quoting, substitution, grouping, lists,
# command and procedure definitions, namespace/variable syntax, here-doc style,
# nested comments (via tricky constructs), backslash sequences, unicode,
# ambiguous whitespace/newline placements, and corner cases.

namespace eval ::parse_tests {}

proc ::parse_tests::fail {msg} {
    puts stderr "PARSE-FAIL: $msg"
    exit 2
}

proc ::parse_tests::assert_eq {a b msg} {
    if { ![string equal -- $a $b] } {
        ::parse_tests::fail "Assertion failed: $msg -- expected <$b> got <$a>"
    }
}

# Section: Basic tokenization
set s1 "simpleToken"
set s2 "with space"
set s3 {brace quoted string}
::parse_tests::assert_eq $s1 simpleToken "basic token"
::parse_tests::assert_eq $s2 {with space} "double quoted with space"
::parse_tests::assert_eq $s3 {brace quoted string} "brace quoted"

# Section: Command substitution and nested substitution
set sub_text [string trim {  nested [concat a b] }]
::parse_tests::assert_eq $sub_text "nested ab" "command substitution nested"

# Section: Variable substitution edge cases
set x 42
set y "value with \$ x\$\{x\}"
set expanded "value with \$ x42"
::parse_tests::assert_eq $y $expanded "variable substitution with escapes"

# Section: Braces vs quotes: ensure braces prevent substitutions
set inside "{not substituted \$x [list a]}"
set braced {not substituted $x [list a]}
::parse_tests::assert_eq $inside $braced "braced literal equality"

# Section: Backslash sequences and line continuations
set bs1 "line1\\\nline2"     ;# backslash-newline removes the newline in Tcl source when parsed
set expected_bs1 "line1line2"
# In many tclshs, the lexer removes backslash-newline at parse time; check that
if { [string first "\n" $bs1] >= 0 } {
    # If Tcl kept newline then this environment is different; just ensure bs1 contains newline removed variant or original trimmed variant
    set try [string map {\n ""} $bs1]
    if { $try ne $expected_bs1 } {
        ::parse_tests::fail "backslash-newline handling"
    }
} else {
    # no newline present; ensure equals expected
    ::parse_tests::assert_eq $bs1 $expected_bs1 "backslash-newline removed"
}

# Section: Lists with tricky whitespace and braces
set list1 { a  b\tc {d e} {f{g}h} "" }
set joined [join $list1 : ]
set expected_join "a: b\tc: {d e}: {f{g}h}: "
::parse_tests::assert_eq $joined $expected_join "list spacing and nested braces"

# Section: Comments and commented line continuations
# A comment starting with # until newline. Ensure that trailing backslash in comment doesn't escape newline.
# (No direct test besides being syntactically present)
# single-line comment
# multi-comment style via multiple # lines
# Ensure parser ignores them and continues parsing following commands
set after_comments "ok"
::parse_tests::assert_eq $after_comments ok "comments present"

# Section: Here-doc-like with braces + command substitution inside quotes
set heredoc {
This is a {heredoc} with [list] and \$escaped variable
}
# The variable $x should not be expanded inside the braces above
if { [string match *heredoc* $heredoc] == 0 } {
    ::parse_tests::fail "heredoc brace mismatch"
}

# Section: Procedure and eval forms (syntax only)
proc testProc {a {b 2} args} {
    # body uses quoting and command substitution but parser must accept proc syntax
    set _s "proc body with \$a \$b"
    return $_s
}
namespace eval ::parse_tests::ns {
    proc nestedProc {x y} { return [list $x $y] }
}
# Call them to ensure parser processed their definitions
set tp [testProc 1 3]
::parse_tests::assert_eq $tp "proc body with 1 3" "proc definition parsed"

# Section: namespace and global/local variable declarations syntax
namespace eval ::ns_test {
    variable v1 10
    proc p1 {} {
        variable v1
        set v1 20
        return $v1
    }
}
set g1 [::ns_test::p1]
::parse_tests::assert_eq $g1 20 "namespace & variable declarations"

# Section: complex quoting: nested quotes, brackets, backslash escapes
set complex "one\"two\\\nthree [string repeat x 0] {literal {nested}} \$notvar"
# Ensure this parsed into a string (no exception), do a simple substring test
if { [string first "one\"two" $complex] < 0 } {
    ::parse_tests::fail "complex quoting not present"
}

# Section: Arrays and dicts (syntax)
array set A {k1 v1 k2 {v 2} }
set d [dict create a 1 b {two three}]
::parse_tests::assert_eq [dict get $d a] 1 "dict create/get"

# Section: backquoted command (deprecated but valid in some Tcls) - treat as string literal if unsupported
# We write it inside a string to avoid execution: " `not executed` "
set backtick_string "contains `backtick` literal"
::parse_tests::assert_eq $backtick_string "contains `backtick` literal" "backtick in string"

# Section: Multiple semicolons, empty commands, and trailing spaces/newlines
set ;;; tmp  ;# ensure parser ignores empty commands separated by ;;; - syntactically valid only as separators
# The preceding line is intentionally odd but syntactically valid as a no-op followed by 'tmp' word (tmp is a bare word, not a command) — to keep parsing-only, we comment out any execution of tmp
# Instead test multiple semicolons in an innocuous construct:
set a 1;; set b 2;;; set c 3
::parse_tests::assert_eq $c 3 "multiple semicolons handled"

# Section: binary data and NUL embed attempt (Tcl strings can contain binary; represent via format)
set bin [binary format c 0]
if { [string length $bin] != 1 } {
    ::parse_tests::fail "binary format created unexpected length"
}

# Section: Unicode and extended characters in identifiers/strings
set uni "µ Δ 漢字 \u2603"
if { [string index $uni 0] eq "" } {
    ::parse_tests::fail "unicode string parse issue"
}

# Section: Control flow syntax shapes (only parse-prove, not heavy logic)
if { 1 } {
    switch -glob -- "abc" {
        "a*" { set sw "matched a" }
        default { set sw "no" }
    }
} else {
    set sw "no-run"
}
::parse_tests::assert_eq $sw "matched a" "switch and if parsing"

# Section: catch/error handling syntax
if { [catch {expr {1 + 1}} res] } {
    ::parse_tests::fail "catch/expr parsing"
}
::parse_tests::assert_eq $res 2 "expr evaluated inside catch"

# Section: tricky nested braces and quoting combos
set tricky { {a {b {c {d}}}} "{a \"b\" [list c]}" }
# Ensure parser read nested structure without error
if { [llength $tricky] < 1 } {
    ::parse_tests::fail "nested braces parse"
}

# Section: validation of quoting of numeric-like tokens and leading zeros
set num "007"
::parse_tests::assert_eq $num 007 "leading-zero numeric string preserved"

# Section: regexp and quoting of backslashes inside patterns
set pattern {^\d+\.\d*$}
# Use it in a regexp invocation to ensure pattern was parsed as a single token
if {![regexp $pattern "123.45"]} {
    ::parse_tests::fail "regexp pattern parse"
}

# Section: eval of constructed command strings (parsing at runtime via eval is allowed for test)
set cs {set z "constructed parse ok"}
eval $cs
::parse_tests::assert_eq $z "constructed parse ok" "eval parsing of constructed command"

# Section: complex proc with lambda-like anonymous proc (using apply)
set aproc [list {x y} {expr {$x + $y}}]
set sum [apply $aproc 3 4]
::parse_tests::assert_eq $sum 7 "apply anonymous proc parsed"

# Section: ensure here-doc style quoting with triple braces and inner braces
set multi {{{inner {brace}}} plain}
::parse_tests::assert_eq [lindex $multi 0] {inner {brace}} "triple-brace list element"

# Section: ensure parser accepts commands split across lines with continuation
set continued "part1 \
part2"
::parse_tests::assert_eq $continued "part1 part2" "line continuation with backslash"

# Section: Eof marker sanity (file ends after these tests)
puts "PARSE-OK"
exit 0

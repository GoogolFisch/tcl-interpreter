

proc greet {msg from} {
	puts "Hello $msg from $from"
}
proc meet {msg from} {
	puts "${from}, ${msg}"
}

greet "Hey" "this"
greet "Bey" "pc"
meet "Hola" "Romeo"
meet "¿Qué tal?" "Julia"

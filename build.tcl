
set CC "gcc"
set debug "-ggdb -Wall -Wextra -fsanitize=address"

eval "exec $CC $debug main.c"

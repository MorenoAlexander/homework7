all: processes threads threadpool examples microbenchmarks

processes: macro_processes.c statistics.c statistics.h
	gcc -Wall -std=gnu99 -ggdb3 -o processes statistics.c macro_processes.c -lm

threads: macro_threads.c statistics.c statistics.h
	gcc -Wall -std=gnu99 -ggdb3 -o threads statistics.c macro_threads.c -lm -lpthread

threadpool: macro_threadpool.c statistics.c statistics.h
	gcc -Wall -std=gnu99 -ggdb3 -o threadpool statistics.c macro_threadpool.c -lm -lpthread

microbenchmarks: microbenchmarks.c statistics.c statistics.h
	gcc -Wall -std=gnu99 -ggdb3 -o microbenchmarks statistics.c microbenchmarks.c -lm -lpthread

examples: examples.c statistics.c statistics.h
	gcc -Wall -std=gnu99 -ggdb3 -o examples examples.c statistics.c -lm

runall:
	./processes ; ./threads ; ./threadpool ; ./microbenchmarks

clean:
	rm -f processes threads threadpool examples microbenchmarks

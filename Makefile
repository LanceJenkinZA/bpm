bpm: main.o
	gcc $< -o $@

.c.o:
	gcc -c -O3 $< -o $@

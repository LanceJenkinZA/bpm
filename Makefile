bpm: main.o cms50d.o
	gcc $^ -o $@

.c.o:
	gcc -c -O3 $< -o $@

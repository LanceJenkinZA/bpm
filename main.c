#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "cms50d.h"

#define NUM_LINES	2
static const char wavmap[] = "0123456789abcdef";

struct stats {
	unsigned	min;
	unsigned	max;
	unsigned	total;
	double		sum;
};

static void print_line(FILE* f, struct stats *s, unsigned w) {
	unsigned i;
	for (i = 0; i < w; i++) {
		struct pkt p;
		if (!cms50d_getpkt(f, &p)) {
			continue;
		}
		if (p.pulse.v && s->min > p.pulse.v) {
			s->min = p.pulse.v;
		}
		if (s->max < p.pulse.v) { 
			s->max = p.pulse.v;
		}
		printf("%c", wavmap[p.wav.signal]);
		fflush(stdout);
		s->sum += p.pulse.v;
		s->total++;
	}
}

static void do_stat(FILE* f) {
	struct stats	s;
	unsigned int	i;

	s.sum = 0.0;
	s.min = 2000;
	s.max = 0;
	s.total = 0;
	for (i = 0; !feof(f) && i < NUM_LINES; i++) {
		print_line(f, &s, 100);
		printf("\r");
	}
	printf("\n");
	printf("bpm: (%d, %g, %d)\n", s.min, s.sum / ((double)s.total), s.max);
}

int main(int argc, char* argv[]) {
	FILE *f;

	f = cms50d_new(argv[1]);
	assert (f != NULL);
	if (argc == 2) {
		do_stat(f);
	} else {
		int i = 0;
		struct pkt p;
		while (1) {
			if (cms50d_getpkt(f, &p)) {
				printf("%d\n", p.wav.signal);
			}
			if (i % 32 == 0) {
				fflush(stdout);
			}
		}
	}

	return 0;
}

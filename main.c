#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <assert.h>

#define NUM_LINES	2
static const char wavmap[] = "  ___...---'''``";
//static const char wavmap[] = "0123456789abcdef";

struct sync_val {
	uint8_t v : 7;
	uint8_t hi : 1;
};

struct pkt {
	uint8_t uh;
	uint8_t	cmd;	// 0x01
	uint8_t status;
	struct {
		uint8_t signal : 4; // 0xf == bad
		uint8_t _ : 2;
		uint8_t pulse : 1;	
		uint8_t hi : 1;
	} wav;
	struct {
		uint8_t graph : 4;
		uint8_t problem : 1;
		uint8_t _ : 3;
	} bar;
	struct sync_val pulse; // hit >127bpm and it overflows...
	struct sync_val spo2;
};


static int getpkt(FILE* f, struct pkt* p) {
	int c;
	while ((c = fgetc(f)) > 0) {
		// wait for first sync byte
		if (c == 0xff) {
			break;
		}
	}
	// second sync byte
	if ((c = fgetc(f) != 0xff)) {
		return 0;
	}
	return fread(p, sizeof(*p), 1, f);
}


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
		if (!getpkt(f, &p)) {
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

FILE* setup(const char *path) {
	FILE		*f;
	int		fd, bw;
	int		arg;
        struct termios	tty;

	fd = open(path, O_RDWR|O_NOCTTY);
	if (tcgetattr(fd, &tty) != 0) {
		exit(1);
	}

	// great fun
	cfsetspeed(&tty, B115200);
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_cflag &= ~(PARENB | PARODD);
	tty.c_iflag |= IXON | IXOFF;
	tty.c_lflag &= ~ICANON;
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;
	if (tcsetattr(fd, 0, &tty) != 0) {
		exit(2);
	}
	arg = TIOCM_DTR;
	ioctl(fd, TIOCMBIS, &arg);
	arg = TIOCM_RTS;
	ioctl(fd, TIOCMBIS, &arg);
	tcflush(fd, TCIOFLUSH);

	// read realtime
	bw = write(fd, "\x7d\x81\xa1", 3);
	assert(bw == 3);
	return fdopen(fd, "rb");
}

int main(int argc, char* argv[]) {
	unsigned int	i;
	struct stats	s;
	FILE		*f;

	f = setup(argv[1]);
	assert (f != NULL);

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
	return 0;
}

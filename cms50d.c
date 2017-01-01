#include <assert.h>
#include <fcntl.h> 
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "cms50d.h"

int cms50d_getpkt(FILE* f, struct pkt* p) {
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

FILE* cms50d_new(const char *path) {
	FILE		*f;
	int		fd, bw;
	int		arg;
        struct termios	tty;

	fd = open(path, O_RDWR|O_NOCTTY);
	if (tcgetattr(fd, &tty) != 0)
		goto err;

	// great fun
	cfsetspeed(&tty, B115200);
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_cflag &= ~(PARENB | PARODD);
	tty.c_iflag |= IXON | IXOFF;
	tty.c_lflag &= ~ICANON;
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;
	if (tcsetattr(fd, 0, &tty) != 0)
		goto err;
	
	arg = TIOCM_DTR;
	ioctl(fd, TIOCMBIS, &arg);
	arg = TIOCM_RTS;
	ioctl(fd, TIOCMBIS, &arg);
	tcflush(fd, TCIOFLUSH);

	// read realtime
	bw = write(fd, "\x7d\x81\xa1", 3);
	assert(bw == 3);
	return fdopen(fd, "rb");

err:
	close(fd);
	return NULL;
}

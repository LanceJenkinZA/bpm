#ifndef CMS50D_H
#define CMS50D_H

#include <stdint.h>
#include <stdio.h>

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

FILE* cms50d_new(const char *path);
int cms50d_getpkt(FILE* f, struct pkt* p);

#endif

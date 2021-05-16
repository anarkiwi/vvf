// Copyright 2021 Josh Bailey (josh@vandervecken.com)

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include <stdio.h>
#include <6502.h>
#include "vessel.h"
#include "midi.h"

#define GETMEM(x) *((unsigned char*)x)

#define SIDCREG(x) GETMEM(0xd412) = x
#define SIDFHREG(x) GETMEM(0xd40e) = x
#define SIDFLREG(x) GETMEM(0xd40f) = x

#define GLITCH1 0x01
#define GLITCH2 0x02
#define GLITCH3 0x04
#define GLITCH4 0x08
#define GLITCH5 0x10

#define C 0
#define CS 1
#define D 2
#define DS 3
#define E 4

#define CCMOD 1
#define CCALLOFF 123
#define CCRESET 121

void initvessel(void) {
  VOUT;
  VCMD(0); // reset
  VCMD(7); // enable messages for ch 9
  VW(0b01110000 | 0x08);
  VCMD(5); // enable ch 9
  VW(0x01);
  VW(0x00);
}

void initsid(void) {
  // init voice 3
  // max freq
  SIDFHREG(0xff);
  SIDFLREG(0xff);
  // noise
  SIDCREG(0x80);
  // pulse width
  GETMEM(0xd411) = 0x08;
}

void initscreen(void) {
  GETMEM(0xd011) = 0x97;
  GETMEM(0xd018) = 0x51;
  GETMEM(0xd016) = 0xd3;
  GETMEM(0xd020) = 0xfa;
  GETMEM(0xd021) = 0xfa;
}

void init(void) {
  SEI();
  initvessel();
  initsid();
  initscreen();
}

void midiloop(void) {
  register unsigned char bc = 0;
  register unsigned char b = 0;
  register unsigned char i = 0;
  register unsigned glitch = 0;
  register unsigned osc = 0;

  for (;;) {
    osc = GETMEM(0xd41b);

    if (glitch & 0x1) {
      GETMEM(0xd016) = osc;
    }

    if (glitch & 0x2) {
      GETMEM(0xd021) = osc;
    }

    if (glitch & 0x4) {
      GETMEM(0xd018) = osc;
    }

    if (glitch & 0x8) {
      GETMEM(0xd020) = osc;
    }

    if (glitch & 0x16) {
      osc |= 0x10; // avoid disabling screen.
      GETMEM(0xd011) = osc;
    }

    VIN;
    bc = VR;
    if (bc) {
      for (i = 0; i < bc; ++i) {
	buf[i] = VR;
      }
      for (i = 0; i < bc; i += 3) {
	switch (buf[i] & 0b11110000) {
	case NOTEON:
	  switch (buf[i+1]) {
	  case C:
	    glitch |= GLITCH1;
	    break;
	  case CS:
	    glitch |= GLITCH2;
	    break;
	  case D:
	    glitch |= GLITCH3;
	    break;
	  case DS:
	    glitch |= GLITCH4;
	    break;
	  case E:
	    glitch |= GLITCH5;
	    break;
	  default:
	    break;
	  }
	  break;
	case NOTEOFF:
	  switch (buf[i+1]) {
	  case C:
	    glitch &= (0xff - GLITCH1);
	    break;
	  case CS:
	    glitch &= (0xff - GLITCH2);
	    break;
	  case D:
	    glitch &= (0xff - GLITCH3);
	    break;
	  case DS:
	    glitch &= (0xff - GLITCH4);
	    break;
	  case E:
	    glitch &= (0xff - GLITCH5);
	    break;
	  default:
	    break;
	  }
	  break;
	case CC:
	  switch (buf[i+1]) {
	    case CCALLOFF:
	      glitch = 0;
	      break;
	    case CCRESET:
	      initsid();
	      initscreen();
	      break;
	    case CCMOD:
	      switch (buf[i+2]) {
		case 0:
		  SIDCREG(0x40);
		  break;
		case 1:
		  SIDCREG(0x80);
		  break;
		case 2:
		  SIDCREG(0x20);
		  break;
		case 3:
		  SIDCREG(0x10);
		  break;
	      }
	      break;
	    default:
	      break;
	  }
	  break;
       case PITCHBEND:
	  SIDFLREG(buf[i+2]);
	  break;
	default:
	  break;
	}
      }
    }
    VOUT;
  }
}

void main(void) {
  init();
  midiloop();
}

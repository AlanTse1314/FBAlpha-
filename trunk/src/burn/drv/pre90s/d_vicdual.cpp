// FB Alpha Sega Vic Dual driver module
// Based on MAME driver by Zsolt Vasvari

/*
	to do:
	  	bugtest
		sound?
*/

#include "tiles_generic.h"
#include "z80_intf.h"
#include "samples.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvI8039ROM;
static UINT8 *DrvColPROM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvVidRAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 coin_status;
static UINT8 palette_bank;
static UINT8 samurai_protection;

static UINT8 DrvJoy1[1]; // coin
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvJoy4[8];
static UINT8 DrvJoy5[8];
static UINT8 DrvDips[2];
static UINT8 DrvInputs[4];
static UINT8 DrvReset;

static struct BurnInputInfo Invho2InputList[] = {
	{"Game Select",		BIT_DIGITAL,	DrvJoy5 + 4,	"p1 fire 2"	},
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 4,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Invho2)

static struct BurnInputInfo DepthchInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy2 + 3,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Depthch)

static struct BurnInputInfo SafariInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy2 + 3,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 right"	},
	{"P1 Aim Up",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 2"	},
	{"P1 Aim Down",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 3"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Safari)

static struct BurnInputInfo FrogsInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 1,	"p1 up"		},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Frogs)

static struct BurnInputInfo AlphahoInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 4,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy5 + 4,	"p1 fire 2"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy5 + 5,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy4 + 0,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy5 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Alphaho)

static struct BurnInputInfo HeiankyoInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy5 + 5,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 4,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy4 + 4,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy5 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 fire 2"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy5 + 1,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy4 + 0,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy5 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Heiankyo)

static struct BurnInputInfo PulsarInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 4,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Pulsar)

static struct BurnInputInfo DiggerInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 7,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy2 + 6,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Digger)

static struct BurnInputInfo InvdsInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy5 + 4,	"p1 fire 3"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Invds)

static struct BurnInputInfo InvincoInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 start"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy2 + 6,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Invinco)

static struct BurnInputInfo SamuraiInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 4,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Samurai)

static struct BurnInputInfo TranqgunInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 4,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy5 + 5,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy4 + 0,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy5 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Tranqgun)

static struct BurnInputInfo HeadonInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 7,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy2 + 6,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Headon)

static struct BurnInputInfo BrdrlineInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 4,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy5 + 5,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy4 + 0,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy5 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Brdrline)

static struct BurnInputInfo SpacetrkInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy3 + 5,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy5 + 4,	"p1 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Spacetrk)

static struct BurnInputInfo CarnivalInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Carnival)

static struct BurnInputInfo CarhntdsInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 4,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy5 + 4,	"p1 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Carhntds)

static struct BurnInputInfo Headon2InputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 7,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy2 + 6,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Headon2)

static struct BurnInputInfo SspaceatInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 start"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy2 + 7,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 fire 1"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 start"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 6,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Sspaceat)

static struct BurnInputInfo SspacahoInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 4,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy5 + 4,	"p1 fire 2"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy5 + 5,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy4 + 0,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy5 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Sspacaho)

static struct BurnInputInfo CarnivalhInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 start"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy2 + 6,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Carnivalh)

static struct BurnInputInfo SpacetrkcInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy3 + 5,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy5 + 4,	"p1 fire 2"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy5 + 5,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy3 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy5 + 0,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy4 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Spacetrkc)

static struct BurnInputInfo StarrkrInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy3 + 5,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy5 + 5,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy3 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy5 + 0,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy4 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Starrkr)

static struct BurnInputInfo CarnivalcInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy5 + 5,	"p2 start"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy5 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Carnivalc)

static struct BurnInputInfo HeadonnInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 4,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy3 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 fire 1"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy5 + 5,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy4 + 0,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy5 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Headonn)

static struct BurnInputInfo SupcrashInputList[] = {
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 7,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 1"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Supcrash)

static struct BurnInputInfo NsubInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy2 + 7,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy2 + 6,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Nsub)

static struct BurnDIPInfo Invho2DIPList[]=
{
	{0x09, 0xff, 0xff, 0x0d, NULL						},
	{0x0a, 0xff, 0xff, 0x03, NULL						},

	{0   , 0xfe, 0   ,    3, "Head On 2 Lives"			},
	{0x09, 0x01, 0x03, 0x00, "2"						},
	{0x09, 0x01, 0x03, 0x01, "3"						},
	{0x09, 0x01, 0x03, 0x03, "4"						},

	{0   , 0xfe, 0   ,    2, "Invinco Lives"			},
	{0x09, 0x01, 0x0c, 0x0c, "3"						},
	{0x09, 0x01, 0x0c, 0x08, "4"						},
};

STDDIPINFO(Invho2)

static struct BurnDIPInfo DepthchDIPList[]=
{
	{0x06, 0xff, 0xff, 0xf0, NULL						},

	{0   , 0xfe, 0   ,    4, "Coinage"					},
	{0x06, 0x01, 0x30, 0x00, "4 Coins 1 Credits"		},
	{0x06, 0x01, 0x30, 0x10, "3 Coins 1 Credits"		},
	{0x06, 0x01, 0x30, 0x20, "2 Coins 1 Credits"		},
	{0x06, 0x01, 0x30, 0x30, "1 Coin  1 Credits"		},
};

STDDIPINFO(Depthch)

static struct BurnDIPInfo SafariDIPList[]=
{
	{0x09, 0xff, 0xff, 0x30, NULL						},

	{0   , 0xfe, 0   ,    4, "Coinage"					},
	{0x09, 0x01, 0x30, 0x00, "4 Coins 1 Credits"		},
	{0x09, 0x01, 0x30, 0x10, "3 Coins 1 Credits"		},
	{0x09, 0x01, 0x30, 0x20, "2 Coins 1 Credits"		},
	{0x09, 0x01, 0x30, 0x30, "1 Coin  1 Credits"		},
};

STDDIPINFO(Safari)

static struct BurnDIPInfo FrogsDIPList[]=
{
	{0x06, 0xff, 0xff, 0x08, NULL						},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x06, 0x01, 0x08, 0x00, "Off"						},
	{0x06, 0x01, 0x08, 0x08, "On"						},
};

STDDIPINFO(Frogs)

static struct BurnDIPInfo AlphahoDIPList[]=
{
	{0x0f, 0xff, 0xff, 0x00, NULL						},
	{0x10, 0xff, 0xff, 0x03, NULL						},

	{0   , 0xfe, 0   ,    2, "Head On Lives"			},
	{0x0f, 0x01, 0x08, 0x00, "3"						},
	{0x0f, 0x01, 0x08, 0x08, "4"						},

	{0   , 0xfe, 0   ,    4, "Alpha Fighter Lives"		},
	{0x10, 0x01, 0x03, 0x03, "3"						},
	{0x10, 0x01, 0x03, 0x02, "4"						},
	{0x10, 0x01, 0x03, 0x01, "5"						},
	{0x10, 0x01, 0x03, 0x00, "6"						},
};

STDDIPINFO(Alphaho)

static struct BurnDIPInfo HeiankyoDIPList[]=
{
	{0x10, 0xff, 0xff, 0x00, NULL						},
	{0x11, 0xff, 0xff, 0x00, NULL						},

	{0   , 0xfe, 0   ,    2, "2 Players Mode"			},
	{0x10, 0x01, 0x08, 0x08, "Alternating"				},
	{0x10, 0x01, 0x08, 0x00, "Simultaneous"				},

	{0   , 0xfe, 0   ,    2, "Lives"					},
	{0x11, 0x01, 0x04, 0x00, "3"						},
	{0x11, 0x01, 0x04, 0x04, "5"						},
};

STDDIPINFO(Heiankyo)

static struct BurnDIPInfo PulsarDIPList[]=
{
	{0x08, 0xff, 0xff, 0x02, NULL						},

	{0   , 0xfe, 0   ,    4, "Lives"					},
	{0x08, 0x01, 0x03, 0x00, "2"						},
	{0x08, 0x01, 0x03, 0x02, "3"						},
	{0x08, 0x01, 0x03, 0x01, "4"						},
	{0x08, 0x01, 0x03, 0x03, "5"						},
};	

STDDIPINFO(Pulsar)

static struct BurnDIPInfo DiggerDIPList[]=
{
	{0x09, 0xff, 0xff, 0x63, NULL						},

	{0   , 0xfe, 0   ,    4, "Lives"					},
	{0x09, 0x01, 0x03, 0x03, "3"						},
	{0x09, 0x01, 0x03, 0x02, "4"						},
	{0x09, 0x01, 0x03, 0x01, "5"						},
	{0x09, 0x01, 0x03, 0x00, "6"						},
};

STDDIPINFO(Digger)

static struct BurnDIPInfo InvdsDIPList[]=
{
	{0x08, 0xff, 0xff, 0x10, NULL						},
	{0x09, 0xff, 0xff, 0x07, NULL						},

	{0   , 0xfe, 0   ,    2, "Unused"					},
	{0x08, 0x01, 0x08, 0x08, "Off"						},
	{0x08, 0x01, 0x08, 0x00, "On"						},
	
	{0   , 0xfe, 0   ,    4, "Invinco Lives"			},
	{0x09, 0x01, 0x03, 0x03, "3"						},
	{0x09, 0x01, 0x03, 0x02, "4"						},
	{0x09, 0x01, 0x03, 0x01, "5"						},
	{0x09, 0x01, 0x03, 0x00, "6"						},

	{0   , 0xfe, 0   ,    4, "Deep Scan Lives"			},
	{0x09, 0x01, 0x0c, 0x08, "2"						},
	{0x09, 0x01, 0x0c, 0x04, "3"						},
	{0x09, 0x01, 0x0c, 0x00, "4"						},
	{0x09, 0x01, 0x0c, 0x0c, "5"						},
};

STDDIPINFO(Invds)

static struct BurnDIPInfo InvincoDIPList[]=
{
	{0x06, 0xff, 0xff, 0x60, NULL						},

	{0   , 0xfe, 0   ,    4, "Lives"					},
	{0x06, 0x01, 0x03, 0x00, "3"						},
	{0x06, 0x01, 0x03, 0x01, "4"						},
	{0x06, 0x01, 0x03, 0x02, "5"						},
	{0x06, 0x01, 0x03, 0x03, "6"						},
};

STDDIPINFO(Invinco)

static struct BurnDIPInfo SamuraiDIPList[]=
{
	{0x08, 0xff, 0xff, 0x0c, NULL						},

	{0   , 0xfe, 0   ,    2, "Lives"					},
	{0x08, 0x01, 0x04, 0x04, "3"						},
	{0x08, 0x01, 0x04, 0x00, "4"						},

	{0   , 0xfe, 0   ,    2, "Infinite Lives (Cheat)"	},
	{0x08, 0x01, 0x08, 0x08, "Off"						},
	{0x08, 0x01, 0x08, 0x00, "On"						},
};

STDDIPINFO(Samurai)

static struct BurnDIPInfo HeadonDIPList[]=
{
	{0x07, 0xff, 0xff, 0x00, NULL						},

	{0   , 0xfe, 0   ,    4, "Lives"					},
	{0x07, 0x01, 0x03, 0x00, "3"						},
	{0x07, 0x01, 0x03, 0x01, "4"						},
	{0x07, 0x01, 0x03, 0x02, "5"						},
	{0x07, 0x01, 0x03, 0x03, "6"						},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x07, 0x01, 0x04, 0x04, "Off"						},
	{0x07, 0x01, 0x04, 0x00, "On"						},
};

STDDIPINFO(Headon)

static struct BurnDIPInfo BrdrlineDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x05, NULL						},

	{0   , 0xfe, 0   ,    4, "Lives"					},
	{0x0e, 0x01, 0x07, 0x01, "3"						},
	{0x0e, 0x01, 0x07, 0x02, "4"						},
	{0x0e, 0x01, 0x07, 0x04, "5"						},
	{0x0e, 0x01, 0x07, 0x07, "Infinite (Cheat)"			},
		
	{0   , 0xfe, 0   ,    2, "Cabinet"					},
	{0x0e, 0x01, 0x08, 0x00, "Upright"					},
	{0x0e, 0x01, 0x08, 0x08, "Cocktail"					},

	{0   , 0xfe, 0   ,    2, "Bonus Life"				},
	{0x0e, 0x01, 0x10, 0x10, "15000"					},
	{0x0e, 0x01, 0x10, 0x00, "20000"					},
};

STDDIPINFO(Brdrline)

static struct BurnDIPInfo SpacetrkDIPList[]=
{
	{0x09, 0xff, 0xff, 0x03, NULL						},

	{0   , 0xfe, 0   ,    2, "Lives"					},
	{0x09, 0x01, 0x01, 0x01, "3"						},
	{0x09, 0x01, 0x01, 0x00, "4"						},

	{0   , 0xfe, 0   ,    2, "Bonus Life"				},
	{0x09, 0x01, 0x02, 0x00, "Off"						},
	{0x09, 0x01, 0x02, 0x02, "On"						},
};

STDDIPINFO(Spacetrk)

static struct BurnDIPInfo CarnivalDIPList[]=
{
	{0x06, 0xff, 0xff, 0x00, NULL						},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x06, 0x01, 0x10, 0x10, "Off"						},
	{0x06, 0x01, 0x10, 0x00, "On"						},
};

STDDIPINFO(Carnival)

static struct BurnDIPInfo CarhntdsDIPList[]=
{
	{0x09, 0xff, 0xff, 0x0d, NULL						},

	{0   , 0xfe, 0   ,    4, "Car Hunt Lives"			},
	{0x09, 0x01, 0x03, 0x03, "1"						},
	{0x09, 0x01, 0x03, 0x02, "2"						},
	{0x09, 0x01, 0x03, 0x01, "3"						},
	{0x09, 0x01, 0x03, 0x00, "4"						},

	{0   , 0xfe, 0   ,    4, "Deep Scan Lives"			},
	{0x09, 0x01, 0x0c, 0x08, "1"						},
	{0x09, 0x01, 0x0c, 0x04, "2"						},
	{0x09, 0x01, 0x0c, 0x00, "3"						},
	{0x09, 0x01, 0x0c, 0x0c, "4"						},
};

STDDIPINFO(Carhntds)

static struct BurnDIPInfo Headon2DIPList[]=
{
	{0x08, 0xff, 0xff, 0x0a, NULL						},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x08, 0x01, 0x02, 0x00, "Off"						},
	{0x08, 0x01, 0x02, 0x02, "On"						},

	{0   , 0xfe, 0   ,    4, "Lives"					},
	{0x08, 0x01, 0x18, 0x08, "4"						},
	{0x08, 0x01, 0x18, 0x10, "5"						},
	{0x08, 0x01, 0x18, 0x18, "5"						},
	{0x08, 0x01, 0x18, 0x00, "6"						},
};

STDDIPINFO(Headon2)

static struct BurnDIPInfo SspaceatDIPList[]=
{
	{0x0a, 0xff, 0xff, 0x6e, NULL						},

	{0   , 0xfe, 0   ,    2, "Bonus Life For Final UFO"	},
	{0x0a, 0x01, 0x01, 0x01, "Off"						},
	{0x0a, 0x01, 0x01, 0x00, "On"						},

	{0   , 0xfe, 0   ,    4, "Lives"					},
	{0x0a, 0x01, 0x0e, 0x0e, "3"						},
	{0x0a, 0x01, 0x0e, 0x0c, "4"						},
	{0x0a, 0x01, 0x0e, 0x0a, "5"						},
	{0x0a, 0x01, 0x0e, 0x06, "6"						},

	{0   , 0xfe, 0   ,    2, "Bonus Life"				},
	{0x0a, 0x01, 0x10, 0x00, "10000"					},
	{0x0a, 0x01, 0x10, 0x10, "15000"					},

	{0   , 0xfe, 0   ,    2, "Credits Display"			},
	{0x0a, 0x01, 0x80, 0x80, "Off"						},
	{0x0a, 0x01, 0x80, 0x00, "On"						},
};

STDDIPINFO(Sspaceat)

static struct BurnDIPInfo SspacahoDIPList[]=
{
	{0x0f, 0xff, 0xff, 0x00, NULL						},
	{0x10, 0xff, 0xff, 0x03, NULL						},

	{0   , 0xfe, 0   ,    2, "Space Attack Final UFO Bonus"	},
	{0x0f, 0x01, 0x01, 0x01, "Off"						},
	{0x0f, 0x01, 0x01, 0x00, "On"						},

	{0   , 0xfe, 0   ,    2, "Space Attack Bonus Life"	},
	{0x0f, 0x01, 0x04, 0x00, "10000"					},
	{0x0f, 0x01, 0x04, 0x04, "15000"					},

	{0   , 0xfe, 0   ,    2, "Head On Lives"			},
	{0x0f, 0x01, 0x08, 0x00, "3"						},
	{0x0f, 0x01, 0x08, 0x08, "4"						},

	{0   , 0xfe, 0   ,    4, "Space Attack Lives"		},
	{0x10, 0x01, 0x03, 0x03, "3"						},
	{0x10, 0x01, 0x03, 0x02, "4"						},
	{0x10, 0x01, 0x03, 0x01, "5"						},
	{0x10, 0x01, 0x03, 0x00, "6"						},
};

STDDIPINFO(Sspacaho)

static struct BurnDIPInfo SpacetrkcDIPList[]=
{
	{0x10, 0xff, 0xff, 0x03, NULL						},

	{0   , 0xfe, 0   ,    2, "Lives"					},
	{0x10, 0x01, 0x01, 0x01, "3"						},
	{0x10, 0x01, 0x01, 0x00, "4"						},

	{0   , 0xfe, 0   ,    2, "Bonus Life"				},
	{0x10, 0x01, 0x02, 0x00, "Off"						},
	{0x10, 0x01, 0x02, 0x02, "On"						},
};

STDDIPINFO(Spacetrkc)

static struct BurnDIPInfo StarrkrDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x11, NULL						},

	{0   , 0xfe, 0   ,    4, "Lives"					},
	{0x0e, 0x01, 0x07, 0x01, "3"						},
	{0x0e, 0x01, 0x07, 0x02, "4"						},
	{0x0e, 0x01, 0x07, 0x04, "5"						},
	{0x0e, 0x01, 0x07, 0x07, "Infinite (Cheat)"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"					},
	{0x0e, 0x01, 0x08, 0x00, "Upright"					},
	{0x0e, 0x01, 0x08, 0x08, "Cocktail"					},

	{0   , 0xfe, 0   ,    2, "Bonus Life"				},
	{0x0e, 0x01, 0x10, 0x10, "15000"					},
	{0x0e, 0x01, 0x10, 0x00, "20000"					},
};

STDDIPINFO(Starrkr)

static struct BurnDIPInfo CarnivalcDIPList[]=
{
	{0x0a, 0xff, 0xff, 0x00, NULL						},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x0a, 0x01, 0x10, 0x10, "Off"						},
	{0x0a, 0x01, 0x10, 0x00, "On"						},
};

STDDIPINFO(Carnivalc)

static struct BurnDIPInfo HeadonnDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x00, NULL						},

	{0   , 0xfe, 0   ,    2, "Lives"					},
	{0x0e, 0x01, 0x08, 0x00, "3"						},
	{0x0e, 0x01, 0x08, 0x08, "4"						},
};

STDDIPINFO(Headonn)

static struct BurnDIPInfo SupcrashDIPList[]=
{
	{0x06, 0xff, 0xff, 0x04, NULL						},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x06, 0x01, 0x40, 0x40, "Off"						},
	{0x06, 0x01, 0x40, 0x00, "On"						},

	{0   , 0xfe, 0   ,    2, "Rom Test"					},
	{0x06, 0x01, 0x04, 0x04, "Off"						},
	{0x06, 0x01, 0x04, 0x00, "On"						},
};

STDDIPINFO(Supcrash)

// 1933560 cycles per sec
// 32226 / frame
// 262 scanlines
// 123 cycles / scanline
// 328 pixel wide 
// hbstart at 256
// hsend at 304
static inline INT32 get_vcounter()
{
	INT32 hpos = ZetTotalCycles() % (123 + 1);

	hpos = (hpos * 328) / 123; // 328=pxl/line, 123=cycles/line

	INT32 vcounter = ZetTotalCycles() / 123; // vpos

	if (hpos >= 0x130)
		return (vcounter + 1) % 262;

	return vcounter;
}

static inline INT32 get_64v(INT32 tval)
{
	return ((get_vcounter() >> 6) & 0x01) ? tval : 0;
}

static inline INT32 get_vblank_comp(INT32 tval)
{
	return (get_vcounter() < 224) ? tval : 0;
}

static inline INT32 get_composite_blank_comp(INT32 tval)
{
	INT32 hpos = ((ZetTotalCycles() % (123 + 1)) * 328) / 123;

	return (get_vblank_comp(1) && (hpos >= 0 && hpos < 256)) ? tval : 0;
}

static inline INT32 get_timer_value(INT32 tval)
{
	return ((ZetTotalCycles() / 3867) & 1) ? tval : 0; // 3867.12 cycles
}

static inline INT32 get_coin_status(INT32 tval)
{
	return (coin_status) ? tval : 0;
}

static void __fastcall invho2_write_port(UINT16 port, UINT8 data)
{
//	if (offset & 0x01)  invho2_audio_w(space, 0, data);
//	if (offset & 0x02)  invinco_audio_w(space, 0, data);
	if (port & 0x08) coin_status = 1;
	if (port & 0x40) palette_bank = data & 0x03;
}

static UINT8 __fastcall invho2_read_port(UINT16 port)
{
	switch (port & 3)
	{
		case 0x00:
			return (DrvInputs[0] & ~0x0c) | ((DrvDips[0] & 0x01) ? 4 : 0);

		case 0x01:
			return (DrvInputs[1] & ~0x0c) | ((DrvDips[0] & 0x02) ? 4 : 0) | get_composite_blank_comp(8);

		case 0x02:
			return (DrvInputs[2] & ~0x0c) | ((DrvDips[0] & 0x04) ? 4 : 0) | get_timer_value(8);

		case 0x03:
			return (DrvInputs[3] & ~0x0c) | ((DrvDips[0] & 0x08) ? 4 : 0) | get_coin_status(8);
	}

	return 0;
}

static void __fastcall depthch_write_port(UINT16 port, UINT8 /*data*/)
{
	if (port & 1) coin_status = 1;
//	if (port & 4) depthch_audio_w
}

static UINT8 __fastcall depthch_read_port(UINT16 port)
{
	if (port & 1) return (DrvInputs[0] & 0xcf) | (DrvDips[0] & 0x30);
	if (port & 8) return 0x7e | get_64v(1) | (coin_status ? 0x80 : 0);

	return 0;
}

static void __fastcall safari_write_port(UINT16 port, UINT8 /*data*/)
{
	if (port & 0x01) coin_status = 1;
//	if (port & 0x02) // sound
}

static UINT8 __fastcall safari_read_port(UINT16 port)
{
	if (port & 1) return DrvInputs[0];
	if (port & 8) return 0x4e | (DrvDips[0] & 0x30) | get_64v(1) | get_coin_status(0x80);

	return 0;
}

static void __fastcall frogs_write_port(UINT16 port, UINT8 /*data*/)
{
	if (port & 0x01) coin_status = 1;
//	if (port & 0x02) // frogs_audio_w(space, 0, data);
}

static UINT8 __fastcall frogs_read_port(UINT16 port)
{
	if (port & 0x01) return (DrvInputs[0] & 0xf7) | (DrvDips[0] & 0x08);
	if (port & 0x08) return 0x7e | get_64v(1) | get_coin_status(0x80);

	return 0;
}

static void __fastcall alphaho_write_port(UINT16 port, UINT8 data)
{
//	if (port & 0x01) // audio
//	if (port & 0x02) // audio
	if (port & 0x08) coin_status = 1;
	if (port & 0x40) palette_bank = data & 3;
}

static UINT8 __fastcall alphaho_read_port(UINT16 port)
{
	switch (port & 3)
	{
		case 0x00:
			return (DrvInputs[0] & ~0x0c) | ((DrvDips[1] & 1) ? 4 : 0) | (DrvDips[0] & 8);

		case 0x01:
			return (DrvInputs[1] & ~0x0c) | ((DrvDips[1] & 2) ? 4 : 0) | get_composite_blank_comp(8);

		case 0x02:
			return (DrvInputs[2] & ~0x08) | get_timer_value(8);

		case 0x03:
			return (DrvInputs[3] & ~0x0c) | get_coin_status(8);
	}

	return 0;
}

static void __fastcall heiankyo_write_port(UINT16 port, UINT8/* data*/)
{
//	if (port & 1) // audio
//	if (port & 2) // audio
	if (port & 8) coin_status = 1;
}

static UINT8 __fastcall heiankyo_read_port(UINT16 port)
{
	switch (port & 3)
	{
		case 0x00:
			return (DrvInputs[0] & ~0x0c) | (DrvDips[0] & 0x0c);

		case 0x01:
			return (DrvInputs[1] & ~0x0c) | get_composite_blank_comp(8);

		case 0x02:
			return (DrvInputs[2] & ~0x2a) | get_timer_value(8);

		case 0x03:
			return (DrvInputs[3] & ~0x0c) | (DrvDips[1] & 0x04) | get_coin_status(8);
	}

	return 0;
}

static void __fastcall pulsar_write_port(UINT16 port, UINT8 data)
{
//	if (port & 0x03) // audio
	if (port & 0x08) coin_status = 1;
	if (port & 0x40) palette_bank = data & 3;
}

static UINT8 __fastcall pulsar_read_port(UINT16 port)
{
	switch (port & 3)
	{
		case 0x00:
			return (DrvInputs[0] & ~0x0c) | ((DrvDips[0] & 1) ? 4 : 0);

		case 0x01:
			return (DrvInputs[1] & ~0x0c) | ((DrvDips[0] & 2) ? 4 : 0) | get_composite_blank_comp(8);

		case 0x02:
			return (DrvInputs[2] & ~0x0c) | get_timer_value(8);

		case 0x03:
			return (DrvInputs[3] & ~0x0c) | get_coin_status(8);
	}

	return 0;
}

static void __fastcall digger_write_port(UINT16 port, UINT8 data)
{
	if (port & 0x01) coin_status = 1;
//	if (port & 0x02) // sound
	if (port & 0x04) palette_bank = data & 3;
//	if (port & 0x18) nop
}

static UINT8 __fastcall digger_read_port(UINT16 port)
{
	UINT8 data = 0xff;

	if (port & 0x01) data &= DrvInputs[0];
	if (port & 0x02) data &= 0xff;
	if (port & 0x04) data &= DrvDips[0];
	if (port & 0x08) data &= 0x7e | get_coin_status(0x80) | get_composite_blank_comp(1);

	return data;
}

static void __fastcall invds_write_port(UINT16 port, UINT8 data)
{
//	if (port & 0x01) // audio
//	if (port & 0x02) // audio
	if (port & 0x08) coin_status = 1;
	if (port & 0x40) palette_bank = data & 3;
}

static UINT8 __fastcall invds_read_port(UINT16 port)
{
	switch (port & 3)
	{
		case 0x00:
			return (DrvInputs[0] & ~0x0c) | ((DrvDips[0] & 1) ? 4 : 0);

		case 0x01:
			return (DrvInputs[1] & ~0x0c) | ((DrvDips[0] & 2) ? 4 : 0) | get_composite_blank_comp(8);

		case 0x02:
			return (DrvInputs[2] & ~0x0c) | ((DrvDips[0] & 4) ? 4 : 0) | get_timer_value(8);

		case 0x03:
			return (DrvInputs[3] & ~0x0c) | ((DrvDips[0] & 8) ? 4 : 0) | get_coin_status(8);
	}

	return 0;
}

static void __fastcall invinco_write_port(UINT16 port, UINT8 data)
{
	if (port & 0x01) coin_status = 1;
//	if (port & 0x02) // audio
	if (port & 0x04) palette_bank = data & 3;
}

static UINT8 __fastcall invinco_read_port(UINT16 port)
{
	if (port & 0x08) return 0x7e | get_composite_blank_comp(1) | get_coin_status(0x80);
	if (port & 0x02) return DrvDips[0];
	if (port & 0x01) return DrvInputs[0];

	return 0;
}

static void __fastcall samurai_write(UINT16 address, UINT8 data)
{
	if (address < 0x8000) {
		samurai_protection = 0;
		if (data == 0xab) {
			samurai_protection = 0x02;
		} else if (data == 0x1d) {
			samurai_protection = 0x0c;
		}
	}
}

static void __fastcall samurai_write_port(UINT16 port, UINT8 data)
{
//	if (port & 0x02) // audio
	if (port & 0x08) coin_status = 1;
	if (port & 0x40) palette_bank = data & 3;
}

static UINT8 __fastcall samurai_read_port(UINT16 port)
{
	switch (port & 3)
	{
		case 0x00:
			return (DrvInputs[0] & ~0x0c) | (DrvDips[0] & 0x0c);

		case 0x01:
			return (DrvInputs[1] & ~0x0e) | (samurai_protection & 2) | get_composite_blank_comp(8);

		case 0x02:
			return (DrvInputs[2] & ~0x0e) | ((samurai_protection >> 1) & 2) | get_timer_value(8);

		case 0x03:
			return (DrvInputs[3] & ~0x0e) | ((samurai_protection >> 2) & 2) | get_coin_status(8);
	}

	return 0;
}

static void __fastcall tranqgun_write_port(UINT16 port, UINT8 data)
{
//	if (port & 0x01) // audio
	if (port & 0x02) palette_bank = data & 3;
	if (port & 0x08) coin_status = 1;
}

static UINT8 __fastcall tranqgun_read_port(UINT16 port)
{
	switch (port & 3)
	{
		case 0x00:
			return (DrvInputs[0] & ~0x0c);

		case 0x01:
			return (DrvInputs[1] & ~0x0c) | get_vblank_comp(8);

		case 0x02:
			return (DrvInputs[2] & ~0x0c) | get_timer_value(8);

		case 0x03:
			return (DrvInputs[3] & ~0x0c) | get_coin_status(8);
	}

	return 0;
}

static void __fastcall headon_write_port(UINT16 port, UINT8 /*data*/)
{
	if (port & 0x01) coin_status = 1;
//	if (port & 0x02) audio
//	if (port & 0x04) // palette bank?
}

static UINT8 __fastcall carnivalh_read_port(UINT16 port)
{
	if (port & 0x08) return 0x7e | get_64v(1) | get_coin_status(0x80);
	if (port & 0x01) return DrvInputs[0];

	return 0;
}

static UINT8 __fastcall headon_read_port(UINT16 port)
{
	if (port & 0x08) return 0x7a | get_64v(1) | get_coin_status(0x80); // ~4 for headonmz
	if (port & 0x01) return (DrvInputs[0] & 0xf8) | (DrvDips[0] & 0x07);

	return 0;
}

static UINT8 __fastcall supcrash_read_port(UINT16 port)
{
	if (port & 0x08) return 0x7b | (DrvDips[0] & 0x04) | get_coin_status(0x80);
	if (port & 0x01) return (DrvInputs[0] & 0xbf) | (DrvDips[0] & 0x40);

	return 0;
}

static void __fastcall brdrline_write_port(UINT16 port, UINT8 data)
{
//	if (port & 0x01) // audio
	if (port & 0x02) palette_bank = data & 3; /* audio */
	if (port & 0x08) coin_status = 1;
}

static UINT8 __fastcall brdrline_read_port(UINT16 port)
{
	switch (port & 3)
	{
		case 0x00:
			return (DrvInputs[0] & ~0x0c) | ((DrvDips[0] & 1) ? 4 : 0) | (DrvDips[0] & 8);

		case 0x01:
			return (DrvInputs[1] & ~0x0c) | ((DrvDips[0] & 2) ? 4 : 0) | get_vblank_comp(8);

		case 0x02:
			return (DrvInputs[2] & ~0x0c) | ((DrvDips[0] & 4) ? 4 : 0) | get_64v(8);

		case 0x03:
			return (DrvInputs[3] & ~0x0c) | ((DrvDips[0] & 8) ? 4 : 0) | get_coin_status(8);
	}

	return 0;
}

static void __fastcall spacetrk_write_port(UINT16 port, UINT8 data)
{
	if (port & 0x03) // audio
	if (port & 0x08) coin_status = 1;
	if (port & 0x40) palette_bank = data & 3;
}

static UINT8 __fastcall spacetrk_read_port(UINT16 port)
{
	switch (port & 3)
	{
		case 0x00:
			return (DrvInputs[0] & ~0x0c) | ((DrvDips[0] & 1) ? 4 : 0);

		case 0x01:
			return (DrvInputs[1] & ~0x0c) | get_composite_blank_comp(8);

		case 0x02:
			return (DrvInputs[2] & ~0x0c) | ((DrvDips[0] & 2) ? 4 : 0) | get_timer_value(8);

		case 0x03:
			return (DrvInputs[3] & ~0x0c) | get_coin_status(8);
	}

	return 0;
}

static void __fastcall carnival_write_port(UINT16 port, UINT8 data)
{
//	if (port & 0x03) // audio
	if (port & 0x08) coin_status = 1;
	if (port & 0x40) palette_bank = data & 3;
}

static UINT8 __fastcall carnival_read_port(UINT16 port)
{
	switch (port & 3)
	{
		case 0x00:
			return (DrvInputs[0] & ~0x1c) | (DrvDips[0] & 0x10);

		case 0x01:
			return (DrvInputs[1] & ~0x0c) | get_composite_blank_comp(8);

		case 0x02:
			return (DrvInputs[2] & ~0x0c) | get_timer_value(8);

		case 0x03:
			return (DrvInputs[3] & ~0x0c) | get_coin_status(8);
	}

	return 0;
}

static void __fastcall carhntds_write_port(UINT16 port, UINT8 data)
{
//	if (port & 0x03) // audio
	if (port & 0x08) coin_status = 1;
	if (port & 0x40) palette_bank = data & 3;
}

static UINT8 __fastcall carhntds_read_port(UINT16 port)
{
	switch (port & 3)
	{
		case 0x00:
			return (DrvInputs[0] & ~0x0c) | ((DrvDips[0] & 1) ? 4 : 0);

		case 0x01:
			return (DrvInputs[1] & ~0x0c) | ((DrvDips[0] & 2) ? 4 : 0) | get_composite_blank_comp(8);

		case 0x02:
			return (DrvInputs[2] & ~0x0c) | ((DrvDips[0] & 4) ? 4 : 0) | get_timer_value(8);

		case 0x03:
			return (DrvInputs[3] & ~0x0c) | ((DrvDips[0] & 8) ? 4 : 0) | get_coin_status(8);
	}

	return 0;
}

static void __fastcall headon2_write_port(UINT16 port, UINT8 /*data*/)
{
	if (port & 1) coin_status = 1;
//	if (port & 2) // audio
}

static UINT8 __fastcall headon2_read_port(UINT16 port)
{
	UINT8 ret = 0xff;

	palette_bank = 3; // hack - only last back of color prom is used

	if (port & 0x01) ret &= DrvInputs[0];
	if (port & 0x04) ret &= 0xe7 | (DrvDips[0] & 0x18);
	if (port & 0x08) ret &= 0x7d | (DrvDips[0] & 0x02) | get_coin_status(0x80);

	return ret;
}

static UINT8 __fastcall car2_read_port(UINT16 port)
{
	UINT8 ret = 0xff;

	if (port & 0x01) ret &= DrvInputs[0] ^ 0xfc;
	if (port & 0x04) ret &= (DrvDips[0] & 0x18);
	if (port & 0x08) ret &= 0x7d | (DrvDips[0] & 0x02) | get_coin_status(0x80);

	return ret;
}

static void __fastcall sspaceat_write_port(UINT16 port, UINT8 /*data*/)
{
	if (port & 0x01) coin_status = 1;
//	if (port & 0x02) // audio
//	if (port & 0x04) // palette_bank (not used)
}

static UINT8 __fastcall sspaceat_read_port(UINT16 port)
{
	if (port & 0x01) return DrvInputs[0];
	if (port & 0x02) return DrvDips[0];
	if (port & 0x08) return 0x7e | get_timer_value(1) | get_coin_status(0x80);

	return 0;
}

static void __fastcall sspacaho_write_port(UINT16 port, UINT8 data)
{
//	if (port & 0x03) // audio
	if (port & 0x08) coin_status = 1;
	if (port & 0x40) palette_bank = data & 0x03;
}

static UINT8 __fastcall sspacaho_read_port(UINT16 port)
{
	switch (port & 3)
	{
		case 0x00:
			return (DrvInputs[0] & 0xf3) | ((DrvDips[1] & 1) ? 4 : 0) | (DrvDips[0] & 0x08);

		case 0x01:
			return (DrvInputs[1] & 0xf3) | ((DrvDips[1] & 2) ? 4 : 0) | get_composite_blank_comp(8);

		case 0x02:
			return (DrvInputs[2] & 0xf3) | (DrvDips[0] & 4) | get_timer_value(8);

		case 0x03:
			return (DrvInputs[3] & 0xf3) | ((DrvDips[0] & 1) ? 4 : 0) | get_coin_status(8);
	}

	return 0;
}

static void __fastcall headonn_write_port(UINT16 port, UINT8 data)
{
//	if (port & 0x01) // audio
	if (port & 0x02) palette_bank = (data & 3) ^ 1;
	if (port & 0x08) coin_status = 1;
}

static UINT8 __fastcall headonn_read_port(UINT16 port)
{
	switch (port & 3)
	{
		case 0x00:
			return (DrvInputs[0] & 0xf7) | (DrvDips[0] & 0x08);

		case 0x01:
		case 0x02:
			return DrvInputs[port & 3];

		case 0x03:
			return (DrvInputs[3] & 0xf7) | get_coin_status(8);
	}

	return 0;
}

static void __fastcall nsub_write_port(UINT16 port, UINT8 data)
{
	if (port & 0x01) coin_status = 1;
//	if (port & 0x02) // audio?
	if (port & 0x04) palette_bank = data & 3;
}

static UINT8 __fastcall nsub_read_port(UINT16 port)
{
	if (port & 0x08) return 0x7e | get_composite_blank_comp(1) | get_coin_status(0x80);
	if (port & 0x01) return DrvInputs[0];

	return 0;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnSampleReset();

	coin_status = 0;
	palette_bank = 0;
	samurai_protection = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM		= Next; Next += 0x008000;

	DrvI8039ROM		= Next; Next += 0x000400;

	DrvColPROM		= Next; Next += 0x000040;

	DrvPalette		= (UINT32*)Next; Next += 0x0008 * sizeof(UINT32);

	AllRam			= Next;

	DrvZ80RAM		= Next; Next += 0x001000; // safari
	DrvVidRAM		= Next; Next += 0x001000;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static INT32 DrvLoadRoms()
{
	char* pRomName;
	struct BurnRomInfo ri;
	UINT8 *zLoad = DrvZ80ROM;

	memset (DrvColPROM, 0xe0, 0x40); // fill w/black & white index

	for (INT32 i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++)
	{
		BurnDrvGetRomInfo(&ri, i);

		if ((ri.nType & BRF_PRG) && ((ri.nType & 0x1f) == 1)) // normal load
		{
			 // carhntds' first rom is double sized with half at 0 and half at 4000
			if ((zLoad - DrvZ80ROM) == 0x800 && ri.nLen == 0x400 && i == 1) {
				memcpy (DrvZ80ROM + 0x4000, DrvZ80ROM + 0x0400, 0x0400);
				zLoad -= 0x400;
			}

			if (BurnLoadRom(zLoad, i, 1)) return 1;
			zLoad += ri.nLen;
			continue;
		}

		if ((ri.nType & BRF_PRG) && ((ri.nType & 0x1f) == 2)) // load nibbles
		{
			if (BurnLoadRom(zLoad,           i + 0, 1)) return 1;
			if (BurnLoadRom(zLoad + ri.nLen, i + 1, 1)) return 1;

			for (UINT32 j = 0; j < ri.nLen; j++) {
				zLoad[j] = (zLoad[j] & 0xf) + (zLoad[j + ri.nLen] << 4);
			}

			zLoad += ri.nLen;
			i++;
			continue;
		}

		if ((ri.nType & BRF_GRA) && ((ri.nType & 0x1f) == 1)) // color prom
		{
			if (BurnLoadRom(DrvColPROM, i, 1)) return 1;
			continue;
		}

		if ((ri.nType & BRF_PRG) && ((ri.nType & 0x1f) == 3)) // i8039 rom
		{
			if (BurnLoadRom(DrvI8039ROM, i, 1)) return 1;
			continue;
		}
	}

	// modify prom contents to make drawing easier
	for (INT32 i = 0; i < 0x40; i++) {
		DrvColPROM[i] = (DrvColPROM[i] >> 1) & 0x77;
	}

	return 0;
}

static INT32 DrvInit(INT32 romsize, INT32 rambase, INT32 has_z80ram, void (__fastcall *wp)(UINT16,UINT8), UINT8 (__fastcall *rp)(UINT16), void (*z80_cb)(), void (*rom_cb)())
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	if (DrvLoadRoms()) return 1;

	ZetInit(0);
	ZetOpen(0);

	{
		for (INT32 i = 0x0000; i < 0x8000; i+= romsize) {
			ZetMapMemory(DrvZ80ROM,			0x0000 + i, (romsize - 1) + i, MAP_ROM);
		}

		for (INT32 i = 0xf000; i >= rambase; i -= 0x1000) {
			ZetMapMemory(DrvVidRAM,			0x0000 + i, 0x0fff + i, MAP_RAM);
		}

		if (rambase == 0xc000 && has_z80ram) {
			for (INT32 i = 0x8000; i < 0xc000; i+= 0x1000) {
				ZetMapMemory(DrvZ80RAM,		0x0000 + i, 0x0fff + i, MAP_RAM);
			}
		}

		ZetSetOutHandler(wp);
		ZetSetInHandler(rp);
	}

	if (z80_cb) {
		z80_cb();
	}

	ZetClose();

	if (rom_cb) {
		rom_cb();
	}

	BurnSampleInit(0);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();
	BurnSampleExit();

	BurnFree(AllMem);

	return 0;
}

static void DrvCreatePalette()
{
	for (INT32 i = 0; i < 8; i++) {
		DrvPalette[i] = BurnHighCol((i & 4) ? 0xff : 0, (i & 1) ? 0xff : 0, (i & 2) ? 0xff : 0, 0);
	}
}

static void draw_layer()
{
	INT32 is_bw = 0; // color for now

	UINT8 x = 0;
	UINT8 y = 0;
	UINT8 video_data = 0;
	UINT8 back_pen = 0;
	UINT8 fore_pen = 0;
	UINT8 *prom = DrvColPROM + (palette_bank * 8) + (is_bw ? 0x20 : 0);

	while (1)
	{
		if ((x & 0x07) == 0)
		{
			UINT16 offs = ((y >> 3) << 5) | (x >> 3);
			UINT8 char_code = DrvVidRAM[offs];

			offs = (char_code << 3) | (y & 0x07);
			video_data = DrvVidRAM[0x800 + offs];

			offs = (char_code >> 5);
			back_pen = prom[offs] & 0xf;
			fore_pen = prom[offs] >> 4;
		}

		pTransDraw[(y * nScreenWidth) + x] = (video_data & 0x80) ? fore_pen : back_pen;

		video_data <<= 1;
		x++;

		if (x == 0) {
			if (y >= nScreenHeight) {
				break;
			}

			y++;
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvCreatePalette();
		DrvRecalc = 0;
	}

	draw_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetNewFrame();

	{
		memset (DrvInputs, 0xff, 4);

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy5[i] & 1) << i;
		}
	}

	INT32 nTotalCycles = 1933560 / 60;

	ZetOpen(0);

	if (DrvJoy1[0] & 1) {
		ZetReset();
		ZetRun(75); // give some cycles for coin to be read
	}

	coin_status = 0; // clear coin status (no coin on hard reset)
	ZetRun(nTotalCycles - ZetTotalCycles());
	ZetClose();

	if (pBurnSoundOut) {
		BurnSampleRender(pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		BurnDrvRedraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029702;
	}

	if (nAction & ACB_VOLATILE) {
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
		BurnSampleScan(nAction, pnMin);

		SCAN_VAR(coin_status);
		SCAN_VAR(palette_bank);
		SCAN_VAR(samurai_protection);
	}

	return 0;
}


// Depthcharge

static struct BurnRomInfo depthchRomDesc[] = {
	{ "50a",				0x0400, 0x56c5ffed, 1 | BRF_PRG | BRF_ESS }, //  0 I8080 Code
	{ "51a",				0x0400, 0x695eb81f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "52",					0x0400, 0xaed0ba1b, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "53",					0x0400, 0x2ccbd2d0, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "54a",				0x0400, 0x1b7f6a43, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "55a",				0x0400, 0x9fc2eb41, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "316-0043.u87",		0x0020, 0xe60a7960, 0 | BRF_OPT },           //  6 Unused PROMs
	{ "316-0042.u88",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           //  7
};

STD_ROM_PICK(depthch)
STD_ROM_FN(depthch)

static struct BurnSampleInfo depthchSampleDesc[] = {
	{ "bonus", SAMPLE_NOLOOP },
	{ "longex", SAMPLE_NOLOOP },
	{ "shortex", SAMPLE_NOLOOP },
	{ "sonar", SAMPLE_NOLOOP },
	{ "spray", SAMPLE_NOLOOP },
	{ "", 0 }
};

STD_SAMPLE_PICK(depthch)
STD_SAMPLE_FN(depthch)

static INT32 DepthchInit()
{
	return DrvInit(0x4000, 0x8000, 0, depthch_write_port, depthch_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvDepthch = {
	"depthch", NULL, NULL, "depthch", "1977",
	"Depthcharge\0", "No sound", "Gremlin", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, depthchRomInfo, depthchRomName, depthchSampleInfo, depthchSampleName, DepthchInputInfo, DepthchDIPInfo,
	DepthchInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Depthcharge (older)

static struct BurnRomInfo depthchoRomDesc[] = {
	{ "316-0025.u63",		0x0400, 0xbec75b9c, 2 | BRF_PRG | BRF_ESS }, //  0 I8080 Code
	{ "316-0022.u51",		0x0400, 0x977b7889, 2 | BRF_PRG | BRF_ESS }, //  1
	{ "316-0030.u89",		0x0400, 0x9e2bbb45, 2 | BRF_PRG | BRF_ESS }, //  2
	{ "316-0028.u77",		0x0400, 0x597ae441, 2 | BRF_PRG | BRF_ESS }, //  3
	{ "316-0026.u64",		0x0400, 0x61cc0802, 2 | BRF_PRG | BRF_ESS }, //  4
	{ "316-0023.u52",		0x0400, 0x9244b613, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "316-0031.u90",		0x0400, 0x861ffed1, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "316-0029.u78",		0x0400, 0x53178634, 2 | BRF_PRG | BRF_ESS }, //  7
	{ "316-0027.u65",		0x0400, 0x4eecfc70, 2 | BRF_PRG | BRF_ESS }, //  8
	{ "316-0024.u53",		0x0400, 0xa9f55883, 2 | BRF_PRG | BRF_ESS }, //  9
	{ "316-0049.u91",		0x0400, 0xdc7eff35, 2 | BRF_PRG | BRF_ESS }, // 10
	{ "316-0048.u79",		0x0400, 0x6e700621, 2 | BRF_PRG | BRF_ESS }, // 11

	{ "316-0013.u27",		0x0020, 0x690ef530, 0 | BRF_OPT },           // 12 Unused PROMs
	{ "316-0014.u28",		0x0020, 0x7b7a8492, 0 | BRF_OPT },           // 13
};

STD_ROM_PICK(depthcho)
STD_ROM_FN(depthcho)

struct BurnDriver BurnDrvDepthcho = {
	"depthcho", "depthch", NULL, "depthch", "1977",
	"Depthcharge (older)\0", "No sound", "Gremlin", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, depthchoRomInfo, depthchoRomName, depthchSampleInfo, depthchSampleName, DepthchInputInfo, DepthchDIPInfo,
	DepthchInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Sub Hunter (Gremlin / Taito)

static struct BurnRomInfo subhuntRomDesc[] = {
	{ "dp04.u63",			0x0400, 0x0ace1aef, 2 | BRF_PRG | BRF_ESS }, //  0 I8080 Code
	{ "dp01.u51",			0x0400, 0xda9e835b, 2 | BRF_PRG | BRF_ESS }, //  1
	{ "dp10.u89",			0x0400, 0xde752f20, 2 | BRF_PRG | BRF_ESS }, //  2
	{ "316-0028.u77",		0x0400, 0x597ae441, 2 | BRF_PRG | BRF_ESS }, //  3
	{ "dp05.u64",			0x0400, 0x1c0530cf, 2 | BRF_PRG | BRF_ESS }, //  4
	{ "316-0023.u52",		0x0400, 0x9244b613, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "dp11.u90",			0x0400, 0x0007044a, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "dp08.u78",			0x0400, 0x4d4e3ec8, 2 | BRF_PRG | BRF_ESS }, //  7
	{ "dp06.u65",			0x0400, 0x63e1184b, 2 | BRF_PRG | BRF_ESS }, //  8
	{ "dp03.u53",			0x0400, 0xd70dbfd8, 2 | BRF_PRG | BRF_ESS }, //  9
	{ "dp12.u91",			0x0400, 0x170d7718, 2 | BRF_PRG | BRF_ESS }, // 10
	{ "dp09.u79",			0x0400, 0x97466803, 2 | BRF_PRG | BRF_ESS }, // 11

	{ "316-0013.u27",		0x0020, 0x690ef530, 0 | BRF_OPT },           // 12 Unused PROMs
	{ "316-0014.u28",		0x0020, 0x7b7a8492, 0 | BRF_OPT },           // 13
};

STD_ROM_PICK(subhunt)
STD_ROM_FN(subhunt)

struct BurnDriver BurnDrvSubhunt = {
	"subhunt", "depthch", NULL, "depthch", "1977",
	"Sub Hunter (Gremlin / Taito)\0", "No sound", "Gremlin (Taito license)", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, subhuntRomInfo, subhuntRomName, depthchSampleInfo, depthchSampleName, DepthchInputInfo, DepthchDIPInfo,
	DepthchInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Invinco / Head On 2

static struct BurnRomInfo invho2RomDesc[] = {
	{ "271b.u33",			0x0400, 0x44356a73, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "272b.u32",			0x0400, 0xbd251265, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "273b.u31",			0x0400, 0x2fc80cd9, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "274b.u30",			0x0400, 0x4fac4210, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "275b.u29",			0x0400, 0x85af508e, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "276b.u28",			0x0400, 0xe305843a, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "277b.u27",			0x0400, 0xb6b4221e, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "278b.u26",			0x0400, 0x74d42250, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "279b.u8",			0x0400, 0x8d30a3e0, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "280b.u7",			0x0400, 0xb5ee60ec, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "281b.u6",			0x0400, 0x21a6d4f2, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "282b.u5",			0x0400, 0x07d54f8a, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "283b.u4",			0x0400, 0xbdbe7ec1, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "284b.u3",			0x0400, 0xae9e9f16, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "285b.u2",			0x0400, 0x8dc3ec34, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "286b.u1",			0x0400, 0x4bab9ba2, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "316-0287.u49",		0x0020, 0xd4374b01, 1 | BRF_GRA },           // 16 Color data

	{ "316-0206.u14",		0x0020, 0x9617d796, 0 | BRF_OPT },           // 17 Unused PROM
};

STD_ROM_PICK(invho2)
STD_ROM_FN(invho2)

static struct BurnSampleInfo invdsSampleDesc[] = {
	{ "fire", SAMPLE_NOLOOP },
	{ "invhit", SAMPLE_NOLOOP },
	{ "move1", SAMPLE_NOLOOP },
	{ "move2", SAMPLE_NOLOOP },
	{ "move3", SAMPLE_NOLOOP },
	{ "move4", SAMPLE_NOLOOP },
	{ "saucer", SAMPLE_NOLOOP },
	{ "shiphit", SAMPLE_NOLOOP },
	{ "", 0 }
};

STD_SAMPLE_PICK(invds)
STD_SAMPLE_FN(invds)

static INT32 Invho2Init()
{
	return DrvInit(0x4000, 0x8000, 0, invho2_write_port, invho2_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvInvho2 = {
	"invho2", NULL, NULL, "invinco", "1979",
	"Invinco / Head On 2\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, invho2RomInfo, invho2RomName, invdsSampleInfo, invdsSampleName, Invho2InputInfo, Invho2DIPInfo,
	Invho2Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Safari (set 1)

static struct BurnRomInfo safariRomDesc[] = {
	{ "316-0066.u48",		0x0400, 0x2a26b098, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "316-0065.u47",		0x0400, 0xb776f7db, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "316-0064.u46",		0x0400, 0x19d8c196, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "316-0063.u45",		0x0400, 0x028bad25, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "316-0062.u44",		0x0400, 0x504e0575, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "316-0061.u43",		0x0400, 0xd4c528e0, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "316-0060.u42",		0x0400, 0x48c7b0cc, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "316-0059.u41",		0x0400, 0x3f7baaff, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "316-0058.u40",		0x0400, 0x0d5058f1, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "316-0057.u39",		0x0400, 0x298e8c41, 1 | BRF_PRG | BRF_ESS }, //  9

	{ "316-0043.u87",		0x0020, 0xe60a7960, 0 | BRF_OPT },           // 10 Unused PROMs
	{ "316-0042.u88",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 11
};

STD_ROM_PICK(safari)
STD_ROM_FN(safari)

static INT32 SafariInit()
{
	return DrvInit(0x4000, 0xc000, 1, safari_write_port, safari_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvSafari = {
	"safari", NULL, NULL, NULL, "1977",
	"Safari (set 1)\0", "No sound", "Gremlin", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, safariRomInfo, safariRomName, NULL, NULL, SafariInputInfo, SafariDIPInfo,
	SafariInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Safari (set 2, bootleg?)

static struct BurnRomInfo safariaRomDesc[] = {
	{ "hu1.22c",			0x0400, 0xf27d5961, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "hu2.20c",			0x0400, 0x11a9cb59, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "hu3.19c",			0x0400, 0x4fe746cb, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "hu4.17c",			0x0400, 0xf0bad948, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "hu5.16c",			0x0400, 0xd994f98a, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "hu6.15c",			0x0400, 0x174b5964, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "hu7.13c",			0x0400, 0x3e94caa1, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "hu8.12c",			0x0400, 0xa8a5dca0, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "hu9.11c",			0x0400, 0x0ace0939, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "hu10.9c",			0x0400, 0x9dae33ca, 1 | BRF_PRG | BRF_ESS }, //  9

	{ "32.21e",				0x0020, 0xe60a7960, 0 | BRF_OPT },           // 10 Unused PROMs
	{ "31.22e",				0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 11
};

STD_ROM_PICK(safaria)
STD_ROM_FN(safaria)

struct BurnDriver BurnDrvSafaria = {
	"safaria", "safari", NULL, NULL, "1977",
	"Safari (set 2, bootleg?)\0", "No sound", "Gremlin", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, safariaRomInfo, safariaRomName, NULL, NULL, SafariInputInfo, SafariDIPInfo,
	SafariInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Frogs

static struct BurnRomInfo frogsRomDesc[] = {
	{ "316-119a.u48",		0x0400, 0xb1d1fce4, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "316-118a.u47",		0x0400, 0x12fdcc05, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "316-117a.u46",		0x0400, 0x8a5be424, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "316-116b.u45",		0x0400, 0x09b82619, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "316-115a.u44",		0x0400, 0x3d4e4fa8, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "316-114a.u43",		0x0400, 0x04a21853, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "316-113a.u42",		0x0400, 0x02786692, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "316-112a.u41",		0x0400, 0x0be2a058, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "316-0043.u87",		0x0020, 0xe60a7960, 0 | BRF_OPT },           //  8 Unused PROMs
	{ "316-0042.u88",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           //  9
};

STD_ROM_PICK(frogs)
STD_ROM_FN(frogs)

static struct BurnSampleInfo frogsSampleDesc[] = {
	{ "boing", SAMPLE_NOLOOP },
	{ "buzzz", SAMPLE_NOLOOP },
	{ "croak", SAMPLE_NOLOOP },
	{ "hop", SAMPLE_NOLOOP },
	{ "splash", SAMPLE_NOLOOP },
	{ "zip", SAMPLE_NOLOOP },
	{ "", 0 }
};

STD_SAMPLE_PICK(frogs)
STD_SAMPLE_FN(frogs)

static INT32 FrogsInit()
{
	return DrvInit(0x4000, 0x8000, 0, frogs_write_port, frogs_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvFrogs = {
	"frogs", NULL, NULL, "frogs", "1978",
	"Frogs\0", "No sound", "Gremlin", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, frogsRomInfo, frogsRomName, frogsSampleInfo, frogsSampleName, FrogsInputInfo, FrogsDIPInfo,
	FrogsInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Alpha Fighter / Head On

static struct BurnRomInfo alphahoRomDesc[] = {
	{ "c0.bin",				0x0400, 0xdb774c23, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "c1.bin",				0x0400, 0xb63f4695, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "c2.bin",				0x0400, 0x4ebf0ba4, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "c3.bin",				0x0400, 0x126f17ec, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "c4.bin",				0x0400, 0x52798c61, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "c5.bin",				0x0400, 0x4827cb36, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "c6.bin",				0x0400, 0x8b2ff47e, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "c7.bin",				0x0400, 0x44921df4, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "c8.bin",				0x0400, 0x9fb12fca, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "c9.bin",				0x0400, 0xe5f622f7, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "ca.bin",				0x0400, 0x82b28e77, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "cb.bin",				0x0400, 0x94fba0ad, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "cc.bin",				0x0400, 0xde338b6d, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "cd.bin",				0x0400, 0xbe76baac, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "ce.bin",				0x0400, 0x3c409d57, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "cf.bin",				0x0400, 0xd03c5a09, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "alphaho.col",		0x0020, 0x00000000, 0 | BRF_NODUMP },        // 16 Color data (undumped)
};

STD_ROM_PICK(alphaho)
STD_ROM_FN(alphaho)

static INT32 AlphahoInit()
{
	return DrvInit(0x4000, 0x8000, 0, alphaho_write_port, alphaho_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvAlphaho = {
	"alphaho", NULL, NULL, NULL, "19??",
	"Alpha Fighter / Head On\0", "No sound", "Data East Corporation", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_RACING, 0,
	NULL, alphahoRomInfo, alphahoRomName, NULL, NULL, AlphahoInputInfo, AlphahoDIPInfo,
	AlphahoInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Heiankyo Alien

static struct BurnRomInfo heiankyoRomDesc[] = {
	{ "ha16.u33",			0x0400, 0x1eec8b36, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "ha15.u32",			0x0400, 0xc1b9a1a5, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ha14.u31",			0x0400, 0x5b7b582e, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "ha13.u30",			0x0400, 0x4aa67e01, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "ha12.u29",			0x0400, 0x75889ca6, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "ha11.u28",			0x0400, 0xd469226a, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "ha10.u27",			0x0400, 0x4e203074, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "ha9.u26",			0x0400, 0x9c3a3dd2, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "ha8.u8",				0x0400, 0x6cc64878, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "ha7.u7",				0x0400, 0x6d2f9527, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "ha6.u6",				0x0400, 0xe467c353, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "ha3.u3",				0x0400, 0x6a55eda8, 1 | BRF_PRG | BRF_ESS }, // 11 (0x800-sized gap)
	{ "ha2.u2",				0x0400, 0x056b3b8b, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "ha1.u1",				0x0400, 0xb8da2b5e, 1 | BRF_PRG | BRF_ESS }, // 13

	{ "316-138.u49",		0x0020, 0x67104ea9, 1 | BRF_GRA },           // 14 Color data

	{ "316-0043.u87",		0x0020, 0xe60a7960, 0 | BRF_OPT },           // 15 Unused PROMs
	{ "316-0042.u88",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 16
};

STD_ROM_PICK(heiankyo)
STD_ROM_FN(heiankyo)

static void heiankyo_callback()
{
	// gap after .u3 of 0x800
	memmove (DrvZ80ROM + 0x3800, DrvZ80ROM + 0x3000, 0x0800);
	memset (DrvZ80ROM + 0x3000, 0, 0x800);
	
	// halves of color prom are swapped, only first bank used
	memcpy (DrvColPROM, DrvColPROM + 0x10, 0x0008);
}

static INT32 HeiankyoInit()
{
	return DrvInit(0x4000, 0x8000, 0, heiankyo_write_port, heiankyo_read_port, NULL, heiankyo_callback);
}

struct BurnDriver BurnDrvHeiankyo = {
	"heiankyo", NULL, NULL, NULL, "1979",
	"Heiankyo Alien\0", "No sound", "Denki Onkyo", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, heiankyoRomInfo, heiankyoRomName, NULL, NULL, HeiankyoInputInfo, HeiankyoDIPInfo,
	HeiankyoInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Pulsar

static struct BurnRomInfo pulsarRomDesc[] = {
	{ "790.u33",			0x0400, 0x5e3816da, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "791.u32",			0x0400, 0xce0aee83, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "792.u31",			0x0400, 0x72d78cf1, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "793.u30",			0x0400, 0x42155dd4, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "794.u29",			0x0400, 0x11c7213a, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "795.u28",			0x0400, 0xd2f02e29, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "796.u27",			0x0400, 0x67737a2e, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "797.u26",			0x0400, 0xec250b24, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "798.u8",				0x0400, 0x1d34912d, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "799.u7",				0x0400, 0xf5695e4c, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "800.u6",				0x0400, 0xbf91ad92, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "801.u5",				0x0400, 0x1e9721dc, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "802.u4",				0x0400, 0xd32d2192, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "803.u3",				0x0400, 0x3ede44d5, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "804.u2",				0x0400, 0x62847b01, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "805.u1",				0x0400, 0xab418e86, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "316-0789.u49",		0x0020, 0x7fc1861f, 1 | BRF_GRA },           // 16 Color data

	{ "316-0206.u14",		0x0020, 0x9617d796, 0 | BRF_OPT },           // 17 Unused prom
};

STD_ROM_PICK(pulsar)
STD_ROM_FN(pulsar)

static struct BurnSampleInfo pulsarSampleDesc[] = {
	{ "alienhit", SAMPLE_NOLOOP },
	{ "ashoot", SAMPLE_NOLOOP },
	{ "birth", SAMPLE_NOLOOP },
	{ "bonus", SAMPLE_NOLOOP },
	{ "clang", SAMPLE_NOLOOP },
	{ "gate", SAMPLE_NOLOOP },
	{ "hbeat", SAMPLE_NOLOOP },
	{ "key", SAMPLE_NOLOOP },
	{ "movmaze", SAMPLE_NOLOOP },
	{ "phit", SAMPLE_NOLOOP },
	{ "pshoot", SAMPLE_NOLOOP },
	{ "sizzle", SAMPLE_NOLOOP },
	{ "", 0 }
};

STD_SAMPLE_PICK(pulsar)
STD_SAMPLE_FN(pulsar)

static INT32 PulsarInit()
{
	return DrvInit(0x4000, 0x8000, 0, pulsar_write_port, pulsar_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvPulsar = {
	"pulsar", NULL, NULL, "pulsar", "1981",
	"Pulsar\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, pulsarRomInfo, pulsarRomName, pulsarSampleInfo, pulsarSampleName, PulsarInputInfo, PulsarDIPInfo,
	PulsarInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Digger

static struct BurnRomInfo diggerRomDesc[] = {
	{ "684.u27",			0x0400, 0xbba0d7c2, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "685.u26",			0x0400, 0x85210d8b, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "686.u25",			0x0400, 0x2d87238c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "687.u24",			0x0400, 0x0dd0604e, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "688.u23",			0x0400, 0x2f649667, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "689.u22",			0x0400, 0x89fd63d9, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "690.u21",			0x0400, 0xa86622a6, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "691.u20",			0x0400, 0x8aca72d8, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "316-507",			0x0020, 0xfdb22e8f, 1 | BRF_GRA },           //  8 Color data
	
	{ "316-0206.u14",		0x0020, 0x9617d796, 0 | BRF_OPT },           //  9 Unused prom
};

STD_ROM_PICK(digger)
STD_ROM_FN(digger)

static INT32 DiggerInit()
{
	return DrvInit(0x2000, 0xc000, 0, digger_write_port, digger_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvDigger = {
	"digger", NULL, NULL, NULL, "1980",
	"Digger\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, diggerRomInfo, diggerRomName, NULL, NULL, DiggerInputInfo, DiggerDIPInfo,
	DiggerInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Invinco / Deep Scan

static struct BurnRomInfo invdsRomDesc[] = {
	{ "367.u33",			0x0400, 0xe6a33eae, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "368.u32",			0x0400, 0x421554a8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "369.u31",			0x0400, 0x531e917a, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "370.u30",			0x0400, 0x2ad68f8c, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "371.u29",			0x0400, 0x1b98dc5c, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "372.u28",			0x0400, 0x3a72190a, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "373.u27",			0x0400, 0x3d361520, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "374.u26",			0x0400, 0xe606e7d9, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "375.u8",				0x0400, 0xadbe8d32, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "376.u7",				0x0400, 0x79409a46, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "377.u6",				0x0400, 0x3f021a71, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "378.u5",				0x0400, 0x49a542b0, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "379.u4",				0x0400, 0xee140e49, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "380.u3",				0x0400, 0x688ba831, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "381.u2",				0x0400, 0x798ba0c7, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "382.u1",				0x0400, 0x8d195c24, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "316-0246.u44",		0x0020, 0xfe4406cb, 1 | BRF_GRA },           // 16 Color data

	{ "316-0206.u14",		0x0020, 0x9617d796, 0 | BRF_OPT },           // 17 Unused prom
};

STD_ROM_PICK(invds)
STD_ROM_FN(invds)

static INT32 InvdsInit()
{
	return DrvInit(0x4000, 0x8000, 0, invds_write_port, invds_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvInvds = {
	"invds", NULL, NULL, "invinco", "1979",
	"Invinco / Deep Scan\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, invdsRomInfo, invdsRomName, invdsSampleInfo, invdsSampleName, InvdsInputInfo, InvdsDIPInfo,
	InvdsInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Invinco

static struct BurnRomInfo invincoRomDesc[] = {
	{ "310a.u27",			0x0400, 0xe3931365, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "311a.u26",			0x0400, 0xde1a6c4a, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "312a.u25",			0x0400, 0xe3c08f39, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "313a.u24",			0x0400, 0xb680b306, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "314a.u23",			0x0400, 0x790f07d9, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "315a.u22",			0x0400, 0x0d13bed2, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "316a.u21",			0x0400, 0x88d7eab8, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "317a.u20",			0x0400, 0x75389463, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "318a.uxx",			0x0400, 0x0780721d, 1 | BRF_PRG | BRF_ESS }, //  8

	{ "316-0246.u44",		0x0020, 0xfe4406cb, 1 | BRF_GRA },           //  9 Color data
};

STD_ROM_PICK(invinco)
STD_ROM_FN(invinco)

static INT32 InvincoInit()
{
	return DrvInit(0x4000, 0xc000, 0, invinco_write_port, invinco_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvInvinco = {
	"invinco", NULL, NULL, "invinco", "1979",
	"Invinco\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, invincoRomInfo, invincoRomName, invdsSampleInfo, invdsSampleName, InvincoInputInfo, InvincoDIPInfo,
	InvincoInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Samurai

static struct BurnRomInfo samuraiRomDesc[] = {
	{ "epr-289.u33",		0x0400, 0xa1a9cb03, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "epr-290.u32",		0x0400, 0x49fede51, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "epr-291.u31",		0x0400, 0x6503dd72, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "epr-292.u30",		0x0400, 0x179c224f, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "epr-366.u29",		0x0400, 0x3df2abec, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "epr-355.u28",		0x0400, 0xb24517a4, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "epr-367.u27",		0x0400, 0x992a6e5a, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "epr-368.u26",		0x0400, 0x403c72ce, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "epr-369.u8",			0x0400, 0x3cfd115b, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "epr-370.u7",			0x0400, 0x2c30db12, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "epr-299.u6",			0x0400, 0x87c71139, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "epr-371.u5",			0x0400, 0x761f56cf, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "epr-301.u4",			0x0400, 0x23de1ff7, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "epr-372.u3",			0x0400, 0x292cfd89, 1 | BRF_PRG | BRF_ESS }, // 13

	{ "pr55.clr",			0x0020, 0x975f5fb0, 1 | BRF_GRA },           // 14 Color data

	{ "316-0043.u87",		0x0020, 0xe60a7960, 0 | BRF_OPT },           // 15 Unused PROMs
	{ "316-0042.u88",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 16
};

STD_ROM_PICK(samurai)
STD_ROM_FN(samurai)

static void samurai_map()
{
	ZetSetWriteHandler(samurai_write);
}

static INT32 SamuraiInit()
{
	return DrvInit(0x4000, 0x8000, 0, samurai_write_port, samurai_read_port, samurai_map, NULL);
}

struct BurnDriver BurnDrvSamurai = {
	"samurai", NULL, NULL, NULL, "1980",
	"Samurai\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, samuraiRomInfo, samuraiRomName, NULL, NULL, SamuraiInputInfo, SamuraiDIPInfo,
	SamuraiInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Tranquillizer Gun

static struct BurnRomInfo tranqgunRomDesc[] = {
	{ "u33.bin",			0x0400, 0x6d50e902, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "u32.bin",			0x0400, 0xf0ba0e60, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "u31.bin",			0x0400, 0x9fe440d3, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "u30.bin",			0x0400, 0x1041608e, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "u29.bin",			0x0400, 0xfb5de95f, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "u28.bin",			0x0400, 0x03fd8727, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "u27.bin",			0x0400, 0x3d93239b, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "u26.bin",			0x0400, 0x20f64a7f, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "u8.bin",				0x0400, 0x5121c695, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "u7.bin",				0x0400, 0xb13d21f7, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "u6.bin",				0x0400, 0x603cee59, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "u5.bin",				0x0400, 0x7f25475f, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "u4.bin",				0x0400, 0x57dc3123, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "u3.bin",				0x0400, 0x7aa7829b, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "u2.bin",				0x0400, 0xa9b10df5, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "u1.bin",				0x0400, 0x431a7449, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "u49.bin",			0x0020, 0x6481445b, 1 | BRF_GRA },           // 16 Color data

	{ "316-0043.u87",		0x0020, 0xe60a7960, 0 | BRF_OPT },           // 17 Unused PROMs
	{ "316-0042.u88",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 18
};

STD_ROM_PICK(tranqgun)
STD_ROM_FN(tranqgun)

static struct BurnSampleInfo tranqgunSampleDesc[] = {
	{ "animal", SAMPLE_NOLOOP },
	{ "animalhit", SAMPLE_NOLOOP },
	{ "cry", SAMPLE_NOLOOP },
	{ "emar", SAMPLE_NOLOOP },
	{ "gun", SAMPLE_NOLOOP },
	{ "jeep", SAMPLE_NOLOOP },
	{ "point", SAMPLE_NOLOOP },
	{ "walk", SAMPLE_NOLOOP },
	{ "", 0 }
};

STD_SAMPLE_PICK(tranqgun)
STD_SAMPLE_FN(tranqgun)

static INT32 TranqgunInit()
{
	return DrvInit(0x4000, 0x8000, 0, tranqgun_write_port, tranqgun_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvTranqgun = {
	"tranqgun", NULL, NULL, "tranqgun", "1980",
	"Tranquillizer Gun\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, tranqgunRomInfo, tranqgunRomName, tranqgunSampleInfo, tranqgunSampleName, TranqgunInputInfo, NULL,
	TranqgunInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Head On (2 players)

static struct BurnRomInfo headonRomDesc[] = {
	{ "316-163a.u27",		0x0400, 0x4bb51259, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "316-164a.u26",		0x0400, 0xaeac8c5f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "316-165a.u25",		0x0400, 0xf1a0cb72, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "316-166c.u24",		0x0400, 0x65d12951, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "316-167c.u23",		0x0400, 0x2280831e, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "316-192a.u22",		0x0400, 0xed4666f2, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "316-193a.u21",		0x0400, 0x37a1df4c, 1 | BRF_PRG | BRF_ESS }, //  6

	{ "316-0138.u44",		0x0020, 0x67104ea9, 1 | BRF_GRA },           //  7 Color data

	{ "316-0043.u87",		0x0020, 0xe60a7960, 0 | BRF_OPT },           //  8 Unused PROMs
	{ "316-0042.u88",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           //  9
};

STD_ROM_PICK(headon)
STD_ROM_FN(headon)

static INT32 HeadonInit()
{
	return DrvInit(0x2000, 0xc000, 0, headon_write_port, headon_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvHeadon = {
	"headon", NULL, NULL, NULL, "1979",
	"Head On (2 players)\0", "No sound", "Gremlin", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_RACING, 0,
	NULL, headonRomInfo, headonRomName, NULL, NULL, HeadonInputInfo, HeadonDIPInfo,
	HeadonInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Head On (1 player)

static struct BurnRomInfo headon1RomDesc[] = {
	{ "316-163a.u27",		0x0400, 0x4bb51259, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "316-164a.u26",		0x0400, 0xaeac8c5f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "316-165a.u25",		0x0400, 0xf1a0cb72, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "316-166b.u24",		0x0400, 0x1c59008a, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "316-167a.u23",		0x0400, 0x069e839e, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "316-192a.u22",		0x0400, 0xed4666f2, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "316193a1.u21",		0x0400, 0xd3782c1d, 1 | BRF_PRG | BRF_ESS }, //  6

	{ "316-0138.u44",		0x0020, 0x67104ea9, 1 | BRF_GRA },           //  7 Color data

	{ "316-0043.u87",		0x0020, 0xe60a7960, 0 | BRF_OPT },           //  8 Unused PROMs
	{ "316-0042.u88",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           //  9
};

STD_ROM_PICK(headon1)
STD_ROM_FN(headon1)

struct BurnDriver BurnDrvHeadon1 = {
	"headon1", "headon", NULL, NULL, "1979",
	"Head On (1 player)\0", "No sound", "Gremlin", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_RACING, 0,
	NULL, headon1RomInfo, headon1RomName, NULL, NULL, HeadonInputInfo, HeadonDIPInfo,
	HeadonInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Head On (Sidam bootleg, set 1)

static struct BurnRomInfo headonsRomDesc[] = {
	{ "0.1a",				0x0400, 0x4bb51259, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "1.3a",				0x0400, 0xaeac8c5f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "2.4a",				0x0400, 0xf1a0cb72, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "3.6a",				0x0400, 0x461c2658, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "4.8a",				0x0400, 0x79fc7f31, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "5.9a",				0x0400, 0xed4666f2, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "6.11a",				0x0400, 0x7a709d68, 1 | BRF_PRG | BRF_ESS }, //  6

	{ "316-0043.u87",		0x0020, 0xe60a7960, 0 | BRF_OPT },           //  7 Unused PROMs
	{ "316-0042.u88",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           //  8
};

STD_ROM_PICK(headons)
STD_ROM_FN(headons)

struct BurnDriverD BurnDrvHeadons = {
	"headons", "headon", NULL, NULL, "1979",
	"Head On (Sidam bootleg, set 1)\0", "No sound", "bootleg (Sidam)", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_RACING, 0,
	NULL, headonsRomInfo, headonsRomName, NULL, NULL, HeadonInputInfo, HeadonDIPInfo,
	HeadonInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Head On (Sidam bootleg, set 2)

static struct BurnRomInfo headonsaRomDesc[] = {
	{ "10305.0.9a",			0x0400, 0x9a37407b, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "10305.1.8a",			0x0400, 0xaeac8c5f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "10305.2.7a",			0x0400, 0xf1a0cb72, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "10305.3.6a",			0x0400, 0xae33fcc4, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "10305.4.5a",			0x0400, 0xe87f6fd8, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "10305.5.4a",			0x0400, 0x387e2eba, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "10305.6b.3a",		0x0400, 0x18749071, 1 | BRF_PRG | BRF_ESS }, //  6

	{ "10303.3e",			0x0020, 0xe60a7960, 0 | BRF_OPT },           //  7 Unused PROMs
	{ "10302.2e",			0x0020, 0xa1506b9d, 0 | BRF_OPT },           //  8
};

STD_ROM_PICK(headonsa)
STD_ROM_FN(headonsa)

struct BurnDriverD BurnDrvHeadonsa = {
	"headonsa", "headon", NULL, NULL, "1979",
	"Head On (Sidam bootleg, set 2)\0", "No sound", "bootleg (Sidam)", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_RACING, 0,
	NULL, headonsaRomInfo, headonsaRomName, NULL, NULL, HeadonInputInfo, HeadonDIPInfo,
	HeadonInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Crash (bootleg of Head On)

static struct BurnRomInfo hocrashRomDesc[] = {
	{ "1-0s.0s",			0x0400, 0x4bb51259, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "2-0r.0r",			0x0400, 0xaeac8c5f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3-0p.0p",			0x0400, 0xf1a0cb72, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "4-0m.0m",			0x0400, 0xfd67208d, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "5-0l.0l",			0x0400, 0x069e839e, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "6-0k.0k",			0x0400, 0x11960190, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "7-0j.0j",			0x0400, 0xd3782c1d, 1 | BRF_PRG | BRF_ESS }, //  6

	{ "316-0043.u87",		0x0020, 0xe60a7960, 0 | BRF_OPT },           //  7 Unused PROMs
	{ "316-0042.u88",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           //  8
};

STD_ROM_PICK(hocrash)
STD_ROM_FN(hocrash)

struct BurnDriver BurnDrvHocrash = {
	"hocrash", "headon", NULL, NULL, "1979",
	"Crash (bootleg of Head On)\0", "No sound", "bootleg (Fraber)", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_RACING, 0,
	NULL, hocrashRomInfo, hocrashRomName, NULL, NULL, HeadonInputInfo, HeadonDIPInfo,
	HeadonInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Head On (bootleg, alt maze)

static struct BurnRomInfo headonmzRomDesc[] = {
	{ "0.bin",				0x0400, 0x1febc85a, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "1.bin",				0x0400, 0xaeac8c5f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "2.bin",				0x0400, 0xa5d0e0f5, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "3.bin",				0x0400, 0x721f3b03, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "4.bin",				0x0400, 0x82c73635, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "5.bin",				0x0400, 0x17c04c3a, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "6.bin",				0x0400, 0x88e43434, 1 | BRF_PRG | BRF_ESS }, //  6

	{ "316-0138.u44",		0x0020, 0x67104ea9, 1 | BRF_GRA },           //  7 Color data

	{ "10303.3e",			0x0020, 0xe60a7960, 0 | BRF_OPT },           //  8 Unused PROMs
	{ "10302.2e",			0x0020, 0xa1506b9d, 0 | BRF_OPT },           //  9
};

STD_ROM_PICK(headonmz)
STD_ROM_FN(headonmz)

struct BurnDriver BurnDrvHeadonmz = {
	"headonmz", "headon", NULL, NULL, "1979",
	"Head On (bootleg, alt maze)\0", "No sound", "bootleg", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_RACING, 0,
	NULL, headonmzRomInfo, headonmzRomName, NULL, NULL, HeadonInputInfo, HeadonDIPInfo,
	HeadonInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Head On N

static struct BurnRomInfo headonnRomDesc[] = {
	{ "rom.e4",				0x0400, 0xa6cd13fc, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "rom.f4",				0x0400, 0xd1cd498f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "rom.g4",				0x0400, 0x0fb02db2, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "rom.h4",				0x0400, 0x38db2d02, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "rom.i4",				0x0400, 0xa04d8522, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "rom.j4",				0x0400, 0x52bd2151, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "rom.k4",				0x0400, 0x9488a8b3, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "rom.l4",				0x0400, 0xa37f0be0, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "prom.g2",			0x0020, 0x67104ea9, 1 | BRF_GRA },           //  8 Color data

	{ "prom.b6",			0x0020, 0x67104ea9, 0 | BRF_OPT },           //  9 Unused PROMs
	{ "prom.f2",			0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 10
};

STD_ROM_PICK(headonn)
STD_ROM_FN(headonn)

static INT32 HeadonnInit()
{
	return DrvInit(0x4000, 0x8000, 0, headonn_write_port, headonn_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvHeadonn = {
	"headonn", "headon", NULL, NULL, "1979",
	"Head On N\0", "No sound", "Nintendo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_RACING, 0,
	NULL, headonnRomInfo, headonnRomName, NULL, NULL, HeadonnInputInfo, HeadonnDIPInfo,
	HeadonnInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Super Crash (bootleg of Head On)

static struct BurnRomInfo supcrashRomDesc[] = {
	{ "1-2-scrash.bin",		0x0800, 0x789a8b73, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "3-4-scrash.bin",		0x0800, 0x7a310527, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "5-6-scrash.bin",		0x0800, 0x62d33c09, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "7-8-scrash.bin",		0x0400, 0x0f8ea335, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "316-0043.u87",		0x0020, 0xe60a7960, 0 | BRF_OPT },           //  4 Unused PROMs
	{ "316-0042.u88",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           //  5
};

STD_ROM_PICK(supcrash)
STD_ROM_FN(supcrash)

static INT32 SupcrashInit()
{
	return DrvInit(0x4000, 0x8000, 0, headon_write_port, supcrash_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvSupcrash = {
	"supcrash", "headon", NULL, NULL, "1979",
	"Super Crash (bootleg of Head On)\0", "No sound", "bootleg (VGG)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_RACING, 0,
	NULL, supcrashRomInfo, supcrashRomName, NULL, NULL, SupcrashInputInfo, SupcrashDIPInfo,
	SupcrashInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Star Trek (Head On hardware)

static struct BurnRomInfo startrksRomDesc[] = {
	{ "0.1a",				0x0400, 0x2ba4202a, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "1.3a",				0x0400, 0xcf6081b8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "2.4a",				0x0400, 0xfd983c0c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "3.6a",				0x0400, 0x607991c7, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "4.8a",				0x0400, 0x043bf767, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "5.9a",				0x0400, 0x2aa21da3, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "6.11a",				0x0400, 0xa5315dc8, 1 | BRF_PRG | BRF_ESS }, //  6

	{ "82s123.15c",			0x0020, 0xe60a7960, 0 | BRF_OPT },           //  7 Unused PROMs
	{ "82s123.14c",			0x0020, 0xa1506b9d, 0 | BRF_OPT },           //  8
};

STD_ROM_PICK(startrks)
STD_ROM_FN(startrks)

struct BurnDriver BurnDrvStartrks = {
	"startrks", NULL, NULL, NULL, "198?",
	"Star Trek (Head On hardware)\0", "No sound", "bootleg (Sidam)", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, startrksRomInfo, startrksRomName, NULL, NULL, HeadonInputInfo, HeadonDIPInfo,
	HeadonInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Borderline

static struct BurnRomInfo brdrlineRomDesc[] = {
	{ "b1.bin",				0x0400, 0xdf182769, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "b2.bin",				0x0400, 0xe1d1c4ce, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "b3.bin",				0x0400, 0x4ec4afa2, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "b4.bin",				0x0400, 0x88de95f6, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "b5.bin",				0x0400, 0x2e4e13b9, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "b6.bin",				0x0400, 0xc181e87a, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "b7.bin",				0x0400, 0x21180015, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "b8.bin",				0x0400, 0x56a7fee0, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "b9.bin",				0x0400, 0xbb532e63, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "b10.bin",			0x0400, 0x64793709, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "b11.bin",			0x0400, 0x2ae2f928, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "b12.bin",			0x0400, 0xe14cfaf5, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "b13.bin",			0x0400, 0x605e0d27, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "b14.bin",			0x0400, 0x93f5714f, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "b15.bin",			0x0400, 0x2f8a9b1c, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "b16.bin",			0x0400, 0xcc138bed, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "borderc.49",			0x0020, 0xbc6be94e, 1 | BRF_GRA },           // 16 Color data

	{ "border.32",			0x0020, 0xc128d0ba, 0 | BRF_OPT },           // 17 Unused PROMs
	{ "bordera.15",			0x0020, 0x6449e678, 0 | BRF_OPT },           // 18
	{ "borderb.14",			0x0020, 0x55dcdef1, 0 | BRF_OPT },           // 19
	{ "prom93427.1",		0x0100, 0x64b98dc7, 0 | BRF_OPT },           // 20
	{ "prom93427.2",		0x0100, 0xbda82367, 0 | BRF_OPT },           // 21
	{ "au.bin",				0x0400, 0xa23e1d9f, 0 | BRF_OPT },           // 22
};

STD_ROM_PICK(brdrline)
STD_ROM_FN(brdrline)

static struct BurnSampleInfo brdrlineSampleDesc[] = {
	{ "boot_and_start", SAMPLE_NOLOOP },
	{ "coin", SAMPLE_NOLOOP },
	{ "crashes", SAMPLE_NOLOOP },
	{ "end_level", SAMPLE_NOLOOP },
	{ "engine_noise", SAMPLE_NOLOOP },
	{ "field", SAMPLE_NOLOOP },
	{ "fire", SAMPLE_NOLOOP },
	{ "", 0 }
};

STD_SAMPLE_PICK(brdrline)
STD_SAMPLE_FN(brdrline)

static INT32 BrdrlineInit()
{
	return DrvInit(0x4000, 0x8000, 0, brdrline_write_port, brdrline_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvBrdrline = {
	"brdrline", NULL, NULL, "brdrline", "1981",
	"Borderline\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, brdrlineRomInfo, brdrlineRomName, brdrlineSampleInfo, brdrlineSampleName, BrdrlineInputInfo, BrdrlineDIPInfo,
	BrdrlineInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Borderline (Sidam bootleg)

static struct BurnRomInfo brdrlinsRomDesc[] = {
	{ "1.33",				0x0400, 0xdf182769, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "2.32",				0x0400, 0x98b26e2a, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3.31",				0x0400, 0x4ec4afa2, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "4.30",				0x0400, 0x88de95f6, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "5.29",				0x0400, 0x2e4e13b9, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "6.28",				0x0400, 0xc181e87a, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "7.27",				0x0400, 0x21180015, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "8.26",				0x0400, 0x56a7fee0, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "9.8",				0x0400, 0xbb532e63, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "10.7",				0x0400, 0x64793709, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "11.6",				0x0400, 0x2ae2f928, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "12.5",				0x0400, 0xe14cfaf5, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "13.4",				0x0400, 0x605e0d27, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "14.3",				0x0400, 0x93f5714f, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "15.2",				0x0400, 0x2f8a9b1c, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "16.1",				0x0400, 0xcc138bed, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "5610.49",			0x0020, 0xbc6be94e, 1 | BRF_GRA },           // 16 Color data

	{ "82s123.bin",			0x0020, 0xc128d0ba, 0 | BRF_OPT },           // 17 Unused PROMs
	{ "5610.15",			0x0020, 0x6449e678, 0 | BRF_OPT },           // 18
	{ "5610.14",			0x0020, 0x55dcdef1, 0 | BRF_OPT },           // 19
	{ "93427.1",			0x0100, 0x64b98dc7, 0 | BRF_OPT },           // 20
	{ "93427.2",			0x0100, 0xbda82367, 0 | BRF_OPT },           // 21

	{ "au.bin",				0x0400, 0xa23e1d9f, 0 | BRF_OPT },           // 22 Sound ROM
};

STD_ROM_PICK(brdrlins)
STD_ROM_FN(brdrlins)

struct BurnDriver BurnDrvBrdrlins = {
	"brdrlins", "brdrline", NULL, "brdrline", "1981",
	"Borderline (Sidam bootleg)\0", "No sound", "bootleg (Sidam)", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, brdrlinsRomInfo, brdrlinsRomName, brdrlineSampleInfo, brdrlineSampleName, BrdrlineInputInfo, BrdrlineDIPInfo,
	BrdrlineInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Borderline (Karateco bootleg)

static struct BurnRomInfo brdrlinbRomDesc[] = {
	{ "border1.33",			0x0800, 0x48387706, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "border2.30",			0x0800, 0x1d669b60, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "border3.29",			0x0800, 0x6e4d6fb3, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "border4.27",			0x0800, 0x718446d8, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "border5.08",			0x0800, 0xa0584337, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "border6.06",			0x0800, 0xcb30fb98, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "border7.04",			0x0800, 0x200c5321, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "border8.02",			0x0800, 0x735e140d, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "borderc.49",			0x0020, 0xbc6be94e, 1 | BRF_GRA },           //  8 Color data

	{ "border.32",			0x0020, 0xc128d0ba, 0 | BRF_OPT },           //  9 Unused PROMs
	{ "bordera.15",			0x0020, 0x6449e678, 0 | BRF_OPT },           // 10
	{ "borderb.14",			0x0020, 0x55dcdef1, 0 | BRF_OPT },           // 11

	{ "bords.bin",			0x0400, 0xa23e1d9f, 0 | BRF_OPT },           // 12 Sound ROM
};

STD_ROM_PICK(brdrlinb)
STD_ROM_FN(brdrlinb)

struct BurnDriver BurnDrvBrdrlinb = {
	"brdrlinb", "brdrline", NULL, "brdrline", "1981",
	"Borderline (Karateco bootleg)\0", "No sound", "bootleg (Karateco)", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, brdrlinbRomInfo, brdrlinbRomName, brdrlineSampleInfo, brdrlineSampleName, BrdrlineInputInfo, BrdrlineDIPInfo,
	BrdrlineInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Star Raker

static struct BurnRomInfo starrkrRomDesc[] = {
	{ "epr-767.u33",		0x0400, 0x2cfe979c, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "epr-768.u32",		0x0400, 0xcf85f158, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "epr-769.u31",		0x0400, 0x22ac6362, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "epr-770.u30",		0x0400, 0xd8d2fc6a, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "epr-771.u29",		0x0400, 0x9a88d577, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "epr-772.u28",		0x0400, 0xbab1574f, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "epr-773.u27",		0x0400, 0xc2406abd, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "epr-774.u26",		0x0400, 0x77686d3b, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "epr-775.u8",			0x0400, 0x1d00b276, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "epr-776.u7",			0x0400, 0x7215a72b, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "epr-777.u6",			0x0400, 0x59176c4c, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "epr-778.u5",			0x0400, 0xb4586631, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "epr-779.u4",			0x0400, 0x1f9a736d, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "epr-780.u3",			0x0400, 0x01d89786, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "epr-781.u2",			0x0400, 0x7d1238a2, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "epr-782.u1",			0x0400, 0x121ce164, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "pr-23.u49",			0x0020, 0x0a2156b3, 1 | BRF_GRA },           // 16 Color data

	{ "pr-33.u15",			0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 17 Unused PROMs
	{ "pr-34.u14",			0x0020, 0xe60a7960, 0 | BRF_OPT },           // 18
	{ "pr-58.5",			0x0800, 0x526ed9d8, 0 | BRF_OPT },           // 19
	{ "pr-60.6",			0x0800, 0x59e6067f, 0 | BRF_OPT },           // 20
	{ "pr-59.12",			0x0800, 0xa2e8090a, 0 | BRF_OPT },           // 21
	{ "pr-61.13",			0x0800, 0xfc663474, 0 | BRF_OPT },           // 22
	{ "pr-65.17",			0x0800, 0xa12430b2, 0 | BRF_OPT },           // 23
	{ "pr-63.18",			0x0800, 0xb3297499, 0 | BRF_OPT },           // 24
	{ "pr-64.25",			0x0800, 0x7342cf53, 0 | BRF_OPT },           // 25
	{ "pr-62.26",			0x0800, 0xd352c545, 0 | BRF_OPT },           // 26
	{ "pr-66.28",			0x0800, 0x895c5733, 0 | BRF_OPT },           // 27
	{ "epr-613.1",			0x0400, 0xff4be0c7, 0 | BRF_OPT },           // 28 Sound PROM
};

STD_ROM_PICK(starrkr)
STD_ROM_FN(starrkr)

struct BurnDriver BurnDrvStarrkr = {
	"starrkr", "brdrline", NULL, "brdrline", "1981",
	"Star Raker\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, starrkrRomInfo, starrkrRomName, brdrlineSampleInfo, brdrlineSampleName, StarrkrInputInfo, StarrkrDIPInfo,
	BrdrlineInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Borderline (Tranquillizer Gun conversion)

static struct BurnRomInfo brdrlinetRomDesc[] = {
	{ "1171a.u33",			0x0400, 0x38dd9880, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "1172a.u32",			0x0400, 0x1a3adff0, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "1173a.u31",			0x0400, 0xe668734d, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "1174a.u30.bad",		0x0400, 0x22c83ae4, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "1175a.u29",			0x0400, 0x116517b8, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "1176a.u28",			0x0400, 0x2b2c4ba8, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "1177a.u27",			0x0400, 0xd8cbcc1e, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "1178a.u26",			0x0400, 0x05b1e3ea, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "1179a.u8",			0x0400, 0xc2dc3181, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "1180a.u7",			0x0400, 0xc00543a7, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "1181a.u6",			0x0400, 0xaba9ca30, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "1182a.u5",			0x0400, 0xfe7cfc31, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "1183a.u4",			0x0400, 0x4e0684cd, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "1184a.u3",			0x0400, 0x0f38ca4c, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "1185a.u2",			0x0400, 0x1dff2ab0, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "1186a.u1",			0x0400, 0x5828ca5a, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "u49.bin",			0x0020, 0x0a2156b3, 1 | BRF_GRA },           // 16 Color data

	{ "pr-52.u14",			0x0020, 0x9617d796, 0 | BRF_OPT },           // 17 Unused PROM
};

STD_ROM_PICK(brdrlinet)
STD_ROM_FN(brdrlinet)

struct BurnDriverD BurnDrvBrdrlinet = {
	"brdrlinet", "brdrline", NULL, "tranqgun", "1981",
	"Borderline (Tranquillizer Gun conversion)\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, brdrlinetRomInfo, brdrlinetRomName, tranqgunSampleInfo, tranqgunSampleName, TranqgunInputInfo, NULL,
	TranqgunInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Space Trek (upright)

static struct BurnRomInfo spacetrkRomDesc[] = {
	{ "u33.bin",			0x0400, 0x9033fe50, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "u32.bin",			0x0400, 0x08f61f0d, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "u31.bin",			0x0400, 0x1088a8c4, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "u30.bin",			0x0400, 0x55560cc8, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "u29.bin",			0x0400, 0x71713958, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "u28.bin",			0x0400, 0x7bcf5ca3, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "u27.bin",			0x0400, 0xad7a2065, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "u26.bin",			0x0400, 0x6060fe77, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "u8.bin",				0x0400, 0x75a90624, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "u7.bin",				0x0400, 0x7b31a2ab, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "u6.bin",				0x0400, 0x94135b33, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "u5.bin",				0x0400, 0xcfbf2538, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "u4.bin",				0x0400, 0xb4b95129, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "u3.bin",				0x0400, 0x03ca1d70, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "u2.bin",				0x0400, 0xa968584b, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "u1.bin",				0x0400, 0xe6e300e8, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "u49.bin",			0x0020, 0xaabae4cd, 1 | BRF_GRA },           // 16 Color data

	{ "316-0043.u87",		0x0020, 0xe60a7960, 0 | BRF_OPT },           // 17 Unused PROMs
	{ "316-0042.u88",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 18
};

STD_ROM_PICK(spacetrk)
STD_ROM_FN(spacetrk)

static INT32 SpacetrkInit()
{
	return DrvInit(0x4000, 0x8000, 0, spacetrk_write_port, spacetrk_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvSpacetrk = {
	"spacetrk", NULL, NULL, NULL, "1980",
	"Space Trek (upright)\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, spacetrkRomInfo, spacetrkRomName, NULL, NULL, SpacetrkInputInfo, SpacetrkDIPInfo,
	SpacetrkInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Space Trek (cocktail)

static struct BurnRomInfo spacetrkcRomDesc[] = {
	{ "u33c.bin",			0x0400, 0xb056b928, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "u32c.bin",			0x0400, 0xdffb11d9, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "u31c.bin",			0x0400, 0x9b25d46f, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "u30c.bin",			0x0400, 0x3a612bfe, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "u29c.bin",			0x0400, 0xd8bb6e0c, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "u28c.bin",			0x0400, 0x0e367740, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "u27c.bin",			0x0400, 0xd59fec86, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "u26c.bin",			0x0400, 0x9deefa0f, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "u8c.bin",			0x0400, 0x613116c5, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "u7c.bin",			0x0400, 0x3bdf2464, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "u6c.bin",			0x0400, 0x039d73fa, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "u5c.bin",			0x0400, 0x1638344f, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "u4c.bin",			0x0400, 0xe34443cd, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "u3c.bin",			0x0400, 0x6f16cbd7, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "u2c.bin",			0x0400, 0x94da3cdc, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "u1c.bin",			0x0400, 0x2a228bf4, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "u49.bin",			0x0020, 0xaabae4cd, 1 | BRF_GRA },           // 16 Color data

	{ "316-0043.u87",		0x0020, 0xe60a7960, 0 | BRF_OPT },           // 17 Unused PROMs
	{ "316-0042.u88",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 18
};

STD_ROM_PICK(spacetrkc)
STD_ROM_FN(spacetrkc)

struct BurnDriver BurnDrvSpacetrkc = {
	"spacetrkc", "spacetrk", NULL, NULL, "1980",
	"Space Trek (cocktail)\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, spacetrkcRomInfo, spacetrkcRomName, NULL, NULL, SpacetrkcInputInfo, SpacetrkcDIPInfo,
	SpacetrkInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Carnival (upright)

static struct BurnRomInfo carnivalRomDesc[] = {
	{ "epr-651.u33",		0x0400, 0x9f2736e6, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "epr-652.u32",		0x0400, 0xa1f58beb, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "epr-653.u31",		0x0400, 0x67b17922, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "epr-654.u30",		0x0400, 0xbefb09a5, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "epr-655.u29",		0x0400, 0x623fcdad, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "epr-656.u28",		0x0400, 0x53040332, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "epr-657.u27",		0x0400, 0xf2537467, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "epr-658.u26",		0x0400, 0xfcc3854e, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "epr-659.u8",			0x0400, 0x28be8d69, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "epr-660.u7",			0x0400, 0x3873ccdb, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "epr-661.u6",			0x0400, 0xd9a96dff, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "epr-662.u5",			0x0400, 0xd893ca72, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "epr-663.u4",			0x0400, 0xdf8c63c5, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "epr-664.u3",			0x0400, 0x689a73e8, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "epr-665.u2",			0x0400, 0x28e7b2b6, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "epr-666.u1",			0x0400, 0x4eec7fae, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "316-633",			0x0020, 0xf0084d80, 1 | BRF_GRA },           // 16 Color data

	{ "epr-412",			0x0400, 0x0dbaa2b0, 3 | BRF_PRG | BRF_ESS }, // 17 I8039 Code

	{ "316-0206.u14",		0x0020, 0x9617d796, 0 | BRF_OPT },           // 18 Unused PROM
};

STD_ROM_PICK(carnival)
STD_ROM_FN(carnival)

static struct BurnSampleInfo carnivalSampleDesc[] = {
	{ "bear", SAMPLE_NOLOOP },
	{ "bonus1", SAMPLE_NOLOOP },
	{ "bonus2", SAMPLE_NOLOOP },
	{ "clang", SAMPLE_NOLOOP },
	{ "duck1", SAMPLE_NOLOOP },
	{ "duck2", SAMPLE_NOLOOP },
	{ "duck3", SAMPLE_NOLOOP },
	{ "pipehit", SAMPLE_NOLOOP },
	{ "ranking", SAMPLE_NOLOOP },
	{ "rifle", SAMPLE_NOLOOP },
	{ "", 0 }
};

STD_SAMPLE_PICK(carnival)
STD_SAMPLE_FN(carnival)

static INT32 CarnivalInit()
{
	return DrvInit(0x4000, 0x8000, 0, carnival_write_port, carnival_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvCarnival = {
	"carnival", NULL, NULL, "carnival", "1980",
	"Carnival (upright)\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, carnivalRomInfo, carnivalRomName, carnivalSampleInfo, carnivalSampleName, CarnivalInputInfo, CarnivalDIPInfo,
	CarnivalInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Carnival (Head On hardware, set 1)

static struct BurnRomInfo carnivalhRomDesc[] = {
	{ "epr-155.u48",		0x0800, 0x0a5f1f65, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "epr-156.u47",		0x0800, 0x422221ff, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "epr-157.u46",		0x0800, 0x1551dffb, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "epr-158.u45",		0x0800, 0x9238b5c0, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "epr-159.u44",		0x0800, 0x5c2b9a33, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "epr-160.u43",		0x0800, 0xdd70471f, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "epr-161.u42",		0x0800, 0x42714a0d, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "epr-162.u41",		0x0800, 0x56e1c120, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "pr-62.u44",			0x0020, 0xf0084d80, 1 | BRF_GRA },           //  8 Color data

	{ "epr-412.u5",			0x0400, 0x0dbaa2b0, 3 | BRF_PRG | BRF_ESS }, //  9 I8039 Code

	{ "316-043.u65",		0x0020, 0xe60a7960, 0 | BRF_OPT },           // 10 Unused PROMs
	{ "316-042.u66",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 11
};

STD_ROM_PICK(carnivalh)
STD_ROM_FN(carnivalh)

static INT32 CarnivalhInit()
{
	return DrvInit(0x4000, 0x8000, 0, headon_write_port, carnivalh_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvCarnivalh = {
	"carnivalh", "carnival", NULL, "carnival", "1980",
	"Carnival (Head On hardware, set 1)\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, carnivalhRomInfo, carnivalhRomName, carnivalSampleInfo, carnivalSampleName, CarnivalhInputInfo, NULL,
	CarnivalhInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Carnival (Head On hardware, set 2)

static struct BurnRomInfo carnivalhaRomDesc[] = {
	{ "epr-155.u48",		0x0800, 0x0a5f1f65, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "epr-156.u47",		0x0800, 0x422221ff, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "epr-157.u46",		0x0800, 0x1551dffb, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "epr-158.u45",		0x0800, 0x9238b5c0, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "epr-159.u44",		0x0800, 0x5c2b9a33, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "epr-160.u43",		0x0800, 0xdd70471f, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "epr-161x.u42",		0x0800, 0x8133ba08, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "epr-162.u41",		0x0800, 0x56e1c120, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "pr-62.u44",			0x0020, 0xf0084d80, 1 | BRF_GRA },           //  8 Color data

	{ "epr-412.u5",			0x0400, 0x0dbaa2b0, 3 | BRF_PRG | BRF_ESS }, //  9 I8039 Code

	{ "316-043.u65",		0x0020, 0xe60a7960, 0 | BRF_OPT },           // 10 Unused PROMs
	{ "316-042.u66",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 11
};

STD_ROM_PICK(carnivalha)
STD_ROM_FN(carnivalha)

struct BurnDriver BurnDrvCarnivalha = {
	"carnivalha", "carnival", NULL, "carnival", "1980",
	"Carnival (Head On hardware, set 2)\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, carnivalhaRomInfo, carnivalhaRomName, carnivalSampleInfo, carnivalSampleName, CarnivalhInputInfo, NULL,
	CarnivalhInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Carnival (cocktail)

static struct BurnRomInfo carnivalcRomDesc[] = {
	{ "epr-501.u33",		0x0400, 0x688503d2, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "epr-652.u32",		0x0400, 0xa1f58beb, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "epr-653.u31",		0x0400, 0x67b17922, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "epr-654.u30",		0x0400, 0xbefb09a5, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "epr-655.u29",		0x0400, 0x623fcdad, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "epr-506.u28",		0x0400, 0xba916e97, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "epr-507.u27",		0x0400, 0xd0bda4a5, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "epr-508.u26",		0x0400, 0xf0258cad, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "epr-509.u8",			0x0400, 0xdcc8a530, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "epr-510.u7",			0x0400, 0x92c2ba51, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "epr-511.u6",			0x0400, 0x3af899a0, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "epr-512.u5",			0x0400, 0x09f7b3e6, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "epr-513.u4",			0x0400, 0x8f41974c, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "epr-514.u3",			0x0400, 0x2788d140, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "epr-515.u2",			0x0400, 0x10decaa9, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "epr-516.u1",			0x0400, 0x7c32b352, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "316-633",			0x0020, 0xf0084d80, 1 | BRF_GRA },           // 16 Color data

	{ "epr-412",			0x0400, 0x0dbaa2b0, 3 | BRF_PRG | BRF_ESS }, // 17 I8039 Code

	{ "316-0206.u14",		0x0020, 0x9617d796, 0 | BRF_OPT },           // 18 Unused PROM
};

STD_ROM_PICK(carnivalc)
STD_ROM_FN(carnivalc)

struct BurnDriver BurnDrvCarnivalc = {
	"carnivalc", "carnival", NULL, "carnival", "1980",
	"Carnival (cocktail)\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, carnivalcRomInfo, carnivalcRomName, carnivalSampleInfo, carnivalSampleName, CarnivalcInputInfo, CarnivalcDIPInfo,
	CarnivalInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Car Hunt / Deep Scan (France)

static struct BurnRomInfo carhntdsRomDesc[] = {
	{ "epr617.u33",			0x0800, 0x0bbfdb4e, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "epr618.u32",			0x0400, 0x5a080b1d, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "epr619.u31",			0x0400, 0xc6f2f399, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "epr620.u30",			0x0400, 0xd9deb88f, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "epr621.u29",			0x0400, 0x43e5de5c, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "epr622.u28",			0x0400, 0xc881a3bc, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "epr623.u27",			0x0400, 0x297e7f42, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "epr624.u26",			0x0400, 0xdc943125, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "epr625.u8",			0x0400, 0xc86a0842, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "epr626.u7",			0x0400, 0x9a48c939, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "epr627.u6",			0x0400, 0xb4b147e2, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "epr628.u5",			0x0400, 0xaecf3c26, 1 | BRF_PRG | BRF_ESS }, // 11
	{ "epr629.u4",			0x0400, 0xc5be665b, 1 | BRF_PRG | BRF_ESS }, // 12
	{ "epr630.u3",			0x0400, 0x4312388b, 1 | BRF_PRG | BRF_ESS }, // 13
	{ "epr631.u2",			0x0400, 0x6766c7e5, 1 | BRF_PRG | BRF_ESS }, // 14
	{ "epr632.u1",			0x0400, 0xae68b7d5, 1 | BRF_PRG | BRF_ESS }, // 15

	{ "316.0390.u49",		0x0020, 0xa0811288, 1 | BRF_GRA },           // 16 Color data
};

STD_ROM_PICK(carhntds)
STD_ROM_FN(carhntds)

static INT32 CarhntdsInit()
{
	return DrvInit(0x8000, 0x8000, 0, carhntds_write_port, carhntds_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvCarhntds = {
	"carhntds", NULL, NULL, NULL, "1979",
	"Car Hunt / Deep Scan (France)\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, carhntdsRomInfo, carhntdsRomName, NULL, NULL, CarhntdsInputInfo, CarhntdsDIPInfo,
	CarhntdsInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Head On 2

static struct BurnRomInfo headon2RomDesc[] = {
	{ "u27.bin",			0x0400, 0xfa47d2fb, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "u26.bin",			0x0400, 0x61c47b15, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "u25.bin",			0x0400, 0xbb16db92, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "u24.bin",			0x0400, 0x17a09f24, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "u23.bin",			0x0400, 0x0024895e, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "u22.bin",			0x0400, 0xf798304d, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "u21.bin",			0x0400, 0x4c19dd40, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "u20.bin",			0x0400, 0x25887ff2, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "316-0138.u44",		0x0020, 0x67104ea9, 1 | BRF_GRA },           //  8 Color data

	{ "316-0206.u65",		0x0020, 0x9617d796, 0 | BRF_OPT },           //  9 Unused PROM
};

STD_ROM_PICK(headon2)
STD_ROM_FN(headon2)

static INT32 Headon2Init()
{
	return DrvInit(0x2000, 0xc000, 0, headon2_write_port, headon2_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvHeadon2 = {
	"headon2", NULL, NULL, NULL, "1979",
	"Head On 2\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_RACING, 0,
	NULL, headon2RomInfo, headon2RomName, NULL, NULL, Headon2InputInfo, Headon2DIPInfo,
	Headon2Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};

// Head On 2 (Sidam bootleg)

static struct BurnRomInfo headon2sRomDesc[] = {
	{ "10304.0.9a",			0x0400, 0x256a1fc8, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "10304.1.8a",			0x0400, 0x61c47b15, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "10304.2.7a",			0x0400, 0xa6c268d4, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "10304.3.6a",			0x0400, 0x17a09f24, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "10304.4.5a",			0x0400, 0x9af8a2e0, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "10304.5.4a",			0x0400, 0x6975286c, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "10304.6.3a",			0x0400, 0x06fbcdce, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "10304.7b.2a",		0x0400, 0x3588fc8f, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "10303.3e",			0x0020, 0xe60a7960, 0 | BRF_OPT },           //  8 Unused PROMs
	{ "10302.2e",			0x0020, 0xa1506b9d, 0 | BRF_OPT },           //  9
};

STD_ROM_PICK(headon2s)
STD_ROM_FN(headon2s)

static INT32 Headon2sInit()
{
	return DrvInit(0x2000, 0xc000, 0, headon2_write_port, car2_read_port, NULL, NULL);
}

struct BurnDriverD BurnDrvHeadon2s = {
	"headon2s", "headon2", NULL, NULL, "1979",
	"Head On 2 (Sidam bootleg)\0", "No sound", "bootleg (Sidam)", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_RACING, 0,
	NULL, headon2sRomInfo, headon2sRomName, NULL, NULL, Headon2InputInfo, Headon2DIPInfo,
	Headon2sInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Car 2 (bootleg of Head On 2)

static struct BurnRomInfo car2RomDesc[] = {
	{ "car2.0",				0x0400, 0x37e031f9, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "car2.1",				0x0400, 0x61c47b15, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "car2.2",				0x0400, 0xa6c268d4, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "car2.3",				0x0400, 0x17a09f24, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "car2.4",				0x0400, 0x9af8a2e0, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "car2.5",				0x0400, 0x6975286c, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "car2.6",				0x0400, 0x4c19dd40, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "car2.7",				0x0400, 0x41a93920, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "316-0206.u65",		0x0020, 0x9617d796, 0 | BRF_OPT },           //  8 Unused PROM
};

STD_ROM_PICK(car2)
STD_ROM_FN(car2)

struct BurnDriver BurnDrvCar2 = {
	"car2", "headon2", NULL, NULL, "1979",
	"Car 2 (bootleg of Head On 2)\0", "No sound", "bootleg (RZ Bologna)", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_RACING, 0,
	NULL, car2RomInfo, car2RomName, NULL, NULL, Headon2InputInfo, Headon2DIPInfo,
	Headon2sInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	256, 224, 4, 3
};


// Space Attack (upright set 1)

static struct BurnRomInfo sspaceatRomDesc[] = {
	{ "155.u27",			0x0400, 0xba7bb86f, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "156.u26",			0x0400, 0x0b3a491c, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "157.u25",			0x0400, 0x3d3fac3b, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "158.u24",			0x0400, 0x843b80f6, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "159.u23",			0x0400, 0x1eacf60d, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "160.u22",			0x0400, 0xe61d482f, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "161.u21",			0x0400, 0xeb5e0993, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "162.u20",			0x0400, 0x5f84d550, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "316-0138.u44",		0x0020, 0x67104ea9, 1 | BRF_GRA },           //  8 Color data

	{ "316-0043.u65",		0x0020, 0xe60a7960, 0 | BRF_OPT },           //  9 Unused PROMs
	{ "316-0042.u66",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 10
};

STD_ROM_PICK(sspaceat)
STD_ROM_FN(sspaceat)

static INT32 SspaceatInit()
{
	return DrvInit(0x2000, 0xc000, 0, sspaceat_write_port,sspaceat_read_port , NULL, NULL);
}

struct BurnDriver BurnDrvSspaceat = {
	"sspaceat", NULL, NULL, NULL, "1979",
	"Space Attack (upright set 1)\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, sspaceatRomInfo, sspaceatRomName, NULL, NULL, SspaceatInputInfo, SspaceatDIPInfo,
	SspaceatInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Space Attack (upright set 2)

static struct BurnRomInfo sspaceat2RomDesc[] = {
	{ "81.u48",				0x0400, 0x3e4b29f6, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "58.u47",				0x0400, 0x176adb80, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "59.u46",				0x0400, 0xb2400d05, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "150.u45",			0x0400, 0xcf9bfa65, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "151.u44",			0x0400, 0x064530f1, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "152.u43",			0x0400, 0xc65c30fe, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "153.u42",			0x0400, 0xea70c7f6, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "156a.u41",			0x0400, 0x9029d2ce, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "316-0138.u44",		0x0020, 0x67104ea9, 1 | BRF_GRA },           //  8 Color data

	{ "316-0043.u65",		0x0020, 0xe60a7960, 0 | BRF_OPT },           //  9 Unused PROMs
	{ "316-0042.u66",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 10
};

STD_ROM_PICK(sspaceat2)
STD_ROM_FN(sspaceat2)

struct BurnDriver BurnDrvSspaceat2 = {
	"sspaceat2", "sspaceat", NULL, NULL, "1979",
	"Space Attack (upright set 2)\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, sspaceat2RomInfo, sspaceat2RomName, NULL, NULL, SspaceatInputInfo, SspaceatDIPInfo,
	SspaceatInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Space Attack (upright set 3)

static struct BurnRomInfo sspaceat3RomDesc[] = {
	{ "epr-115.u48",		0x0400, 0x9bc36d80, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "epr-116.u47",		0x0400, 0x2c2750b3, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "epr-117.u46",		0x0400, 0xfa7c2cc0, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "epr-118.u45",		0x0400, 0x273884ae, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "epr-119.u44",		0x0400, 0x1b53c6de, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "epr-120.u43",		0x0400, 0x60add585, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "epr-121.u42",		0x0400, 0x0979f72b, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "epr-122.u41",		0x0400, 0x45cb3486, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "316-0138.u44",		0x0020, 0x67104ea9, 1 | BRF_GRA },           //  8 Color data

	{ "316-0043.u65",		0x0020, 0xe60a7960, 0 | BRF_OPT },           //  9 Unused PROMs
	{ "316-0042.u66",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 10
};

STD_ROM_PICK(sspaceat3)
STD_ROM_FN(sspaceat3)

struct BurnDriver BurnDrvSspaceat3 = {
	"sspaceat3", "sspaceat", NULL, NULL, "1979",
	"Space Attack (upright set 3)\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, sspaceat3RomInfo, sspaceat3RomName, NULL, NULL, SspaceatInputInfo, SspaceatDIPInfo,
	SspaceatInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Space Attack (cocktail)

static struct BurnRomInfo sspaceatcRomDesc[] = {
	{ "139.u27",			0x0400, 0x9f2112fc, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "140.u26",			0x0400, 0xddbeed35, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "141.u25",			0x0400, 0xb159924d, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "142.u24",			0x0400, 0xf2ebfce9, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "143.u23",			0x0400, 0xbff34a66, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "144.u22",			0x0400, 0xfa062d58, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "145.u21",			0x0400, 0x7e950614, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "146.u20",			0x0400, 0x8ba94fbc, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "316-0138.u44",		0x0020, 0x67104ea9, 1 | BRF_GRA },           //  8 Color data

	{ "316-0043.u65",		0x0020, 0xe60a7960, 0 | BRF_OPT },           //  9 Unused PROMs
	{ "316-0042.u66",		0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 10
};

STD_ROM_PICK(sspaceatc)
STD_ROM_FN(sspaceatc)

struct BurnDriver BurnDrvSspaceatc = {
	"sspaceatc", "sspaceat", NULL, NULL, "1979",
	"Space Attack (cocktail)\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, sspaceatcRomInfo, sspaceatcRomName, NULL, NULL, SspaceatInputInfo, SspaceatDIPInfo,
	SspaceatInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// Space Attack / Head On

static struct BurnRomInfo sspacahoRomDesc[] = {
	{ "epr-0001.bin",		0x0800, 0xba62f57a, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "epr-0002.bin",		0x0800, 0x94b3c59c, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "epr-0003.bin",		0x0800, 0xdf13aef2, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "epr-0004.bin",		0x0800, 0x8431e15e, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "epr-0005.bin",		0x0800, 0xeec2b6e7, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "epr-0006.bin",		0x0800, 0x780e47ed, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "epr-0007.bin",		0x0800, 0x8189a2fa, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "epr-0008.bin",		0x0800, 0x34a64a80, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "316-0138.u44",		0x0020, 0x67104ea9, 1 | BRF_GRA },           //  8 Color data

	{ "316-0206.u14",		0x0020, 0x9617d796, 0 | BRF_OPT },           //  9 Unused PROMs
};

STD_ROM_PICK(sspacaho)
STD_ROM_FN(sspacaho)

static INT32 SspacahoInit()
{
	return DrvInit(0x4000, 0x8000, 0, sspacaho_write_port, sspacaho_read_port, NULL, NULL);
}

struct BurnDriver BurnDrvSspacaho = {
	"sspacaho", NULL, NULL, NULL, "1979",
	"Space Attack / Head On\0", "No sound", "Sega", "Vic Dual",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, sspacahoRomInfo, sspacahoRomName, NULL, NULL, SspacahoInputInfo, SspacahoDIPInfo,
	SspacahoInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};


// N-Sub (upright)

static struct BurnRomInfo nsubRomDesc[] = {
	{ "epr-268.u48",		0x0800, 0x485b4704, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "epr-269.u47",		0x0800, 0x32774ac9, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "epr-270.u46",		0x0800, 0xaf7ca40a, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "epr-271.u45",		0x0800, 0x3f9c180b, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "epr-272.u44",		0x0800, 0xd818aa51, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "epr-273.u43",		0x0800, 0x03a6f12a, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "epr-274.u42",		0x0800, 0xd69eb098, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "epr-275.u41",		0x0800, 0x1c7d90cc, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "pr-69.u11",			0x0020, 0xc94dd091, 1 | BRF_GRA },           //  8 Color data

	{ "pr33.u82",			0x0020, 0xe60a7960, 0 | BRF_OPT },           //  9 Unused PROMs
	{ "pr34.u83",			0x0020, 0xa1506b9d, 0 | BRF_OPT },           // 10
};

STD_ROM_PICK(nsub)
STD_ROM_FN(nsub)

static struct BurnSampleInfo nsubSampleDesc[] = {
	{ "SND_BOAT", SAMPLE_NOLOOP },
	{ "SND_BONUS0", SAMPLE_NOLOOP },
	{ "SND_BONUS1", SAMPLE_NOLOOP },
	{ "SND_CODE", SAMPLE_NOLOOP },
	{ "SND_EXPL_L0", SAMPLE_NOLOOP },
	{ "SND_EXPL_L1", SAMPLE_NOLOOP },
	{ "SND_EXPL_S0", SAMPLE_NOLOOP },
	{ "SND_EXPL_S1", SAMPLE_NOLOOP },
	{ "SND_LAUNCH0", SAMPLE_NOLOOP },
	{ "SND_LAUNCH1", SAMPLE_NOLOOP },
	{ "SND_SONAR", SAMPLE_NOLOOP },
	{ "SND_WARNING0", SAMPLE_NOLOOP },
	{ "SND_WARNING1", SAMPLE_NOLOOP },
	{ "", 0 }
};

STD_SAMPLE_PICK(nsub)
STD_SAMPLE_FN(nsub)

static void nsub_callback()
{
	for (INT32 i = 0; i < 0x20; i++) { // invert color prom
		DrvColPROM[i] ^= 0x77;
	}
}

static INT32 NsubInit()
{
	return DrvInit(0x4000, 0xc000, 0, nsub_write_port, nsub_read_port, NULL, nsub_callback);
}

struct BurnDriver BurnDrvNsub = {
	"nsub", NULL, NULL, "nsub", "1980",
	"N-Sub (upright)\0", NULL, "Sega", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, nsubRomInfo, nsubRomName, nsubSampleInfo, nsubSampleName, NsubInputInfo, NULL,
	NsubInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 8,
	224, 256, 3, 4
};

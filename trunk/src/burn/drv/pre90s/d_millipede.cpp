// Millipede emu-layer for FB Alpha by dink, based on Ivan Mackintosh Millipede/Centipede emulator and MAME driver.
#include "tiles_generic.h"
#include "m6502_intf.h"
#include "pokey.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv6502ROM;
static UINT8 *Drv6502RAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSpriteRAM;

static UINT8 *DrvBGGFX;
static UINT8 *DrvSpriteGFX;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 m_dsw_select;
static UINT8 m_control_select;
static UINT32 m_flipscreen = 0;
static UINT32 vblank;
// transmask stuff
UINT8 m_penmask[64];
// trackball stuff
static int oldpos[4];
static UINT8 sign[4];

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvJoy4[8];
static UINT8 DrvDip[4];
static UINT8 DrvInput[4];
static UINT8 DrvReset;

static struct BurnInputInfo MillipedInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy3 + 3,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy3 + 1,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 coin"},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy4 + 3,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy4 + 2,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy4 + 1,	"p2 left"},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy4 + 0,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 7,	"service"},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 4,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDip + 0,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvDip + 1,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvDip + 2,	"dip"},
	{"Dip D",		BIT_DIPSWITCH,	DrvDip + 3,	"dip"},
};

STDINPUTINFO(Milliped)


static struct BurnDIPInfo MillipedDIPList[]=
{
	{0x11, 0xff, 0xff, 0x04, NULL		},
	{0x12, 0xff, 0xff, 0x60, NULL		},
	{0x13, 0xff, 0xff, 0x14, NULL		},
	{0x14, 0xff, 0xff, 0x02, NULL		},

	{0   , 0xfe, 0   ,    4, "Language"		},
	{0x11, 0x01, 0x03, 0x00, "English"		},
	{0x11, 0x01, 0x03, 0x01, "German"		},
	{0x11, 0x01, 0x03, 0x02, "French"		},
	{0x11, 0x01, 0x03, 0x03, "Spanish"		},

	{0   , 0xfe, 0   ,    4, "Bonus"		},
	{0x11, 0x01, 0x0c, 0x00, "0"		},
	{0x11, 0x01, 0x0c, 0x04, "0 1x"		},
	{0x11, 0x01, 0x0c, 0x08, "0 1x 2x"		},
	{0x11, 0x01, 0x0c, 0x0c, "0 1x 2x 3x"		},

	{0   , 0xfe, 0   ,    2, "Credit Minimum"		},
	{0x12, 0x01, 0x04, 0x00, "1"		},
	{0x12, 0x01, 0x04, 0x04, "2"		},

	{0   , 0xfe, 0   ,    2, "Coin Counters"		},
	{0x12, 0x01, 0x08, 0x00, "1"		},
	{0x12, 0x01, 0x08, 0x08, "2"		},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x12, 0x01, 0x20, 0x20, "Upright"		},
	{0x12, 0x01, 0x20, 0x00, "Cocktail"		},

	{0   , 0xfe, 0   ,    0, "Millipede Head"		},
	{0x13, 0x01, 0x01, 0x00, "Easy"		},
	{0x13, 0x01, 0x01, 0x01, "Hard"		},

	{0   , 0xfe, 0   ,    2, "Beetle"		},
	{0x13, 0x01, 0x02, 0x00, "Easy"		},
	{0x13, 0x01, 0x02, 0x02, "Hard"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x0c, 0x00, "2"		},
	{0x13, 0x01, 0x0c, 0x04, "3"		},
	{0x13, 0x01, 0x0c, 0x08, "4"		},
	{0x13, 0x01, 0x0c, 0x0c, "5"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x13, 0x01, 0x30, 0x00, "12000"		},
	{0x13, 0x01, 0x30, 0x10, "15000"		},
	{0x13, 0x01, 0x30, 0x20, "20000"		},
	{0x13, 0x01, 0x30, 0x30, "None"		},

	{0   , 0xfe, 0   ,    2, "Spider"		},
	{0x13, 0x01, 0x40, 0x00, "Easy"		},
	{0x13, 0x01, 0x40, 0x40, "Hard"		},

	{0   , 0xfe, 0   ,    2, "Starting Score Select"		},
	{0x13, 0x01, 0x80, 0x80, "Off"		},
	{0x13, 0x01, 0x80, 0x00, "On"		},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x14, 0x01, 0x03, 0x03, "2 Coins 1 Credits"		},
	{0x14, 0x01, 0x03, 0x02, "1 Coin  1 Credits"		},
	{0x14, 0x01, 0x03, 0x01, "1 Coin  2 Credits"		},
	{0x14, 0x01, 0x03, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    4, "Right Coin"		},
	{0x14, 0x01, 0x0c, 0x00, "*1"		},
	{0x14, 0x01, 0x0c, 0x04, "*4"		},
	{0x14, 0x01, 0x0c, 0x08, "*5"		},
	{0x14, 0x01, 0x0c, 0x0c, "*6"		},

	{0   , 0xfe, 0   ,    2, "Left Coin"		},
	{0x14, 0x01, 0x10, 0x00, "*1"		},
	{0x14, 0x01, 0x10, 0x10, "*2"		},

	{0   , 0xfe, 0   ,    7, "Bonus Coins"		},
	{0x14, 0x01, 0xe0, 0x00, "None"		},
	{0x14, 0x01, 0xe0, 0x20, "3 credits/2 coins"		},
	{0x14, 0x01, 0xe0, 0x40, "5 credits/4 coins"		},
	{0x14, 0x01, 0xe0, 0x60, "6 credits/4 coins"		},
	{0x14, 0x01, 0xe0, 0x80, "6 credits/5 coins"		},
	{0x14, 0x01, 0xe0, 0xa0, "4 credits/3 coins"		},
	{0x14, 0x01, 0xe0, 0xc0, "Demo Mode"		},
};

STDDIPINFO(Milliped)

void milliped_set_color(UINT16 offset, UINT8 data)
{
	UINT32 color;
	int bit0, bit1, bit2;
	int r, g, b;

	/* red component */
	bit0 = (~data >> 5) & 0x01;
	bit1 = (~data >> 6) & 0x01;
	bit2 = (~data >> 7) & 0x01;
	r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	/* green component */
	bit0 = 0;
	bit1 = (~data >> 3) & 0x01;
	bit2 = (~data >> 4) & 0x01;
	g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	/* blue component */
	bit0 = (~data >> 0) & 0x01;
	bit1 = (~data >> 1) & 0x01;
	bit2 = (~data >> 2) & 0x01;
	b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	color = BurnHighCol(r, g, b, 0);

	/* character colors, set directly */
	if (offset < 0x10)
		DrvPalette[offset] = color;

	/* sprite colors - set all the applicable ones */
	else
	{
		int i;

		int base = offset & 0x0c;

		offset = offset & 0x03;

		for (i = (base << 6); i < (base << 6) + 0x100; i += 4)
		{
			if (offset == ((i >> 2) & 0x03))
				DrvPalette[i + 0x100 + 1] = color;

			if (offset == ((i >> 4) & 0x03))
				DrvPalette[i + 0x100 + 2] = color;

			if (offset == ((i >> 6) & 0x03))
				DrvPalette[i + 0x100 + 3] = color;
		}
	}
}

static void millipede_write(UINT16 address, UINT8 data)
{
	address &= 0x7fff; // 15bit addressing
	if (address >= 0x1000 && address <= 0x13bf) { // Video Ram
		DrvVidRAM[address - 0x1000] = data;
		return;
	}
	if (address >= 0x13c0 && address <= 0x13ff) { // Sprite Ram
		DrvSpriteRAM[address - 0x13c0] = data;
		return;
	}
	if (address >= 0x2480 && address <= 0x249f) { // Palette Ram
		DrvPalRAM[address - 0x2480] = data;
		milliped_set_color(address - 0x2480, data);
		return;
	}

	if ((address & 0x400) == 0x400) { // Pokey 1
		if (address >= 0x400 && address <= 0x40f) {
			pokey1_w(address - 0x400, data);
		}
		return;
	}
	if ((address & 0x800) == 0x800) { // Pokey 2
		if (address >= 0x800 && address <= 0x80f) {
			pokey2_w(address - 0x800, data);
		}
		return;
	}
	switch (address)
	{
		case 0x2505:
			m_dsw_select = (~data >> 7) & 1;
		return;

		case 0x2506:
			m_flipscreen = data >> 7;
		return;
		case 0x2507:
			m_control_select = (data >> 7) & 1;
		return;
		case 0x2600:
			M6502SetIRQLine(0, CPU_IRQSTATUS_NONE);
		return;
	}
	//bprintf(0, _T("mw %X,"), address);
}

INLINE int read_trackball(int idx, int switch_port)
{
	int newpos;

	/* adjust idx if we're cocktail flipped */
	if (m_flipscreen)
		idx += 2;

	/* if we're to read the dipswitches behind the trackball data, do it now */
	if (m_dsw_select)
		return (DrvInput[switch_port] & 0x7f) | sign[idx];

	/* get the new position and adjust the result */
	//newpos = readinputport(6 + idx);
	newpos = 0; // no trackball!! -dink
	if (newpos != oldpos[idx])
	{
		sign[idx] = (newpos - oldpos[idx]) & 0x80;
		oldpos[idx] = newpos;
	}

	/* blend with the bits from the switch port */
	return (DrvInput[switch_port] & 0x70) | (oldpos[idx] & 0x0f) | sign[idx];
}


static UINT8 millipede_read(UINT16 address)
{
	address &= 0x7fff; // 15bit addressing
	if (address >= 0x1000 && address <= 0x13bf) { // Video Ram
		return DrvVidRAM[address - 0x1000];
	}
	if (address >= 0x13c0 && address <= 0x13ff) { // Sprite Ram
		return DrvSpriteRAM[address - 0x13c0];
	}
	if (address >= 0x2480 && address <= 0x249f) { // Palette Ram
		return DrvPalRAM[address - 0x2480];
	}
	if (address >= 0x4000 && address <= 0x7fff) { // ROM
		return Drv6502ROM[address];
	}

	switch (address)
	{
		case 0x2000: return ((read_trackball(0, 0) | DrvDip[0]) & 0x3f) | ((vblank) ? 0x40 : 0x00);
		case 0x2001: return read_trackball(1, 1) | DrvDip[1] ;
		case 0x2010: return DrvInput[2];
		case 0x2011: return DrvInput[3];
		case 0x0400:
		case 0x0401:
		case 0x0402:
		case 0x0403:
		case 0x0404:
		case 0x0405:
		case 0x0406:
		case 0x0407: return pokey1_r(address);
		case 0x0408: return DrvDip[2];
		case 0x0409:
		case 0x040a:
		case 0x040b:
		case 0x040c:
		case 0x040d:
		case 0x040e:
		case 0x040f: return pokey1_r(address);
		case 0x0800:
		case 0x0801:
		case 0x0802:
		case 0x0803:
		case 0x0804:
		case 0x0805:
		case 0x0806:
		case 0x0807: return pokey2_r(address);
		case 0x0808: return DrvDip[3];
		case 0x0809:
		case 0x080a:
		case 0x080b:
		case 0x080c:
		case 0x080d:
		case 0x080e:
		case 0x080f: return pokey2_r(address);
	}

	//bprintf(0, _T("mr %X,"), address);

	return 0;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	M6502Open(0);
	M6502Reset();
	M6502Close();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv6502ROM		= Next; Next += 0x012000;

	DrvPalette		= (UINT32*)Next; Next += 0x0600 * sizeof(UINT32);
	DrvBGGFX        = Next; Next += 0x10000;
	DrvSpriteGFX    = Next; Next += 0x10000;

	AllRam			= Next;

	Drv6502RAM		= Next; Next += 0x00400;
	DrvVidRAM		= Next; Next += 0x01000;
	DrvSpriteRAM	= Next; Next += 0x01000;
	DrvPalRAM		= Next; Next += 0x01000;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

void init_penmask()
{
	int i;

	for (i = 0; i < 64; i++)
	{
		UINT8 mask = 1;
		if (((i >> 0) & 3) == 0) mask |= 2;
		if (((i >> 2) & 3) == 0) mask |= 4;
		if (((i >> 4) & 3) == 0) mask |= 8;
		m_penmask[i] = mask;
	}
}

static INT32 CharPlaneOffsets[2] = { 256*8*8, 0  };
static INT32 CharXOffsets[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 CharYOffsets[8] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 };

static INT32 SpritePlaneOffsets[2] = { 128*16*8, 0 };
static INT32 SpriteXOffsets[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0 };
static INT32 SpriteYOffsets[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
                                    8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 };

static INT32 DrvInit()
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv6502ROM + 0x4000, 0, 1)) return 1;
		if (BurnLoadRom(Drv6502ROM + 0x5000, 1, 1)) return 1;
		if (BurnLoadRom(Drv6502ROM + 0x6000, 2, 1)) return 1;
		if (BurnLoadRom(Drv6502ROM + 0x7000, 3, 1)) return 1;

		UINT8 *DrvTempRom = (UINT8 *)BurnMalloc(0x10000);
		memset(DrvTempRom, 0, 0x10000);
		if (BurnLoadRom(DrvTempRom         , 4, 1)) return 1;
		if (BurnLoadRom(DrvTempRom+0x800   , 5, 1)) return 1;
		GfxDecode(0x100, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, DrvTempRom, DrvBGGFX);
		GfxDecode(0x80, 2, 8, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x80, DrvTempRom, DrvSpriteGFX);
		BurnFree(DrvTempRom);
	}

	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(Drv6502RAM,	0x0000, 0x03ff, MAP_RAM);
	M6502MapMemory(Drv6502ROM + 0x4000,	0x4000, 0x7fff, MAP_ROM);
	M6502SetWriteHandler(millipede_write);
	M6502SetReadHandler(millipede_read);
	M6502SetWriteMemIndexHandler(millipede_write);
	M6502SetReadMemIndexHandler(millipede_read);
	M6502SetReadOpArgHandler(millipede_read);
	M6502SetReadOpHandler(millipede_read);
	M6502Close();

	PokeyInit(12096000/8, 2, 6, 0);

	init_penmask();

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	PokeyExit();

	M6502Exit();

	BurnFree(AllMem);

	return 0;
}


extern int counter;


static void draw_bg()
{
	UINT8 *videoram = DrvVidRAM;
	for (INT32 offs = 0; offs <= 0x3bf; offs++)
	{
		int sx = offs % 32;
		int sy = offs / 32;

		int data = videoram[offs];
		int bank = ((data >> 6) & 1);
		int color = (data >> 6) & 3;
		int code = (data & 0x3f) + 0x40 + (bank * 0x80);

		// Flip both x and y if flipscreen is non-zero
		int flip_tiles = (m_flipscreen) ? 0x03 : 0;

		sx = 8 * sx;
		sy = 8 * sy;
		if (sx >= nScreenWidth) continue;
		if (sy >= nScreenHeight) continue;

		if (flip_tiles) {
			Render8x8Tile_FlipXY_Clip(pTransDraw, code, 248 - sx, 184 - sy, color, 2, 0, DrvBGGFX);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 2, 0, DrvBGGFX);
		}
	}
}

void RenderTileCPMP(INT32 code, INT32 color, INT32 sx, INT32 sy, INT32 flipx, INT32 flipy, INT32 width, INT32 height)
{
	UINT16 *dest = pTransDraw;
	UINT8 *gfx = DrvSpriteGFX;

	INT32 flip = 0;
	if (flipy) flip |= (height - 1) * width;
	if (flipx) flip |= width - 1;

	gfx += code * width * height;

	for (INT32 y = 0; y < height; y++, sy++) {
		if (sy < 0 || sy >= nScreenHeight) continue;

		for (INT32 x = 0; x < width; x++, sx++) {
			if (sx < 0 || sx >= nScreenWidth) continue;

			INT32 pxl = gfx[((y * width) + x) ^ flip];

			if (m_penmask[color & 0x3f] & (1 << pxl) || !pxl) continue; // is this right?
			dest[sy * nScreenWidth + sx] = pxl | (color << 2) | 0x100;
		}
		sx -= width;
	}
}


static void draw_sprites()
{
	UINT8 *spriteram = DrvSpriteRAM;
	for (INT32 offs = 0; offs < 0x10; offs++)
	{
		int code = ((spriteram[offs] & 0x3e) >> 1) | ((spriteram[offs] & 0x01) << 6);
		int color = spriteram[offs + 0x30];
		int flipx = m_flipscreen;
		int flipy = (spriteram[offs] & 0x80);
		int x = spriteram[offs + 0x20];
		int y = 240 - spriteram[offs + 0x10];
		if (flipx) {
			flipy = !flipy;
		}

		if (x+8>=nScreenWidth) continue; // clip top 8px of sprites (top of screen)

		RenderTileCPMP(code, color, x, y, flipx, flipy, 8, 16);
	}
}

static INT32 DrvDraw()
{
	BurnTransferClear();

    draw_bg();
	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static void DrvMakeInputs()
{
	// Reset Inputs - bring active-LOW stuff HIGH
	DrvInput[0] = 0x10 + 0x20;
	DrvInput[1] = 0x01 + 0x02 + 0x10 + 0x20 + 0x40;
	DrvInput[2] = 0xff;
	DrvInput[3] = 0x01 + 0x02 + 0x04 + 0x08 + 0x10 + 0x40 + 0x80;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] -= (DrvJoy1[i] & 1) << i;
		DrvInput[1] -= (DrvJoy2[i] & 1) << i;
		DrvInput[2] -= (DrvJoy3[i] & 1) << i;
		DrvInput[3] -= (DrvJoy4[i] & 1) << i;
	}

}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}
	DrvMakeInputs();

	M6502NewFrame();

	INT32 nTotalCycles = 12096000/8 / 60;
	INT32 nInterleave = 4;

	vblank = 0;

	M6502Open(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		M6502Run(nTotalCycles / nInterleave);
		M6502SetIRQLine(0,CPU_IRQSTATUS_AUTO);

		if (i == 2)
		    vblank = 1;
	}

	M6502Close();
	if (pBurnSoundOut) {
		pokey_update(0, pBurnSoundOut, nBurnSoundLen);
		pokey_update(1, pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029722;
	}

	if (nAction & ACB_VOLATILE) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		M6502Scan(nAction);

	}

	return 0;
}


// Millipede

static struct BurnRomInfo millipedRomDesc[] = {
	{ "136013-104.1mn",	0x1000, 0x40711675, 1 | BRF_PRG | BRF_ESS }, //  0 maincpu
	{ "136013-103.1l",	0x1000, 0xfb01baf2, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "136013-102.1jk",	0x1000, 0x62e137e0, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "136013-101.1h",	0x1000, 0x46752c7d, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "136013-107.5r",	0x0800, 0x68c3437a, 2 | BRF_PRG | BRF_ESS }, //  4 gfx1
	{ "136013-106.5p",	0x0800, 0xf4468045, 2 | BRF_PRG | BRF_ESS }, //  5

	{ "136001-213.7e",	0x0100, 0x6fa3093a, 3 | BRF_PRG | BRF_ESS }, //  6 proms
};

STD_ROM_PICK(milliped)
STD_ROM_FN(milliped)

struct BurnDriver BurnDrvMilliped = {
	"milliped", NULL, NULL, NULL, "1982",
	"Millipede\0", NULL, "Atari", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, millipedRomInfo, millipedRomName, NULL, NULL, MillipedInputInfo, MillipedDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x600,
	240, 256, 3, 4
};


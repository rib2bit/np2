#include	"compiler.h"
#include	"vramhdl.h"
#include	"menubase.h"


#define	MICON_MAX			16
#define	MICON_CACHE			8


typedef struct {
//	UINT16	type;
const void	*res;
} ICONREG;

typedef struct {
	UINT16	id;
	UINT16	count;
	VRAMHDL	hdl;
} ICONCACHE;


static ICONREG		iconreg[MICON_MAX - 1];
static ICONCACHE	iconcache[MICON_CACHE];

static const BYTE icon24a[178] = {		// 32x32
		0x07,0x17,0x00,0x00,0x80,0x01,0x7f,0xff,0x01,0x16,0x2b,0x56,0x34,
		0x44,0xff,0x2e,0x29,0x31,0x2f,0x2c,0xaf,0x32,0xac,0x2e,0x3b,0x31,
		0x23,0x2f,0xc2,0x00,0x14,0xff,0x2e,0x14,0x00,0x02,0x26,0x9d,0x10,
		0x16,0x2e,0x27,0x00,0x05,0x2e,0x1a,0x10,0x19,0xff,0x8c,0x9e,0x00,
		0x0e,0x2e,0x1a,0x10,0x16,0x5f,0xaa,0x20,0x91,0x3e,0xa0,0x2f,0x8b,
		0xff,0xeb,0x1a,0x23,0x94,0x2e,0x22,0x31,0x0c,0x2f,0x9a,0x26,0x91,
		0x2e,0x28,0x2f,0xa7,0xff,0x31,0x1a,0x1a,0x9d,0x2f,0xa9,0x31,0x14,
		0x19,0x20,0x2f,0xff,0x8f,0xe6,0xef,0xdc,0xff,0x26,0x94,0x31,0x28,
		0x2f,0x86,0x31,0x17,0x23,0x9a,0x31,0x22,0x2e,0x09,0x2f,0x94,0xff,
		0x20,0xa0,0x10,0x1f,0x2f,0x9e,0x20,0x8b,0x9e,0xa0,0x5f,0xa0,0x31,
		0x1a,0xe0,0x9c,0xff,0x2e,0x27,0xf4,0x20,0x01,0x2b,0x2e,0x15,0x2f,
		0xe2,0x31,0x43,0x2e,0x1b,0x31,0x0a,0xff,0x32,0xb2,0x1a,0x83,0x00,
		0x29,0x31,0x2e,0x2c,0xaa,0x64,0x10,0x34,0x1a,0x10,0x06,0xf8,0x00,
		0x3e,0x34,0x0a,0x01,0x0c,0x00,0x7f,0x00,0x66};

static const BYTE icon24am[83] = {		// 32x32
		0x06,0x5f,0x00,0x00,0x09,0xff,0x00,0x06,0x04,0x8a,0x07,0x11,0x08,
		0x96,0x07,0x8d,0xff,0x08,0x0f,0x07,0x4f,0x08,0x4e,0x07,0x93,0x08,
		0x0a,0x07,0x95,0x08,0x09,0x07,0xd7,0xfe,0x08,0x06,0x07,0x98,0x08,
		0x05,0x07,0x9a,0x07,0xe0,0x08,0x03,0x07,0xdc,0xff,0xff,0x07,0x9e,
		0x07,0xff,0x00,0x3f,0x00,0x3f,0x00,0x21,0x38,0x3f,0x57,0xdf,0x68,
		0x21,0xff,0x78,0x7b,0x98,0x24,0xa8,0x61,0xb8,0x5f,0xc8,0x60,0xd8,
		0x60,0xe8,0x61,0xf8,0x56};

static const BYTE icon24b[195] = {		// 32x32
		0x06,0x61,0x80,0x00,0x3f,0x00,0x3f,0xc0,0xc0,0xc0,0xff,0x00,0x10,
		0xff,0x05,0x02,0x15,0xbf,0x15,0x93,0x1a,0x3f,0x16,0x6b,0x19,0x75,
		0x16,0x6f,0x00,0x0b,0x73,0x00,0x00,0x1c,0x17,0x17,0x33,0x83,0x00,
		0x00,0x00,0x8e,0x38,0xd7,0xff,0x17,0x35,0x13,0x49,0x18,0xb4,0x5c,
		0xdb,0x13,0x45,0x04,0x51,0x05,0xd1,0x24,0x8a,0xff,0x2f,0x26,0x13,
		0x55,0x17,0xdf,0x30,0x85,0x8c,0x27,0x13,0x4d,0x62,0xcf,0x93,0x99,
		0xff,0x18,0x8b,0x9e,0x1d,0x5b,0x4c,0x17,0x22,0x30,0x91,0xb9,0x29,
		0x17,0x26,0x17,0xf5,0xff,0x17,0x29,0x17,0xfb,0xf7,0x66,0x2f,0xf9,
		0xf2,0xe8,0x8f,0xe4,0x00,0x34,0x8f,0xce,0xff,0x30,0xa3,0x92,0x0f,
		0x2f,0x28,0x30,0xa7,0xa6,0x68,0x2f,0x11,0x30,0xa4,0xbe,0x74,0xff,
		0x00,0x0b,0x05,0xc2,0x47,0xff,0x17,0x19,0x18,0xa3,0x5d,0xb2,0x18,
		0x94,0x93,0xb2,0xff,0x5c,0xd7,0x18,0x97,0x19,0x69,0x16,0x45,0x17,
		0x29,0x19,0x4e,0x1a,0x17,0x15,0x88,0xff,0x16,0x72,0x1a,0x23,0x15,
		0xbf,0x7f,0x5f,0x17,0xff,0x17,0xff,0x18,0xbf,0x17,0xff,0xfe,0x18,
		0x9f,0x17,0xff,0x7c,0x62,0x11,0x7f,0x00,0x3f,0x00,0x3f,0x00,0x26};

static const BYTE icon24bm[97] = {		// 32x32
		0x06,0x5f,0x00,0x00,0x09,0xff,0x00,0x06,0x04,0x8a,0x07,0x11,0x08,
		0x95,0x07,0x4d,0xff,0x08,0x50,0x07,0x91,0x08,0x0c,0x07,0x93,0x08,
		0x0a,0x07,0x95,0x08,0x08,0x07,0x97,0xff,0x08,0x06,0x07,0x99,0x08,
		0x05,0x07,0xdb,0x08,0x02,0x07,0x9c,0x08,0x02,0x07,0xfe,0xff,0x00,
		0x3f,0x00,0x3f,0x30,0x3f,0x48,0x5f,0x60,0x63,0x70,0x5f,0x80,0x60,
		0x90,0x60,0xff,0xa0,0x60,0xb0,0x60,0xc0,0x60,0xd0,0x61,0x08,0x4a,
		0x07,0xd9,0x08,0x05,0x07,0xda,0xfc,0x08,0x04,0x07,0xdb,0x08,0x03,
		0x07,0xdc,0x08,0x02,0x03,0x0a};

static const BYTE icon24c[200] = {		// 32x32
		0x07,0x19,0x00,0x80,0x80,0x01,0x7f,0x01,0x04,0xff,0xff,0x01,0x02,
		0x0e,0xc0,0xc0,0xc0,0x00,0x00,0x50,0x2e,0x07,0x31,0x0b,0x80,0xff,
		0x00,0x49,0x2f,0x8e,0x01,0x03,0x2f,0xc9,0x2e,0x12,0x61,0x4c,0x2f,
		0x94,0x61,0x47,0xff,0x2e,0x18,0x61,0x46,0x2f,0x9a,0x61,0x41,0x2e,
		0x1e,0x61,0x40,0x2f,0x8b,0x25,0x05,0xff,0x00,0x05,0x05,0x83,0x61,
		0x40,0x5e,0x0f,0x00,0x0e,0x61,0x40,0x2f,0xa6,0x61,0x35,0xff,0xbc,
		0x92,0x2f,0x99,0x61,0x32,0x2f,0xac,0x61,0x2f,0x2e,0x12,0x2f,0x9f,
		0x61,0x2c,0xff,0x2f,0x95,0x0a,0x02,0x00,0x08,0x40,0x16,0x2e,0x39,
		0x50,0x87,0x00,0x05,0x05,0x83,0xff,0x10,0x16,0x2f,0xb8,0x91,0x0c,
		0x10,0x19,0x2e,0x39,0x91,0x08,0x11,0x99,0x61,0x20,0xff,0x2f,0x9d,
		0x1f,0x05,0x8e,0x14,0x61,0x23,0x5e,0x1e,0x91,0x08,0x10,0x1c,0x61,
		0x1a,0xff,0x2f,0xa4,0x01,0x23,0x2f,0x93,0x2e,0x23,0x4d,0x85,0x91,
		0x20,0x61,0x14,0x8e,0x27,0xff,0x00,0x08,0x61,0x2b,0x8c,0xa7,0x2f,
		0xa8,0x61,0x0e,0x2f,0xa6,0x8f,0xa6,0x61,0x11,0xff,0xec,0xc8,0x2f,
		0xe7,0xf1,0x11,0x31,0x4a,0x2e,0x11,0x31,0x09,0x00,0x46,0x2e,0x11,
		0xc0,0x00,0x7f,0x00,0x3c};

static const BYTE icon24cm[106] = {		// 32x32
		0x06,0x47,0x00,0x00,0x0b,0xff,0xff,0xff,0x03,0xcc,0x00,0x0e,0x07,
		0x82,0xff,0x08,0x1b,0x07,0x84,0x08,0x5a,0x07,0xc7,0x08,0x16,0x07,
		0x88,0x08,0x16,0x07,0xde,0xff,0x07,0x8a,0x08,0x14,0x07,0xde,0x07,
		0x8c,0x08,0x12,0x07,0xde,0x07,0x8e,0x08,0x10,0xff,0x07,0xde,0x07,
		0x90,0x08,0x0e,0x07,0xde,0x07,0x92,0x08,0x0c,0x07,0xde,0x07,0x94,
		0xff,0x08,0x0a,0x07,0xde,0x07,0x96,0x08,0x08,0x07,0xde,0x07,0x98,
		0x08,0x06,0x07,0xde,0xff,0x07,0x9a,0x08,0x04,0x07,0xde,0x07,0x9c,
		0x08,0x02,0x07,0xfe,0x00,0x20,0x18,0x20,0xe0,0x08,0x1e,0x50,0x63,
		0x60,0x5b};

static const BYTE icon24d[179] = {		// 32x32
		0x07,0x43,0x80,0x00,0x7f,0xc0,0xc0,0xc0,0xff,0x00,0x10,0x0a,0x02,
		0xf8,0x2b,0x53,0x34,0x44,0x2c,0xa6,0x32,0xb5,0x1f,0x17,0xff,0x00,
		0x00,0xdf,0x01,0x08,0x40,0x17,0x00,0x00,0x1c,0x2e,0x17,0x1c,0x85,
		0x01,0x0f,0x14,0x18,0xff,0x2e,0x31,0x2f,0xab,0x31,0x14,0xb9,0xa0,
		0x8f,0xa6,0x16,0x08,0x5e,0x2d,0x00,0x31,0xff,0x29,0x85,0x00,0x08,
		0x47,0x9d,0x00,0x32,0x61,0x05,0x2f,0x88,0x46,0xa0,0xbf,0xac,0xff,
		0x61,0x11,0x79,0xa6,0x2f,0xff,0x2f,0xff,0x2f,0xd8,0xf7,0x02,0x2f,
		0xcd,0x28,0x02,0xff,0x2f,0x8b,0x61,0x24,0x2f,0xab,0x2e,0x0e,0x61,
		0x24,0x2f,0xa8,0x5e,0x11,0x61,0x1e,0xff,0x2c,0x8e,0x32,0x9f,0x2e,
		0x14,0x00,0x02,0x0b,0x82,0xf4,0x24,0x8c,0xae,0x31,0x4a,0xff,0x2e,
		0x1d,0x31,0x08,0xc5,0xa1,0xb9,0xa8,0x31,0x17,0x32,0xa9,0x2c,0x85,
		0x2e,0x29,0xff,0x32,0x8e,0x34,0x17,0x2b,0x08,0x2c,0xb2,0x34,0x23,
		0x2b,0x41,0xfe,0x9d,0x2f,0xff,0xff,0x31,0x3f,0x2f,0xd6,0x31,0x08,
		0x2f,0xd9,0xf8,0x88,0x2f,0xd9,0x00,0x7f,0x00,0x0c};

static const BYTE icon24dm[97] = {		// 32x32
		0x06,0x5f,0x00,0x00,0x09,0xff,0x00,0x06,0x04,0x8a,0x07,0x11,0x08,
		0x95,0x07,0x4d,0xff,0x08,0x50,0x07,0x91,0x08,0x0c,0x07,0x93,0x08,
		0x0a,0x07,0x95,0x08,0x08,0x07,0x97,0xff,0x08,0x06,0x07,0x99,0x08,
		0x05,0x07,0xdb,0x08,0x02,0x07,0x9c,0x08,0x02,0x07,0xfe,0xff,0x00,
		0x3f,0x00,0x3f,0x30,0x3f,0x48,0x5f,0x60,0x63,0x70,0x5f,0x80,0x60,
		0x90,0x60,0xff,0xa0,0x60,0xb0,0x60,0xc0,0x60,0xd0,0x61,0x08,0x4a,
		0x07,0xd9,0x08,0x05,0x07,0xda,0xfc,0x08,0x04,0x07,0xdb,0x08,0x03,
		0x07,0xdc,0x08,0x02,0x03,0x0a};

static const BYTE iconfld[75] = {		// 32x32
		0x09,0x18,0x00,0x9c,0x9c,0x04,0x84,0x00,0x41,0x9c,0x9c,0xff,0x3f,
		0xff,0xff,0x06,0x02,0x04,0x14,0xc4,0x3f,0xb8,0x1c,0xc4,0x3f,0xb8,
		0x22,0x83,0xc4,0x39,0x9c,0x9c,0x63,0xce,0xce,0x04,0x29,0xc4,0x03,
		0xaf,0x04,0x2e,0xff,0x00,0x55,0x00,0x00,0x05,0xbe,0x04,0x06,0x02,
		0x04,0x50,0x1f,0x63,0xce,0xce,0xbf,0xff,0xbf,0xff,0xbf,0xff,0xbe,
		0xc5,0x10,0x02,0xe0,0x04,0x56,0xbe,0x03,0x01,0xde};

static const BYTE iconfldm[38] = {		// 32x32
		0x0a,0x5f,0x00,0x00,0x22,0xff,0x00,0x08,0x78,0x1e,0x80,0x14,0x78,
		0x0b,0x80,0x12,0xfb,0x78,0x0d,0x80,0x10,0x78,0x0f,0xb0,0x0f,0x7c,
		0x1d,0xff,0x7c,0x1f,0x02,0x60,0x60,0x00,0x78,0x1e,0x00,0x7f};

static const BYTE iconfil[178] = {		// 32x32
		0x07,0x5b,0x84,0x00,0x6a,0xff,0x00,0x34,0x1c,0x02,0x00,0x00,0x22,
		0x2f,0xbb,0xc7,0x31,0x23,0x2f,0xbb,0xc6,0xc6,0xc6,0x31,0x20,0x2f,
		0xbb,0x02,0x82,0xff,0x31,0x20,0x2f,0xbe,0x31,0x20,0xef,0xff,0x00,
		0x27,0x62,0x82,0x2f,0xff,0x2f,0xfe,0xff,0x1a,0x8b,0x2f,0xcd,0xdd,
		0x97,0x2f,0xb2,0x4d,0x91,0x27,0x8b,0x18,0x04,0x01,0x04,0xff,0x31,
		0x10,0x2f,0xac,0x02,0x88,0x2e,0x0d,0x2f,0x8a,0x31,0x07,0x8f,0xb5,
		0x84,0x91,0xff,0x2f,0xc7,0xdd,0x11,0x5e,0x0c,0x1b,0x09,0x2f,0xbc,
		0x02,0x88,0x2b,0x07,0xb8,0x8a,0xff,0xf1,0x16,0x8f,0xae,0x01,0x0b,
		0xee,0x8b,0x28,0x83,0x01,0x03,0x2f,0xb8,0x90,0x10,0xff,0xee,0x8d,
		0x2f,0x87,0x0b,0x88,0x2f,0xb5,0x90,0x0a,0xee,0x8d,0x2f,0xc0,0x90,
		0x10,0xff,0xee,0x93,0x32,0x84,0xef,0xb6,0x8f,0x92,0xee,0x98,0x2f,
		0xb7,0x02,0x88,0x25,0x14,0xff,0x0d,0x0b,0x8f,0xb5,0x2c,0x9a,0x76,
		0x17,0xbf,0xad,0x00,0x31,0x2f,0xff,0x2f,0xff,0xfc,0x2f,0xff,0x2f,
		0xfa,0x0d,0x02,0x00,0x44,0x2f,0x94,0x00,0x56};

static const BYTE iconfilm[34] = {		// 32x32
		0x0b,0x0f,0x00,0x00,0x00,0xff,0x00,0x12,0xb0,0x02,0x00,0x08,0xf8,
		0x13,0x55,0xff,0xf8,0x1f,0xff,0xf8,0x1f,0xff,0xf8,0x1f,0xff,0xf8,
		0x1f,0x50,0xff,0xf8,0x1f,0xff,0xfb,0x22};

static const MENURES icon24[6] = {
				{32, 32, icon24a, icon24am}, {32, 32, icon24b, icon24bm},
				{32, 32, icon24c, icon24cm}, {32, 32, icon24d, icon24dm},
				{32, 32, iconfld, iconfldm}, {32, 32, iconfil, iconfilm}};


void menuicon_initialize(void) {

	int		i;

	ZeroMemory(iconreg, sizeof(iconreg));
	ZeroMemory(iconcache, sizeof(iconcache));
	for (i=0; i<6; i++) {
		iconreg[i].res = icon24 + i;
	}
}

void menuicon_deinitialize(void) {

	ICONCACHE	*ic;
	ICONCACHE	*icterm;

	ic = iconcache;
	icterm = ic + MICON_CACHE;
	do {
		vram_destroy(ic->hdl);
	} while(++ic < icterm);
	ZeroMemory(iconcache, sizeof(iconcache));
}

void menuicon_regist(UINT16 id, const MENURES *res) {

	if ((id != 0) && (id < MICON_MAX)) {
		iconreg[id - 1].res = res;
	}
}

VRAMHDL menuicon_lock(UINT16 id, int width, int height, int bpp) {

	ICONCACHE	*icorg;
	ICONCACHE	*ic;
	ICONCACHE	*icterm;
	VRAMHDL		hdl;
const MENURES	*res;
	VRAMHDL		ret;

	if ((id == 0) || (id >= MICON_MAX)) {
		return(NULL);
	}
	icorg = iconcache;
	ic = icorg;
	icterm = icorg + MICON_CACHE;
	do {
		if (ic->id == id) {
			hdl = ic->hdl;
			if ((hdl->width == width) && (hdl->height == height) &&
				(hdl->bpp == bpp)) {
				ic->count++;
				return(hdl);
			}
		}
	} while(++ic < icterm);
	res = (MENURES *)(iconreg[id - 1].res);
	if (res == NULL) {
		return(NULL);
	}
	hdl = menuvram_resload(res, 24);
	ret = vram_resize(hdl, width, height, bpp);
	vram_destroy(hdl);
	if (ret) {
		do {
			ic--;
			if (ic->count == 0) {
				vram_destroy(ic->hdl);
				while(ic > icorg) {
					CopyMemory(ic, ic - 1, sizeof(ICONCACHE));
					ic--;
				}
				ic->id = id;
				ic->count = 1;
				ic->hdl = ret;
				break;
			}
		} while(ic > icorg);
	}
	return(ret);
}

void menuicon_unlock(VRAMHDL vram) {

	ICONCACHE	*ic;
	ICONCACHE	*icterm;

	if (vram) {
		ic = iconcache;
		icterm = ic + MICON_CACHE;
		do {
			if (ic->hdl == vram) {
				ic->count--;
				return;
			}
		} while(++ic < icterm);
		vram_destroy(vram);
	}
}


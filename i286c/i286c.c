#include	"compiler.h"
#include	"i286.h"
#include	"i286c.h"
#include	"v30patch.h"
#include	"memory.h"
#include	"pccore.h"
#include	"iocore.h"
#include	"dmap.h"
#include	"i286c.mcr"


	I286REG		i286reg;

const BYTE iflags[256] = {					// Z_FLAG, S_FLAG, P_FLAG
			0x44, 0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x00,
			0x00, 0x04, 0x04, 0x00, 0x04, 0x00, 0x00, 0x04,
			0x00, 0x04, 0x04, 0x00, 0x04, 0x00, 0x00, 0x04,
			0x04, 0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x00,
			0x00, 0x04, 0x04, 0x00, 0x04, 0x00, 0x00, 0x04,
			0x04, 0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x00,
			0x04, 0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x00,
			0x00, 0x04, 0x04, 0x00, 0x04, 0x00, 0x00, 0x04,
			0x00, 0x04, 0x04, 0x00, 0x04, 0x00, 0x00, 0x04,
			0x04, 0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x00,
			0x04, 0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x00,
			0x00, 0x04, 0x04, 0x00, 0x04, 0x00, 0x00, 0x04,
			0x04, 0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x00,
			0x00, 0x04, 0x04, 0x00, 0x04, 0x00, 0x00, 0x04,
			0x00, 0x04, 0x04, 0x00, 0x04, 0x00, 0x00, 0x04,
			0x04, 0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x00,
			0x80, 0x84, 0x84, 0x80, 0x84, 0x80, 0x80, 0x84,
			0x84, 0x80, 0x80, 0x84, 0x80, 0x84, 0x84, 0x80,
			0x84, 0x80, 0x80, 0x84, 0x80, 0x84, 0x84, 0x80,
			0x80, 0x84, 0x84, 0x80, 0x84, 0x80, 0x80, 0x84,
			0x84, 0x80, 0x80, 0x84, 0x80, 0x84, 0x84, 0x80,
			0x80, 0x84, 0x84, 0x80, 0x84, 0x80, 0x80, 0x84,
			0x80, 0x84, 0x84, 0x80, 0x84, 0x80, 0x80, 0x84,
			0x84, 0x80, 0x80, 0x84, 0x80, 0x84, 0x84, 0x80,
			0x84, 0x80, 0x80, 0x84, 0x80, 0x84, 0x84, 0x80,
			0x80, 0x84, 0x84, 0x80, 0x84, 0x80, 0x80, 0x84,
			0x80, 0x84, 0x84, 0x80, 0x84, 0x80, 0x80, 0x84,
			0x84, 0x80, 0x80, 0x84, 0x80, 0x84, 0x84, 0x80,
			0x80, 0x84, 0x84, 0x80, 0x84, 0x80, 0x80, 0x84,
			0x84, 0x80, 0x80, 0x84, 0x80, 0x84, 0x84, 0x80,
			0x84, 0x80, 0x80, 0x84, 0x80, 0x84, 0x84, 0x80,
			0x80, 0x84, 0x84, 0x80, 0x84, 0x80, 0x80, 0x84};


// ----

	UINT32	EA_FIX;
	BYTE	*reg8_b53[256];
	BYTE	*reg8_b20[256];
	UINT16	*reg16_b53[256];
	UINT16	*reg16_b20[256];
	BYTE	szpcflag[0x200];
	CALCEA	c_calc_ea_dst[256];
	CALCLEA	c_calc_lea[192];
	GETLEA	c_get_ea[192];

#if !defined(CPUW2TEST)
	BYTE	szpflag_w[0x10000];
#endif


static UINT32 ea_nop(void) {

	return(0);
}

void i286_initialize(void) {

	UINT	i;
	UINT	bit;
	BYTE	f;
	int		pos;

	for (i=0; i<0x100; i++) {
		f = P_FLAG;
		for (bit=0x80; bit; bit>>=1) {
			if (i & bit) {
				f ^= P_FLAG;
			}
		}
		if (!(i & 0xff)) {
			f |= Z_FLAG;
		}
		if (i & 0x80) {
			f |= S_FLAG;
		}
		szpcflag[i+0x000] = f;
		szpcflag[i+0x100] = f | C_FLAG;
	}

	for (i=0; i<0x100; i++) {
#if defined(BYTESEX_LITTLE)
		pos = ((i & 0x20)?1:0);
#else
		pos = ((i & 0x20)?0:1);
#endif
		pos += ((i >> 3) & 3) * 2;
		reg8_b53[i] = ((BYTE *)&I286_REG) + pos;
#if defined(BYTESEX_LITTLE)
		pos = ((i & 0x4)?1:0);
#else
		pos = ((i & 0x4)?0:1);
#endif
		pos += (i & 3) * 2;
		reg8_b20[i] = ((BYTE *)&I286_REG) + pos;
		reg16_b53[i] = ((UINT16 *)&I286_REG) + ((i >> 3) & 7);
		reg16_b20[i] = ((UINT16 *)&I286_REG) + (i & 7);
	}

	for (i=0; i<0xc0; i++) {
		pos = ((i >> 3) & 0x18) + (i & 0x07);
		c_calc_ea_dst[i] = i286c_ea_dst_tbl[pos];
		c_calc_lea[i] = i286c_lea_tbl[pos];
		c_get_ea[i] = i286c_ea_tbl[pos];
	}
	for (; i<0x100; i++) {
		c_calc_ea_dst[i] = ea_nop;
	}

#if !defined(CPUW2TEST)
	for (i=0; i<0x10000; i++) {
		f = P_FLAG;
		for (bit=0x80; bit; bit>>=1) {
			if (i & bit) {
				f ^= P_FLAG;
			}
		}
		if (!i) {
			f |= Z_FLAG;
		}
		if (i & 0x8000) {
			f |= S_FLAG;
		}
		szpflag_w[i] = f;
	}
#endif
	v30init();
}

void i286_reset(void) {

	i286_initialize();
	ZeroMemory(&i286reg, sizeof(i286reg));
	I286_CS = 0x1fc0;
	CS_BASE = 0x1fc00;
}

void i286_resetprefetch(void) {
}

void CPUCALL i286_intnum(UINT vect, UINT16 IP) {

const BYTE	*ptr;

	REGPUSH0(REAL_FLAGREG)
	REGPUSH0(I286_CS)
	REGPUSH0(IP)

	I286_FLAG &= ~(T_FLAG | I_FLAG);
	I286_TRAP = 0;

	ptr = I286_MEM + (vect * 4);
	I286_IP = LOADINTELWORD(ptr+0);				// real mode!
	I286_CS = LOADINTELWORD(ptr+2);				// real mode!
	CS_BASE = I286_CS << 4;
	I286_WORKCLOCK(20);
}

void CPUCALL i286_interrupt(BYTE vect) {

	UINT	op;
const BYTE	*ptr;

	op = i286_memoryread(I286_IP + CS_BASE);
	if (op == 0xf4) {							// hlt
		I286_IP++;
	}
	REGPUSH0(REAL_FLAGREG)						// ここV30で辻褄が合わない
	REGPUSH0(I286_CS)
	REGPUSH0(I286_IP)

	I286_FLAG &= ~(T_FLAG | I_FLAG);
	I286_TRAP = 0;

	ptr = I286_MEM + (vect * 4);
	I286_IP = LOADINTELWORD(ptr+0);				// real mode!
	I286_CS = LOADINTELWORD(ptr+2);				// real mode!
	CS_BASE = I286_CS << 4;
	I286_WORKCLOCK(20);
}

void i286(void) {

	UINT	opcode;

	if (I286_TRAP) {
		do {
			GET_PCBYTE(opcode);
			i286op[opcode]();
			if (I286_TRAP) {
				i286_interrupt(1);
			}
			dmap_i286();
		} while(I286_REMCLOCK > 0);
	}
	else if (dmac.working) {
		do {
			GET_PCBYTE(opcode);
			i286op[opcode]();
			dmap_i286();
		} while(I286_REMCLOCK > 0);
	}
	else {
		do {
			GET_PCBYTE(opcode);
			i286op[opcode]();
		} while(I286_REMCLOCK > 0);
	}
}

void i286_step(void) {

	UINT	opcode;

	I286_OV = I286_FLAG & O_FLAG;
	I286_FLAG &= ~(O_FLAG);

	GET_PCBYTE(opcode);
	i286op[opcode]();

	I286_FLAG &= ~(O_FLAG);
	if (I286_OV) {
		I286_FLAG |= (O_FLAG);
	}
	dmap_i286();
}


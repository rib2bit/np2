#include	"compiler.h"
#include	"cpucore.h"
#include	"pccore.h"
#include	"iocore.h"
#include	"pcm86io.h"
#include	"sound.h"
#include	"fmboard.h"


extern	PCM86CFG	pcm86cfg;

static const UINT8 pcm86bits[] = {1, 1, 1, 2, 0, 0, 0, 1};
static const SINT32 pcm86rescue[] = {PCM86_RESCUE * 32, PCM86_RESCUE * 24,
									 PCM86_RESCUE * 16, PCM86_RESCUE * 12,
									 PCM86_RESCUE *  8, PCM86_RESCUE *  6,
									 PCM86_RESCUE *  4, PCM86_RESCUE *  3};

static const UINT8 s_irqtable[8] = {0xff, 0xff, 0xff, 0xff, 0x03, 0x0a, 0x0d, 0x0c};


/*
I/O		A460h
���O	�T�E���h�@�\���ʗpID�t�B�[���h�^OPNA�}�X�N�ݒ�
�Ώ�	�S�@��
�`�b�v	
�@�\
		[WRITE]YM2608(OPNA)�}�X�N�ݒ�
  		bit 7�`2: ���g�p(�ǂݏo�����l��ۑ����邱��)
		bit 1: YM2608(OPNA)�}�X�N�ݒ�
				0= YM2608(OPNA)���}�X�N���Ȃ�
				1= YM2608(OPNA)���}�X�N����
				* 1��ݒ肷���OPNA���؂�������
		bit 0: YM2608(OPNA)�g�������@�\
				0= YM2203(OPN)���������̂ݎg�p����
				1= YM2608(OPNA)�g���������g�p����
	���  o YM2608���ڋ@�ŁAYM2608�̊g�������̃}�X�N��ݒ�^��������

�֘A	I/O 0188h,018Ah
		I/O 018Ch,018Eh
		I/O 0288h,028Ah
		I/O 028Ch,028Eh
*/
static void IOOUTCALL pcm86_oa460(UINT port, REG8 val)
{
	TRACEOUT(("86pcm out %.4x %.2x", port, val));
	g_pcm86.soundflags = (g_pcm86.soundflags & 0xfe) | (val & 1);
	fmboard_extenable((REG8)(val & 1));
	(void)port;
}

/*
I/O		A466h
���O	86�^PCM - FIFO�X�e�[�^�X�^�d�q�{�����[������
		Undocumented
�Ώ�	PC-9801-86,PC-9801-86�^�������ڋ@
�`�b�v	
�@�\
		[WRITE] �d�q�{�����[������
		bit 7�`5: �d�q�{�����[���I��
				000b= VOL1(FM�������ڏo�̓��x��)
				001b= VOL2(FM�����Ԑڏo�̓��x��)
				010b= VOL3(CD-DA(MULTi�̂�),LINE���ڏo�̓��x��)
				011b= VOL4(CD-DA(MULTi�̂�),LINE�Ԑڏo�̓��x��)
				101b= VOL6(PCM���ڏo�̓��x��)

		bit 3�`0: �{�����[���ݒ�l
				0000b= ���ʍő�
				  :
				1111b= ���ʍŏ�
	���  o �e�d�q�{�����[���̃A�b�e�l�[�g�l��ݒ肷��B
	
�֘A	I/O A46Ch
*/
static void IOOUTCALL pcm86_oa466(UINT port, REG8 val) {

	TRACEOUT(("86pcm out %.4x %.2x", port, val));
	if ((val & 0xe0) == 0xa0) {
		sound_sync();
		g_pcm86.vol5 = (~val) & 15;
		g_pcm86.volume = pcm86cfg.vol * g_pcm86.vol5;
	}
	(void)port;
}

/*
I/O		A468h
���O	86�^PCM - FIFO����
		Undocumented
�Ώ�	PC-9801-86,PC-9801-86�^�������ڋ@
�`�b�v	
�@�\
		[READ/WRITE]
		bit 7: FIFO����ݒ�
				1= FIFO����
				0= FIFO��~
		bit 6: FIFO�������[�h
				1= FIFO��CPU(�^�����[�h)
				0= CPU��FIFO(�Đ����[�h)
		bit 5: FIFO���荞�݋��t���O
				1= FIFO���荞�݋���
				0= FIFO���荞�݋֎~
		bit 4: FIFO���荞�ݗv���t���O
				1= FIFO���荞�ݗv������
				0= FIFO���荞�ݗv���Ȃ�
				* 0���������ނ�FIFO���荞�ݗv�����N���A�����
		bit 3: FIFO�������t���O
				1= FIFO���Z�b�g��ݒ�
				0= FIFO���Z�b�g��ݒ肵�Ȃ�(�ʏ�)
				* 1���������񂾂�A0�ɖ߂��K�v������
		bit 2�`0: �W�{�����g�����[�g
				111b=  4.13kHz
				110b=  5.52kHz
				101b=  8.27kHz
				100b= 11.03kHz
				011b= 16.54kHz
				010b= 22.05kHz
				001b= 33.08kHz
				000b= 44.10kHz
���  o PCM FIFO�𐧌䂷��B
�֘A	I/O A46Ah
		I/O A46Ch
*/
static void IOOUTCALL pcm86_oa468(UINT port, REG8 val) {

	REG8	xchgbit;

	TRACEOUT(("86pcm out %.4x %.2x", port, val));
	sound_sync();
	xchgbit = g_pcm86.fifo ^ val;
	// �o�b�t�@���Z�b�g����
	if ((xchgbit & 8) && (val & 8)) {
		g_pcm86.readpos = 0;				// �o�b�t�@���Z�b�g
		g_pcm86.wrtpos = 0;
		g_pcm86.realbuf = 0;
		g_pcm86.virbuf = 0;
		g_pcm86.lastclock = CPU_CLOCK + CPU_BASECLOCK - CPU_REMCLOCK;
		g_pcm86.lastclock <<= 6;
	}
	if ((xchgbit & 0x10) && (!(val & 0x10))) {
		g_pcm86.irqflag = 0;
//		g_pcm86.write = 0;
//		g_pcm86.reqirq = 0;
	}
	// �T���v�����O���[�g�ύX
	if (xchgbit & 7) {
		g_pcm86.rescue = pcm86rescue[val & 7] << g_pcm86.stepbit;
		pcm86_setpcmrate(val);
	}
#if 1	// ����d��ȃo�O....
	g_pcm86.fifo = val;
#else
	g_pcm86.fifo = val & (~0x10);
#endif
	if ((xchgbit & 0x80) && (val & 0x80)) {
		g_pcm86.lastclock = CPU_CLOCK + CPU_BASECLOCK - CPU_REMCLOCK;
		g_pcm86.lastclock <<= 6;
	}
	pcm86_setnextintr();
	(void)port;
}

/*
I/O		A46Ah
���O	86�^PCM - D/A�R���o�[�^����^FIFO���荞�݃^�C�~���O�ݒ�
		Undocumented
�Ώ�	PC-9801-86,PC-9801-86�^�������ڋ@
�`�b�v	
�@�\
		[READ/WRITE] D/A�R���o�[�^���� ��I/O A468h bit 5��0�̂Ƃ�
		bit 7: PCM�N���b�N
				* �ʏ�1�ɐݒ肷��

		bit 6: PCM�ʎq���r�b�g��
				1=  8�r�b�g
				0= 16�r�b�g
		bit 5,4: PCM�o�̓C�l�[�u��
				11b= L/R-ch�Ƃ��o��(�X�e���I)
				10b= L-ch�̂ݏo��
				01b= R-ch�̂ݏo��
				00b= PCM�o�͂��Ȃ�
		bit 3: ���g�p(��ɂ�0)
		bit 2�`0: PCM���[�h
				* PCM��^�Ă���Ƃ���010b�ɐݒ肷��B
				  AVSDRV�̃t�@���N�V����0Dh,0Eh��PCM���[�h�Őݒ肳���B
				  ���[�h0�̂Ƃ�101b�A���[�h1,6,7�̂Ƃ�010b�B
	���  o ���[�h���́AI/O A468h bit 5�̏�ԂɊւ炸D/A�R���o�[�^�����
			�ݒ�l���ǂ߂�B

		[WRITE]	FIFO���荞�݃^�C�~���O�ݒ� ��I/O A468h bit 5��1�̂Ƃ�
		bit 7�`0: FIFO���荞�݃^�C�~���O�̐ݒ�l
	���  o FIFO�̎c��o�C�g����[FIFO���荞�݃^�C�~���O�̐ݒ�l]*128
		�o�C�g�ɂȂ����Ƃ��A���荞�݂���������B
�֘A	I/O A468h
*/
static void IOOUTCALL pcm86_oa46a(UINT port, REG8 val) {

	TRACEOUT(("86pcm out %.4x %.2x", port, val));
	sound_sync();
	if (g_pcm86.fifo & 0x20) {
#if 1
		if (val != 0xff) {
			g_pcm86.fifosize = (UINT16)((val + 1) << 7);
		}
		else {
			g_pcm86.fifosize = 0x7ffc;
		}
#else
		if (!val) {
			val++;
		}
		g_pcm86.fifosize = (WORD)(val) << 7;
#endif
	}
	else {
		g_pcm86.dactrl = val;
		g_pcm86.stepbit = pcm86bits[(val >> 4) & 7];
		g_pcm86.stepmask = (1 << g_pcm86.stepbit) - 1;
		g_pcm86.rescue = pcm86rescue[g_pcm86.fifo & 7] << g_pcm86.stepbit;
	}
	pcm86_setnextintr();
	(void)port;
}

/*
I/O		A46Ch
���O	86�^PCM - FIFO���o��
		Undocumented
�Ώ�	PC-9801-86,PC-9801-86�^�������ڋ@
�`�b�v	
�@�\
		[READ/WRITE]
		bit 7�`0: FIFO�ƃf�[�^�̓��o�͂��s���B
���  o PCM FIFO�Ƃ̓��o�͂��s���BFIFO�������̃T�C�Y��32K�o�C�g�B
�֘A	I/O A466h bit 7�`5
		I/O A468h bit 6
*/
static void IOOUTCALL pcm86_oa46c(UINT port, REG8 val) {

	TRACEOUT(("86pcm out %.4x %.2x", port, val));
#if 1
	if (g_pcm86.virbuf < PCM86_LOGICALBUF) {
		g_pcm86.virbuf++;
	}
	g_pcm86.buffer[g_pcm86.wrtpos] = val;
	g_pcm86.wrtpos = (g_pcm86.wrtpos + 1) & PCM86_BUFMSK;
	g_pcm86.realbuf++;
	// �o�b�t�@�I�[�o�[�t���[�̊Ď�
	if (g_pcm86.realbuf >= PCM86_REALBUFSIZE) {
#if 1
		g_pcm86.realbuf -= 4;
		g_pcm86.readpos = (g_pcm86.readpos + 4) & PCM86_BUFMSK;
#else
		g_pcm86.realbuf &= 3;				// align4���߃E�`
		g_pcm86.realbuf += PCM86_REALBUFSIZE - 4;
#endif
	}
	g_pcm86.write = 1;	/*	�R�����g�O��(Kai1)	*/
	g_pcm86.reqirq = 1;
#else
	if (g_pcm86.virbuf < PCM86_LOGICALBUF) {
		g_pcm86.virbuf++;
		g_pcm86.buffer[g_pcm86.wrtpos] = val;
		g_pcm86.wrtpos = (g_pcm86.wrtpos + 1) & PCM86_BUFMSK;
		g_pcm86.realbuf++;
		// �o�b�t�@�I�[�o�[�t���[�̊Ď�
		if (g_pcm86.realbuf >= PCM86_REALBUFSIZE) {
			g_pcm86.realbuf &= 3;				// align4���߃E�`
			g_pcm86.realbuf += PCM86_REALBUFSIZE - 4;
		}
//		g_pcm86.write = 1;
		g_pcm86.reqirq = 1;
	}
#endif
	(void)port;
}

/*
I/O		A460h
���O	�T�E���h�@�\���ʗpID�t�B�[���h�^OPNA�}�X�N�ݒ�
�Ώ�	�S�@��
�`�b�v	
�@�\
	��[READ]�T�E���h�@�\���ʗpID�ǂ݂���
	  	bit 7�`4: �T�E���h�@�\���ʗpID
				0000b= PC-98DO+��������
				0001b= PC-98GS��������
				0010b= PC-9801-73(I/O�|�[�g018xh��)
				0011b= PC-9801-73�76(I/O�|�[�g028xh��)
				0100b= PC-9821����Ap�As�Ae�Af�Ap2�As2�An�Ap3�As3�Ce�Cs2�Ce2
				       ��������,PC-9801-86(I/O�|�[�g018xh��)
				0101b= PC-9801-86(I/O�|�[�g028xh��)
				0110b= PC-9821Nf�Np��������
				0111b= PC-9821Xt�Xa�Xf�Xn�Xp�Xs�Xa10�Xa9�Xa7�Xt13�Xe10/C4�
				              Xa12�Xa7e��������,PC-9821XE10-B??
				1000b= PC-9821Cf�Cx�Cb�Cx2�Cb2�Cx3�Cb3�Na7�Nx��������
				1111b= �����@�\�Ȃ��A�܂���PC-9801-26�����@�\����
		bit 3,2: ���g�p
		bit 1: YM2608(OPNA)�}�X�N�ݒ�
				1= YM2608(OPNA)���}�X�N���Ă���
				0= YM2608(OPNA)���}�X�N���Ă��Ȃ�(���Z�b�g���)
		bit 1: YMF278(OPL3)�����@�\��[PC-9821Cx3�Cb3�Na12�Na9,PC-9801-118]
				1= 
				0= 
		bit 0: YM2608(OPNA)�g�������@�\
				1= YM2608(OPNA)�g���������g�p����
				0= YM2203(OPN)���������̂ݎg�p����(���Z�b�g���)
	���  o �T�E���h�@�\ID��ǂݏo���B
	      o PC-9801-26�͂��̃|�[�g���T�|�[�g���Ă��Ȃ��̂ŁA�����̗L����
			���̃|�[�g���画�f���邱�Ƃ͂ł��Ȃ��B

�֘A	I/O 0188h,018Ah
		I/O 018Ch,018Eh
		I/O 0288h,028Ah
		I/O 028Ch,028Eh
*/
static REG8 IOINPCALL pcm86_ia460(UINT port)
{
	(void)port;
	return g_pcm86.soundflags;
}

/*
I/O		A466h
���O	86�^PCM - FIFO�X�e�[�^�X�^�d�q�{�����[������
		Undocumented
�Ώ�	PC-9801-86,PC-9801-86�^�������ڋ@
�`�b�v	
�@�\
		[READ] FIFO�X�e�[�^�X
		bit 7: FIFO�t��
				1= FIFO�͖��t(FIFO���̃f�[�^��32K�o�C�g���傤�ǂ̂Ƃ�)
				0= FIFO�͖��t�łȂ�
		bit 6: FIFO�G���v�e�B
				1= FIFO�͋�
				0= FIFO�͋�łȂ�
		bit 5: FIFO�I�[�o�[�t���[(�^�����̂�)
				1= FIFO���I�[�o�[�t���[����
				0= FIFO�̓I�[�o�[�t���[���Ă��Ȃ�
		bit 4�`1: ���g�p
		bit 0: L/R�N���b�N
				* I/O A468h�Őݒ肵���T���v�����O���[�g�ɓ�������
				  ���]���J��Ԃ�
	���  o PCM FIFO�̃X�e�[�^�X��ǂݏo���B
	
�֘A	I/O A46Ch
*/
static REG8 IOINPCALL pcm86_ia466(UINT port) {

	UINT32	past;
	UINT32	cnt;
	UINT32	stepclock;
	REG8	ret;

	past = CPU_CLOCK + CPU_BASECLOCK - CPU_REMCLOCK;
	past <<= 6;
	past -= g_pcm86.lastclock;
	stepclock = g_pcm86.stepclock;
	if (past >= stepclock) {
		cnt = past / stepclock;
		g_pcm86.lastclock += (cnt * stepclock);
		past -= cnt * stepclock;
		if (g_pcm86.fifo & 0x80) {
			sound_sync();
			RECALC_NOWCLKWAIT(cnt);
		}
	}
	ret = ((past << 1) >= stepclock)?1:0;
	if (g_pcm86.virbuf >= PCM86_LOGICALBUF) {			// �o�b�t�@�t��
		ret |= 0x80;
	}
	else if (!g_pcm86.virbuf) {						// �o�b�t�@�O
		ret |= 0x40;								// ���ƕρc
	}
	(void)port;
	TRACEOUT(("86pcm in %.4x %.2x", port, ret));
	return(ret);
}

/*
I/O		A468h
���O	86�^PCM - FIFO����
		Undocumented
�Ώ�	PC-9801-86,PC-9801-86�^�������ڋ@
�`�b�v	
�@�\
		[READ/WRITE]
		bit 7: FIFO����ݒ�
				1= FIFO����
				0= FIFO��~
		bit 6: FIFO�������[�h
				1= FIFO��CPU(�^�����[�h)
				0= CPU��FIFO(�Đ����[�h)
		bit 5: FIFO���荞�݋��t���O
				1= FIFO���荞�݋���
				0= FIFO���荞�݋֎~
		bit 4: FIFO���荞�ݗv���t���O
				1= FIFO���荞�ݗv������
				0= FIFO���荞�ݗv���Ȃ�
				* 0���������ނ�FIFO���荞�ݗv�����N���A�����
		bit 3: FIFO�������t���O
				1= FIFO���Z�b�g��ݒ�
				0= FIFO���Z�b�g��ݒ肵�Ȃ�(�ʏ�)
				* 1���������񂾂�A0�ɖ߂��K�v������
		bit 2�`0: �W�{�����g�����[�g
				111b=  4.13kHz
				110b=  5.52kHz
				101b=  8.27kHz
				100b= 11.03kHz
				011b= 16.54kHz
				010b= 22.05kHz
				001b= 33.08kHz
				000b= 44.10kHz
���  o PCM FIFO�𐧌䂷��B
�֘A	I/O A46Ah
		I/O A46Ch
*/
static REG8 IOINPCALL pcm86_ia468(UINT port) {

	REG8	ret;

	ret = g_pcm86.fifo & (~0x10);
/*	���L�̂悤�ɕύX�����M&M DoX(21��)�A�����̌���������ł�PCM�Đ������P�����	*/
#if 0	/*	#if 1 -> #if 0	�C��(Kai1)	*/
	if (pcm86gen_intrq()) {
		ret |= 0x10;
	}
#elif 1		// �ނ��낱���H
	if (g_pcm86.fifo & 0x20) {
		sound_sync();
#if 0
		if (g_pcm86.virbuf <= g_pcm86.fifosize) {
			if (g_pcm86.write) {
				g_pcm86.write = 0;
			}
			else {
				ret |= 0x10;
			}
		}
#endif
		/*	fmgen�ł�86PCM�������Ă���Ȋ����H	*/
		if (g_pcm86.write) {
			if (g_pcm86.virbuf)
				g_pcm86.write = 0;
		} else if (g_pcm86.virbuf <= g_pcm86.fifosize) {
			ret |= 0x10;
		}
	}
#else
	if ((g_pcm86.write) && (g_pcm86.fifo & 0x20)) {
//		g_pcm86.write = 0;
		sound_sync();
		if (g_pcm86.virbuf <= g_pcm86.fifosize) {
			g_pcm86.write = 0;
			ret |= 0x10;
		}
	}
#endif
	(void)port;
//	TRACEOUT(("86pcm in %.4x %.2x", port, ret));
	TRACEOUT(("86pcm in %.4x %.2x - [VBUF[%d]:RBUF[%d]:FIFO[%d]]", port, ret, g_pcm86.virbuf, g_pcm86.realbuf, g_pcm86.fifosize));
	return(ret);
}

/*
I/O		A46Ah
���O	86�^PCM - D/A�R���o�[�^����^FIFO���荞�݃^�C�~���O�ݒ�
		Undocumented
�Ώ�	PC-9801-86,PC-9801-86�^�������ڋ@
�`�b�v	
�@�\
		[READ/WRITE] D/A�R���o�[�^���� ��I/O A468h bit 5��0�̂Ƃ�
		bit 7: PCM�N���b�N
				* �ʏ�1�ɐݒ肷��

		bit 6: PCM�ʎq���r�b�g��
				1=  8�r�b�g
				0= 16�r�b�g
		bit 5,4: PCM�o�̓C�l�[�u��
				11b= L/R-ch�Ƃ��o��(�X�e���I)
				10b= L-ch�̂ݏo��
				01b= R-ch�̂ݏo��
				00b= PCM�o�͂��Ȃ�
		bit 3: ���g�p(��ɂ�0)
		bit 2�`0: PCM���[�h
				* PCM��^�Ă���Ƃ���010b�ɐݒ肷��B
				  AVSDRV�̃t�@���N�V����0Dh,0Eh��PCM���[�h�Őݒ肳���B
				  ���[�h0�̂Ƃ�101b�A���[�h1,6,7�̂Ƃ�010b�B
	���  o ���[�h���́AI/O A468h bit 5�̏�ԂɊւ炸D/A�R���o�[�^�����
			�ݒ�l���ǂ߂�B

�֘A	I/O A468h
*/
static REG8 IOINPCALL pcm86_ia46a(UINT port) {

	(void)port;
	TRACEOUT(("86pcm in %.4x %.2x", port, g_pcm86.dactrl));
	return(g_pcm86.dactrl);
}

static REG8 IOINPCALL pcm86_inpdummy(UINT port) {

	(void)port;
	return(0);
}


// ----

/**
 * Reset
 * @param[in] cDipSw Dip switch
 */
void pcm86io_setopt(REG8 cDipSw)
{
	g_pcm86.soundflags = ((~cDipSw) >> 1) & 0x70;
	g_pcm86.irq = s_irqtable[(cDipSw >> 2) & 7];
}

void pcm86io_bind(void) {

	sound_streamregist(&g_pcm86, (SOUNDCB)pcm86gen_getpcm);

	iocore_attachout(0xa460, pcm86_oa460);
	iocore_attachout(0xa466, pcm86_oa466);
	iocore_attachout(0xa468, pcm86_oa468);
	iocore_attachout(0xa46a, pcm86_oa46a);
	iocore_attachout(0xa46c, pcm86_oa46c);

	iocore_attachinp(0xa460, pcm86_ia460);
	iocore_attachinp(0xa462, pcm86_inpdummy);
	iocore_attachinp(0xa464, pcm86_inpdummy);
	iocore_attachinp(0xa466, pcm86_ia466);
	iocore_attachinp(0xa468, pcm86_ia468);
	iocore_attachinp(0xa46a, pcm86_ia46a);
	iocore_attachinp(0xa46c, pcm86_inpdummy);
	iocore_attachinp(0xa46e, pcm86_inpdummy);
}


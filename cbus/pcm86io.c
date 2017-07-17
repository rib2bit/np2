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
名前	サウンド機能識別用IDフィールド／OPNAマスク設定
対象	全機種
チップ	
機能
		[WRITE]YM2608(OPNA)マスク設定
  		bit 7～2: 未使用(読み出した値を保存すること)
		bit 1: YM2608(OPNA)マスク設定
				0= YM2608(OPNA)をマスクしない
				1= YM2608(OPNA)をマスクする
				* 1を設定するとOPNAが切り放される
		bit 0: YM2608(OPNA)拡張部分機能
				0= YM2203(OPN)相当部分のみ使用する
				1= YM2608(OPNA)拡張部分も使用する
	解説  o YM2608搭載機で、YM2608の拡張部分のマスクを設定／解除する

関連	I/O 0188h,018Ah
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
名前	86型PCM - FIFOステータス／電子ボリューム制御
		Undocumented
対象	PC-9801-86,PC-9801-86型音源搭載機
チップ	
機能
		[WRITE] 電子ボリューム制御
		bit 7～5: 電子ボリューム選択
				000b= VOL1(FM音源直接出力レベル)
				001b= VOL2(FM音源間接出力レベル)
				010b= VOL3(CD-DA(MULTiのみ),LINE直接出力レベル)
				011b= VOL4(CD-DA(MULTiのみ),LINE間接出力レベル)
				101b= VOL6(PCM直接出力レベル)

		bit 3～0: ボリューム設定値
				0000b= 音量最大
				  :
				1111b= 音量最小
	解説  o 各電子ボリュームのアッテネート値を設定する。
	
関連	I/O A46Ch
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
名前	86型PCM - FIFO制御
		Undocumented
対象	PC-9801-86,PC-9801-86型音源搭載機
チップ	
機能
		[READ/WRITE]
		bit 7: FIFO動作設定
				1= FIFO動作
				0= FIFO停止
		bit 6: FIFO方向モード
				1= FIFO→CPU(録音モード)
				0= CPU→FIFO(再生モード)
		bit 5: FIFO割り込み許可フラグ
				1= FIFO割り込み許可
				0= FIFO割り込み禁止
		bit 4: FIFO割り込み要求フラグ
				1= FIFO割り込み要求あり
				0= FIFO割り込み要求なし
				* 0を書き込むとFIFO割り込み要求がクリアされる
		bit 3: FIFO初期化フラグ
				1= FIFOリセットを設定
				0= FIFOリセットを設定しない(通常)
				* 1を書き込んだら、0に戻す必要がある
		bit 2～0: 標本化周波数レート
				111b=  4.13kHz
				110b=  5.52kHz
				101b=  8.27kHz
				100b= 11.03kHz
				011b= 16.54kHz
				010b= 22.05kHz
				001b= 33.08kHz
				000b= 44.10kHz
解説  o PCM FIFOを制御する。
関連	I/O A46Ah
		I/O A46Ch
*/
static void IOOUTCALL pcm86_oa468(UINT port, REG8 val) {

	REG8	xchgbit;

	TRACEOUT(("86pcm out %.4x %.2x", port, val));
	sound_sync();
	xchgbit = g_pcm86.fifo ^ val;
	// バッファリセット判定
	if ((xchgbit & 8) && (val & 8)) {
		g_pcm86.readpos = 0;				// バッファリセット
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
	// サンプリングレート変更
	if (xchgbit & 7) {
		g_pcm86.rescue = pcm86rescue[val & 7] << g_pcm86.stepbit;
		pcm86_setpcmrate(val);
	}
#if 1	// これ重大なバグ....
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
名前	86型PCM - D/Aコンバータ制御／FIFO割り込みタイミング設定
		Undocumented
対象	PC-9801-86,PC-9801-86型音源搭載機
チップ	
機能
		[READ/WRITE] D/Aコンバータ制御 ■I/O A468h bit 5が0のとき
		bit 7: PCMクロック
				* 通常1に設定する

		bit 6: PCM量子化ビット数
				1=  8ビット
				0= 16ビット
		bit 5,4: PCM出力イネーブル
				11b= L/R-chとも出力(ステレオ)
				10b= L-chのみ出力
				01b= R-chのみ出力
				00b= PCM出力しない
		bit 3: 未使用(常にに0)
		bit 2～0: PCMモード
				* PCMを録再するときは010bに設定する。
				  AVSDRVのファンクション0Dh,0EhのPCMモードで設定される。
				  モード0のとき101b、モード1,6,7のとき010b。
	解説  o リード時は、I/O A468h bit 5の状態に関らずD/Aコンバータ制御の
			設定値が読める。

		[WRITE]	FIFO割り込みタイミング設定 ■I/O A468h bit 5が1のとき
		bit 7～0: FIFO割り込みタイミングの設定値
	解説  o FIFOの残りバイト数が[FIFO割り込みタイミングの設定値]*128
		バイトになったとき、割り込みが発生する。
関連	I/O A468h
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
名前	86型PCM - FIFO入出力
		Undocumented
対象	PC-9801-86,PC-9801-86型音源搭載機
チップ	
機能
		[READ/WRITE]
		bit 7～0: FIFOとデータの入出力を行う。
解説  o PCM FIFOとの入出力を行う。FIFOメモリのサイズは32Kバイト。
関連	I/O A466h bit 7～5
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
	// バッファオーバーフローの監視
	if (g_pcm86.realbuf >= PCM86_REALBUFSIZE) {
#if 1
		g_pcm86.realbuf -= 4;
		g_pcm86.readpos = (g_pcm86.readpos + 4) & PCM86_BUFMSK;
#else
		g_pcm86.realbuf &= 3;				// align4決めウチ
		g_pcm86.realbuf += PCM86_REALBUFSIZE - 4;
#endif
	}
	g_pcm86.write = 1;	/*	コメント外し(Kai1)	*/
	g_pcm86.reqirq = 1;
#else
	if (g_pcm86.virbuf < PCM86_LOGICALBUF) {
		g_pcm86.virbuf++;
		g_pcm86.buffer[g_pcm86.wrtpos] = val;
		g_pcm86.wrtpos = (g_pcm86.wrtpos + 1) & PCM86_BUFMSK;
		g_pcm86.realbuf++;
		// バッファオーバーフローの監視
		if (g_pcm86.realbuf >= PCM86_REALBUFSIZE) {
			g_pcm86.realbuf &= 3;				// align4決めウチ
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
名前	サウンド機能識別用IDフィールド／OPNAマスク設定
対象	全機種
チップ	
機能
	●[READ]サウンド機能識別用ID読みだし
	  	bit 7～4: サウンド機能識別用ID
				0000b= PC-98DO+内蔵音源
				0001b= PC-98GS内蔵音源
				0010b= PC-9801-73(I/Oポート018xh時)
				0011b= PC-9801-73･76(I/Oポート028xh時)
				0100b= PC-9821初代･Ap･As･Ae･Af･Ap2･As2･An･Ap3･As3･Ce･Cs2･Ce2
				       内蔵音源,PC-9801-86(I/Oポート018xh時)
				0101b= PC-9801-86(I/Oポート028xh時)
				0110b= PC-9821Nf･Np内蔵音源
				0111b= PC-9821Xt･Xa･Xf･Xn･Xp･Xs･Xa10･Xa9･Xa7･Xt13･Xe10/C4･
				              Xa12･Xa7e内蔵音源,PC-9821XE10-B??
				1000b= PC-9821Cf･Cx･Cb･Cx2･Cb2･Cx3･Cb3･Na7･Nx内蔵音源
				1111b= 音源機能なし、またはPC-9801-26相当機能あり
		bit 3,2: 未使用
		bit 1: YM2608(OPNA)マスク設定
				1= YM2608(OPNA)をマスクしている
				0= YM2608(OPNA)をマスクしていない(リセット状態)
		bit 1: YMF278(OPL3)部分機能■[PC-9821Cx3･Cb3･Na12･Na9,PC-9801-118]
				1= 
				0= 
		bit 0: YM2608(OPNA)拡張部分機能
				1= YM2608(OPNA)拡張部分も使用する
				0= YM2203(OPN)相当部分のみ使用する(リセット状態)
	解説  o サウンド機能IDを読み出す。
	      o PC-9801-26はこのポートをサポートしていないので、実装の有無を
			このポートから判断することはできない。

関連	I/O 0188h,018Ah
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
名前	86型PCM - FIFOステータス／電子ボリューム制御
		Undocumented
対象	PC-9801-86,PC-9801-86型音源搭載機
チップ	
機能
		[READ] FIFOステータス
		bit 7: FIFOフル
				1= FIFOは満杯(FIFO内のデータが32Kバイトちょうどのとき)
				0= FIFOは満杯でない
		bit 6: FIFOエンプティ
				1= FIFOは空
				0= FIFOは空でない
		bit 5: FIFOオーバーフロー(録音時のみ)
				1= FIFOがオーバーフローした
				0= FIFOはオーバーフローしていない
		bit 4～1: 未使用
		bit 0: L/Rクロック
				* I/O A468hで設定したサンプリングレートに同期して
				  反転を繰り返す
	解説  o PCM FIFOのステータスを読み出す。
	
関連	I/O A46Ch
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
	if (g_pcm86.virbuf >= PCM86_LOGICALBUF) {			// バッファフル
		ret |= 0x80;
	}
	else if (!g_pcm86.virbuf) {						// バッファ０
		ret |= 0x40;								// ちと変…
	}
	(void)port;
	TRACEOUT(("86pcm in %.4x %.2x", port, ret));
	return(ret);
}

/*
I/O		A468h
名前	86型PCM - FIFO制御
		Undocumented
対象	PC-9801-86,PC-9801-86型音源搭載機
チップ	
機能
		[READ/WRITE]
		bit 7: FIFO動作設定
				1= FIFO動作
				0= FIFO停止
		bit 6: FIFO方向モード
				1= FIFO→CPU(録音モード)
				0= CPU→FIFO(再生モード)
		bit 5: FIFO割り込み許可フラグ
				1= FIFO割り込み許可
				0= FIFO割り込み禁止
		bit 4: FIFO割り込み要求フラグ
				1= FIFO割り込み要求あり
				0= FIFO割り込み要求なし
				* 0を書き込むとFIFO割り込み要求がクリアされる
		bit 3: FIFO初期化フラグ
				1= FIFOリセットを設定
				0= FIFOリセットを設定しない(通常)
				* 1を書き込んだら、0に戻す必要がある
		bit 2～0: 標本化周波数レート
				111b=  4.13kHz
				110b=  5.52kHz
				101b=  8.27kHz
				100b= 11.03kHz
				011b= 16.54kHz
				010b= 22.05kHz
				001b= 33.08kHz
				000b= 44.10kHz
解説  o PCM FIFOを制御する。
関連	I/O A46Ah
		I/O A46Ch
*/
static REG8 IOINPCALL pcm86_ia468(UINT port) {

	REG8	ret;

	ret = g_pcm86.fifo & (~0x10);
/*	下記のように変更するとM&M DoX(21版)、やんやんの激闘同窓会等でのPCM再生が改善される	*/
#if 0	/*	#if 1 -> #if 0	修正(Kai1)	*/
	if (pcm86gen_intrq()) {
		ret |= 0x10;
	}
#elif 1		// むしろこう？
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
		/*	fmgen版の86PCM調整ってこんな感じ？	*/
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
名前	86型PCM - D/Aコンバータ制御／FIFO割り込みタイミング設定
		Undocumented
対象	PC-9801-86,PC-9801-86型音源搭載機
チップ	
機能
		[READ/WRITE] D/Aコンバータ制御 ■I/O A468h bit 5が0のとき
		bit 7: PCMクロック
				* 通常1に設定する

		bit 6: PCM量子化ビット数
				1=  8ビット
				0= 16ビット
		bit 5,4: PCM出力イネーブル
				11b= L/R-chとも出力(ステレオ)
				10b= L-chのみ出力
				01b= R-chのみ出力
				00b= PCM出力しない
		bit 3: 未使用(常にに0)
		bit 2～0: PCMモード
				* PCMを録再するときは010bに設定する。
				  AVSDRVのファンクション0Dh,0EhのPCMモードで設定される。
				  モード0のとき101b、モード1,6,7のとき010b。
	解説  o リード時は、I/O A468h bit 5の状態に関らずD/Aコンバータ制御の
			設定値が読める。

関連	I/O A468h
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


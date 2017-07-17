#include	"compiler.h"
#include	"dosio.h"
#include	"pccore.h"
#include	"iocore.h"

#include	"DiskImage/fddfile.h"
#include	"DiskImage/FD/fdd_mahalito.h"

static const UINT8 mhlt_FileID_2HD[10]	= {'v','e','r',' ','1','.','1','0',' ',' '};
static const UINT8 mhlt_FileID_2DD[10]	= {'2','D','D',' ','v','e','r','1','.','0'};
static const UINT8 mhlt_FileID_2D[10]	= {'2','D',' ',' ','v','e','r','1','.','0'};

static const OEMCHAR str_dat[]		= OEMTEXT(".dat");

BRESULT fdd_set_mhlt(FDDFILE fdd, FDDFUNC fdd_fn, const OEMCHAR *fname, int ro) {

	short		attr;
	FILEH		fh;
	UINT		rsize;
	UINT32		ptr;
	UINT		i;
	UINT		j;
	BOOL		bValidID;
	MHLT_INFTRK	trk;
	MHLT_SEC	sec;
	UINT8		secMax;

	attr = file_attr(fname);
	if (attr & 0x18) {
		return(FAILURE);
	}
	fh = file_open_rb(fname);
	if (fh == FILEH_INVALID) {
		return(FAILURE);
	}
	rsize = file_read(fh, &fdd->inf.mhlt.head, MHLT_HEADERSIZE);
	file_close(fh);
	if (rsize != MHLT_HEADERSIZE) {
		return(FAILURE);
	}

	/*	識別IDチェック	*/
	bValidID = FALSE;
	if (memcmp(fdd->inf.mhlt.head.verID, mhlt_FileID_2HD, 10) == 0) {
		/*	2HD	*/
		bValidID = TRUE;
		fdd->inf.xdf.disktype = DISKTYPE_2HD;
	}
	else if (memcmp(fdd->inf.mhlt.head.verID, mhlt_FileID_2DD, 10) == 0) {
		/*	2DD	*/
		bValidID = TRUE;
		fdd->inf.xdf.disktype = DISKTYPE_2DD;
	}
	else if (memcmp(fdd->inf.mhlt.head.verID, mhlt_FileID_2D, 10) == 0) {
		/*	2D	*/
		bValidID = TRUE;
		fdd->inf.xdf.disktype = DISKTYPE_2D;
	}

	if ( bValidID == TRUE )
	{
		fdd->type = DISKTYPE_MAHALITO;
		fdd->protect = ((attr & 0x01) || (ro)) ? TRUE : FALSE;

		fdd->inf.xdf.rpm = 0;
		fdd->inf.xdf.tracks		= fdd->inf.mhlt.head.cly * 2;
		//fdd->inf.xdf.sectors	= NFD_SECMAX;

		/*	トラック情報用のメモリ確保	*/
		fdd->inf.mhlt.trks = _MALLOC(sizeof(_MHLT_INFTRK) * fdd->inf.xdf.tracks, "MAHALITO_head[T]");
		if ( fdd->inf.mhlt.trks == NULL ) {
			TRACEOUT(("fdd_set_mhlt FAILURE can't alloc memory for header[MHLT_INFTRK]"));
			return(FAILURE);
		}

		fh = file_open_rb(fname);
		if (fh == FILEH_INVALID) {
			return(FAILURE);
		}
		if (file_seek(fh, MHLT_HEADERSIZE, FSEEK_SET) != MHLT_HEADERSIZE) {
			file_close(fh);
			return(FAILURE);
		}
		/*	トラック情報、セクタ情報の読込	*/
		secMax = 0;
		for (i = 0; i < fdd->inf.xdf.tracks; i++) {
			/*	トラック情報読込	*/
			if (file_read(fh, &fdd->inf.mhlt.trks[i].trk, sizeof(_MHLT_TRK)) != sizeof(_MHLT_TRK)) {
				file_close(fh);
				return(FAILURE);
			}

			/*	セクタ情報用のメモリ確保	*/
			fdd->inf.mhlt.trks[i].ids = _MALLOC(sizeof(_MHLT_SEC) * fdd->inf.mhlt.trks[i].trk.sectors, "MAHALITO_head[S]");
			if ( fdd->inf.mhlt.trks[i].ids == NULL ) {
				TRACEOUT(("fdd_set_mhlt FAILURE can't alloc memory for header[MHLT_SEC]"));
				return(FAILURE);
			}
			fdd->inf.mhlt.trks[i].ptr = _MALLOC(sizeof(UINT32) * fdd->inf.mhlt.trks[i].trk.sectors, "MAHALITO_head[ptr]");
			if ( fdd->inf.mhlt.trks[i].ids == NULL ) {
				TRACEOUT(("fdd_set_mhlt FAILURE can't alloc memory for header[ptr]"));
				return(FAILURE);
			}

			if (fdd->inf.mhlt.trks[i].trk.sectors >= secMax) {
				secMax = fdd->inf.mhlt.trks[i].trk.sectors;
			}

			for (j = 0; j < fdd->inf.mhlt.trks[i].trk.sectors; j++) {
				/*	セクタ情報の読込	*/
				if (file_read(fh, &fdd->inf.mhlt.trks[i].ids[j], sizeof(_MHLT_SEC)) != sizeof(_MHLT_SEC)) {
					file_close(fh);
					return(FAILURE);
				}
			}
		}

		/*	読み込んだトラック情報、セクタ情報を元に各セクタへのオフセットを計算	*/
		ptr = 0;
		trk = fdd->inf.mhlt.trks;
		for (i = 0; i < fdd->inf.xdf.tracks; i++) {
			sec = trk[i].ids;
			for (j = 0; j < fdd->inf.mhlt.trks[i].trk.sectors; j++) {
				trk[i].ptr[j] = ptr;
				if (sec[j].flgDAT == 0x00) {
					ptr += 128 << sec[j].N;
				}
			}
		}
		fdd->inf.xdf.sectors	= secMax;
		fdd->inf.xdf.n	= fdd->inf.mhlt.trks[0].ids[0].N;

		/*	処理関数群を登録	*/
		fdd_fn->eject		= fdd_eject_xxx;
		fdd_fn->diskaccess	= fdd_diskaccess_common;
		fdd_fn->seek		= fdd_seek_common;
		fdd_fn->seeksector	= fdd_seeksector_mhlt;
		fdd_fn->read		= fdd_read_mhlt;
		fdd_fn->write		= fdd_write_mhlt;
		fdd_fn->readid		= fdd_readid_mhlt;
		fdd_fn->writeid		= fdd_dummy_xxx;
		fdd_fn->formatinit	= fdd_dummy_xxx;
		fdd_fn->formating	= fdd_formating_xxx;
		fdd_fn->isformating	= fdd_isformating_xxx;
		fdd_fn->fdcresult	= FALSE;
	}
	else
	{
		return(FAILURE);
	}

	return(SUCCESS);
}

BRESULT fdd_seeksector_mhlt(FDDFILE fdd)
{
	UINT	trk;
	BYTE	MaxR;
	UINT	i;

	TRACEOUT(("MAHALITO seeksector [%03d]", (fdc.treg[fdc.us] << 1) + fdc.hd));

	if ((CTRL_FDMEDIA != fdd->inf.xdf.disktype) ||
		(fdc.rpm[fdc.us] != fdd->inf.xdf.rpm) ||
		(fdc.treg[fdc.us] >= (fdd->inf.xdf.tracks >> 1))) {
		TRACEOUT(("fdd_seeksector_mhlt FAILURE CTRL_FDMEDIA[%02x], DISKTYPE[%02x]", CTRL_FDMEDIA, fdd->inf.xdf.disktype));
		TRACEOUT(("fdd_seeksector_mhlt FAILURE fdc.rpm[%02x], fdd->rpm[%02x]", fdc.rpm[fdc.us], fdd->inf.xdf.rpm));
		TRACEOUT(("fdd_seeksector_mhlt FAILURE fdc.treg[%02x], fdd->trk[%02x]", fdc.treg[fdc.us], (fdd->inf.xdf.tracks >> 1)));
		fddlasterror = 0xe0;
		return(FAILURE);
	}
	if (!fdc.R) {
		TRACEOUT(("fdd_seeksector_mhlt FAILURE fdc.R[%02x]", fdc.R));
		fddlasterror = 0xc0;
		return(FAILURE);
	}

	trk = (fdc.treg[fdc.us] << 1) + fdc.hd;
	MaxR = 0;
	for (i = 0; i < fdd->inf.mhlt.trks[trk].trk.sectors; i++) {
//		TRACEOUT(("fdd_seeksector_mhlt read sector_id[C:%02x,H:%02x,R:%02x,N:%02x]", sec_id.C, sec_id.H, sec_id.R, sec_id.N));
		if (fdd->inf.mhlt.trks[trk].ids[i].R > MaxR) {
			MaxR = fdd->inf.mhlt.trks[trk].ids[i].R;
		}
	}

	if (fdc.R > MaxR) {
		TRACEOUT(("fdd_seeksector_mhlt FAILURE fdc.R[%02x],MaxR[%02x]", fdc.R, MaxR));
		fddlasterror = 0xc0;
		return(FAILURE);
	}
	if ((fdc.mf != 0xff) && (fdc.mf != 0x40)) {
		TRACEOUT(("fdd_seeksector_mhlt FAILURE fdc.mf[%02x]", fdc.mf));
		fddlasterror = 0xc0;
		return(FAILURE);
	}
	return(SUCCESS);
}
//

BRESULT fdd_read_mhlt(FDDFILE fdd) {

	FILEH	hdl;
	UINT	trk;
	UINT	sec;
	UINT	secR;
	UINT	secsize;
	long	seekp;
	UINT	i;
	OEMCHAR		path[MAX_PATH];
	MHLT_SEC	pSec;

	fddlasterror = 0x00;
	if (fdd_seeksector_mhlt(fdd)) {
		TRACEOUT(("MAHALITO read FAILURE seeksector"));
		return(FAILURE);
	}
	trk = (fdc.treg[fdc.us] << 1) + fdc.hd;
	sec = fdc.R - 1;
	secR = 0xff;
	for (i = 0; i < fdd->inf.mhlt.trks[trk].trk.sectors; i++) {
		if (fdd->inf.mhlt.trks[trk].ids[i].R == fdc.R) {
			secR = i;
//			break;
		}
	}
	if (secR == 0xff) {
		TRACEOUT(("MAHALITO read FAILURE R[%02x] not found", fdc.R));
		fddlasterror = 0xc0;
		return(FAILURE);
	}
	pSec = &fdd->inf.mhlt.trks[trk].ids[secR];
	if (fdc.N != pSec->N) {
		TRACEOUT(("MAHALITO read FAILURE N not match FDC[%02x],DSK[%02x]", fdc.N, fdd->inf.mhlt.trks[trk].ids[secR].N));
		fddlasterror = 0xc0;
		return(FAILURE);
	}

	if (fdd->type == DISKTYPE_MAHALITO) {
		secsize = 128 << pSec->N;
		if (pSec->flgDAT == 0x00) {
			seekp = fdd->inf.mhlt.trks[trk].ptr[secR];

			file_cpyname(path, fdd->fname, NELEMENTS(path));
			file_cutext(path);
			file_catname(path, str_dat, NELEMENTS(path));

			hdl = file_open_rb(path);
			if (hdl == FILEH_INVALID) {
				fddlasterror = 0xe0;
				return(FAILURE);
			}
			TRACEOUT(("MAHALITO read seek to ... [%08x]", seekp));
			if ((file_seek(hdl, seekp, FSEEK_SET) != seekp) ||
				(file_read(hdl, fdc.buf, secsize) != secsize)) {
				file_close(hdl);
				fddlasterror = 0xe0;
				return(FAILURE);
			}
			file_close(hdl);
		}
		else {
			/*	繰り返しデータ有の場合	*/
			FillMemory(fdc.buf, secsize, pSec->DAT);
		}
	}

	fdc.bufcnt = secsize;

	return(SUCCESS);
}

BRESULT fdd_write_mhlt(FDDFILE fdd)
{
	FILEH	hdl;
	UINT	trk;
	UINT	sec;
	UINT	secR;
	UINT	secsize;
	long	seekp;
	UINT	i;

	/*	壊さないように仮にライトプロテクトしておく	*/
	fddlasterror = 0x70;
	return(FAILURE);

	fddlasterror = 0x00;
	if (fdd_seeksector_mhlt(fdd)) {
		fddlasterror = 0xe0;
		return(FAILURE);
	}
	if (fdd->protect) {
		fddlasterror = 0x70;
		return(FAILURE);
	}
	trk = (fdc.treg[fdc.us] << 1) + fdc.hd;
	sec = fdc.R - 1;
	secR = 0xff;
	for (i = 0; i < fdd->inf.mhlt.trks[trk].trk.sectors; i++) {
		if (fdd->inf.mhlt.trks[trk].ids[i].R == fdc.R) {
			secR = i;
			break;
		}
	}
	if (secR == 0xff) {
		return(FAILURE);
	}
	if (fdc.N != fdd->inf.nfd.head.r0.si[trk][secR].N) {
		fddlasterror = 0xc0;
		return(FAILURE);
	}

	if (fdd->type == DISKTYPE_MAHALITO) {
		secsize = 128 << fdd->inf.mhlt.trks[trk].ids[secR].N;
		seekp = fdd->inf.mhlt.trks[trk].ptr[secR];

		hdl = file_open(fdd->fname);
		if (hdl == FILEH_INVALID) {
			fddlasterror = 0xc0;
			return(FAILURE);
		}
		if ((file_seek(hdl, seekp, FSEEK_SET) != seekp) ||
			(file_write(hdl, fdc.buf, secsize) != secsize)) {
			file_close(hdl);
			fddlasterror = 0xc0;
			return(FAILURE);
		}
		file_close(hdl);
	}

	fdc.bufcnt = secsize;
	fddlasterror = 0x00;

	return(SUCCESS);
}

BRESULT fdd_readid_mhlt(FDDFILE fdd)
{
	UINT	trk;
	UINT	sec;
	UINT	i;

	fddlasterror = 0x00;
	if ((!fdc.mf) ||
		(fdc.rpm[fdc.us] != fdd->inf.xdf.rpm) ||
		(fdc.crcn >= fdd->inf.xdf.sectors)) {
		fddlasterror = 0xe0;
		return(FAILURE);
	}
	fdc.C = fdc.treg[fdc.us];
	fdc.H = fdc.hd;
	fdc.R = ++fdc.crcn;
	trk = (fdc.C << 1) + fdc.H;
	sec = 0xff;
	for (i = 0; i < fdd->inf.mhlt.trks[trk].trk.sectors; i++) {
		if (fdd->inf.mhlt.trks[trk].ids[i].R == fdc.R) {
			sec = i;
			break;
		}
	}
	if (sec == 0xff) {
		fddlasterror = 0xe0;
		return(FAILURE);
	}
	fdc.N = fdd->inf.mhlt.trks[trk].ids[sec].N;
	return(SUCCESS);
}

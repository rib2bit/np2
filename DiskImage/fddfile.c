/*	本家の物をベースに大幅改修	*/

#include	"compiler.h"
#include	"strres.h"
#include	"dosio.h"
#include	"sysmng.h"
#include	"pccore.h"
#include	"iocore.h"
#include	"fddfile.h"

#include	"DiskImage/FD/fdd_xdf.h"
#include	"DiskImage/FD/fdd_d88.h"
#include	"DiskImage/FD/fdd_dcp.h"
#include	"DiskImage/FD/fdd_bkdsk.h"
#include	"DiskImage/FD/fdd_nfd.h"
#include	"DiskImage/FD/fdd_vfdd.h"
#include	"DiskImage/FD/fdd_mahalito.h"


	_FDDFILE	fddfile[MAX_FDDFILE];
	_FDDFUNC	fddfunc[MAX_FDDFILE];
	UINT8		fddlasterror;

void fddfunc_init(FDDFUNC fdd_fn) {

	/*	とりあえずダミーで埋めておく	*/
	fdd_fn->eject		= fdd_eject_xxx;
	fdd_fn->diskaccess	= fdd_dummy_xxx;
	fdd_fn->seek		= fdd_dummy_xxx;
	fdd_fn->seeksector	= fdd_dummy_xxx;
	fdd_fn->read		= fdd_dummy_xxx;
	fdd_fn->write		= fdd_dummy_xxx;
	fdd_fn->readid		= fdd_dummy_xxx;
	fdd_fn->writeid		= fdd_dummy_xxx;
	fdd_fn->formatinit	= fdd_dummy_xxx;
	fdd_fn->formating	= fdd_formating_xxx;
	fdd_fn->isformating	= fdd_isformating_xxx;
	fdd_fn->fdcresult	= FALSE;
}

void fddfile_initialize(void) {

	UINT	i;

	for (i = 0; i < MAX_FDDFILE; i++) {
		fddfunc_init(&fddfunc[i]);
	}
	ZeroMemory(fddfile, sizeof(fddfile));
}

void fddfile_reset2dmode(void) { 			// ver0.29
#if 0
	int		i;

	for (i=0; i<4; i++) {
		fddfile[i].mode2d = 0;
	}
#endif
}

OEMCHAR *fdd_diskname(REG8 drv) {

	if (drv >= MAX_FDDFILE) {
		return(NULL);
	}
	return(fddfile[drv].fname);
}

OEMCHAR *fdd_getfileex(REG8 drv, UINT *ftype, int *ro) {

	FDDFILE	fdd;

	if (drv >= MAX_FDDFILE) {
		return((OEMCHAR *)str_null);
	}
	fdd = fddfile + drv;
	if (ftype) {
		*ftype = fdd->ftype;
	}
	if (ro) {
		*ro = fdd->ro;
	}
	return(fdd->fname);
}

BOOL fdd_diskready(REG8 drv) {

	if ((drv >= MAX_FDDFILE) || (!fddfile[drv].fname[0])) {
		return(FALSE);
	}
	return(TRUE);
}

BOOL fdd_diskprotect(REG8 drv) {

	if ((drv >= MAX_FDDFILE) || (!fddfile[drv].protect)) {
		return(FALSE);
	}
	return(TRUE);
}


// --------------------------------------------------------------------------

BRESULT fdd_set(REG8 drv, const OEMCHAR *fname, UINT ftype, int ro) {

	FDDFILE		fdd;
	FDDFUNC		fdd_fn;
	UINT		fddtype;
const OEMCHAR	*p;
	BRESULT		r;

	if (drv >= MAX_FDDFILE) {
		return(FAILURE);
	}
	fddtype = ftype;
	if (fddtype == FTYPE_NONE) {
		p = file_getext(fname);
		if ((!milstr_cmp(p, str_d88)) || (!milstr_cmp(p, str_88d)) ||
			(!milstr_cmp(p, str_d98)) || (!milstr_cmp(p, str_98d))) {
			fddtype = FTYPE_D88;
		}
		else if (!milstr_cmp(p, str_fdi)) {
			fddtype = FTYPE_FDI;
		}
		else if ((!milstr_cmp(p, str_dcp)) || (!milstr_cmp(p, str_dcu))) {
			fddtype = FTYPE_DCP;
		}
		else if (!milstr_cmp(p, str_nfd)) {
			fddtype = FTYPE_NFD;
		}
		else if (!milstr_cmp(p, str_vfdd)) {
			fddtype = FTYPE_VFDD;
		}
		else if ((!milstr_cmp(p, str_mhlt1)) || (!milstr_cmp(p, str_mhlt2)) ||
				 (!milstr_cmp(p, str_mhlt3))) {
			fddtype = FTYPE_MHLT;
		}
		else {
			fddtype = FTYPE_BETA;
		}
	}
	fdd = fddfile + drv;
	fdd_fn = fddfunc + drv;
	fdd_fn->eject(fdd);		/*	セット済ディスクをイジェクト	*/
	switch(fddtype) {
		case FTYPE_FDI:
			r = fdd_set_fdi(fdd, fdd_fn, fname, ro);
			if (r == SUCCESS) {
				break;
			}
			/* FALLTHROUGH */

		case FTYPE_BETA:
			r = fdd_set_xdf(fdd, fdd_fn, fname, ro);
			if (r != SUCCESS) {
				/*	BKDSK(HDB)	BASIC 2HDかな？かな？	*/
				r = fdd_set_bkdsk(fdd, fdd_fn, fname, ro);
				break;
			}
			break;

		case FTYPE_D88:
			r = fdd_set_d88(fdd, fdd_fn, fname, ro);
			break;
		case FTYPE_DCP:
			r = fdd_set_dcp(fdd, fdd_fn, fname, ro);
			break;
		case FTYPE_NFD:
			r = fdd_set_nfd(fdd, fdd_fn, fname, ro);
			break;
		case FTYPE_VFDD:
			r = fdd_set_vfdd(fdd, fdd_fn, fname, ro);
			break;
		case FTYPE_MHLT:
			r = fdd_set_mhlt(fdd, fdd_fn, fname, ro);
			if (r != SUCCESS) {
				/*	*.2HDはベタ形式の可能性が…	*/
				fddtype = FTYPE_BETA;
				r = fdd_set_xdf(fdd, fdd_fn, fname, ro);
				break;
			}
			break;
		default:
			r = FAILURE;
	}
	if (r == SUCCESS) {
		file_cpyname(fdd->fname, fname, NELEMENTS(fdd->fname));
		fdd->ftype = ftype;
		fdd->ro = ro;
	}
	return(FAILURE);
}

BRESULT fdd_eject(REG8 drv) {

	BRESULT		ret;
	FDDFILE		fdd;
	FDDFUNC		fdd_fn;

	if (drv >= MAX_FDDFILE) {
		return(FAILURE);
	}
	fdd = fddfile + drv;
	fdd_fn = fddfunc + drv;

	ret = fdd_fn->eject(fdd);

	ZeroMemory(fdd, sizeof(_FDDFILE));

	fdd->fname[0] = '\0';
	fdd->type = DISKTYPE_NOTREADY;

	fddfunc_init(fdd_fn);

	return ret;
}

//	----
/*	未実装、未対応用ダミー関数群	*/
BRESULT fdd_dummy_xxx(FDDFILE fdd)
{
	(void)fdd;
	return(FAILURE);
}

BRESULT fdd_eject_xxx(FDDFILE fdd)
{
	(void)fdd;
	return(SUCCESS);
}

BRESULT fdd_formating_xxx(FDDFILE fdd, const UINT8 *ID)
{
	(void)fdd;
	(void)ID;
	return(FAILURE);
}

BOOL fdd_isformating_xxx(FDDFILE fdd)
{
	(void)fdd;
	return(FAILURE);
}
// ----
/*	ベタ系イメージ用共通処理関数群	*/
BRESULT fdd_diskaccess_common(FDDFILE fdd)
{
	if (CTRL_FDMEDIA != fdd->inf.xdf.disktype) {
		TRACEOUT(("fdd_diskaccess_common [FAILURE] [%02x]:[%02x]",CTRL_FDMEDIA, fdd->inf.xdf.disktype));
		return(FAILURE);
	}
	TRACEOUT(("fdd_diskaccess_common [SUCCESS]"));
	return(SUCCESS);
}

BRESULT fdd_seek_common(FDDFILE fdd)
{
	if ((CTRL_FDMEDIA != fdd->inf.xdf.disktype) ||
		(fdc.rpm[fdc.us] != fdd->inf.xdf.rpm) ||
		(fdc.ncn >= (fdd->inf.xdf.tracks >> 1))) {
TRACEOUT(("fdd_seek_common [FAILURE] CTRL_FDMEDIA[%02x], DISKTYPE[%02x]", CTRL_FDMEDIA, fdd->inf.xdf.disktype));
TRACEOUT(("fdd_seek_common [FAILURE] fdc.rpm[%02x], fdd->rpm[%02x]", fdc.rpm[fdc.us], fdd->inf.xdf.rpm));
TRACEOUT(("fdd_seek_common [FAILURE] fdc.ncn[%02x], fdd->trk[%02x]", fdc.ncn, (fdd->inf.xdf.tracks >> 1)));
		return(FAILURE);
	}
	TRACEOUT(("fdd_seek_common [SUCCESS]"));
	return(SUCCESS);
}

BRESULT fdd_seeksector_common(FDDFILE fdd)
{
	if ((CTRL_FDMEDIA != fdd->inf.xdf.disktype) ||
		(fdc.rpm[fdc.us] != fdd->inf.xdf.rpm) ||
		(fdc.treg[fdc.us] >= (fdd->inf.xdf.tracks >> 1))) {
TRACEOUT(("fdd_seeksector_common [FAILURE] CTRL_FDMEDIA[%02x], DISKTYPE[%02x]", CTRL_FDMEDIA, fdd->inf.xdf.disktype));
TRACEOUT(("fdd_seeksector_common [FAILURE] fdc.rpm[%02x], fdd->rpm[%02x]", fdc.rpm[fdc.us], fdd->inf.xdf.rpm));
TRACEOUT(("fdd_seeksector_common [FAILURE] fdc.treg[%02x], fdd->trk[%02x]", fdc.treg[fdc.us], (fdd->inf.xdf.tracks >> 1)));
		fddlasterror = 0xe0;
		return(FAILURE);
	}
	if ((!fdc.R) || (fdc.R > fdd->inf.xdf.sectors)) {
TRACEOUT(("fdd_seeksector_common [FAILURE] fdc.R[%02x], Secters[%02x]", fdc.R, fdd->inf.xdf.sectors));
		fddlasterror = 0xc0;
		return(FAILURE);
	}
	if ((fdc.mf != 0xff) && (fdc.mf != 0x40)) {
TRACEOUT(("fdd_seeksector_common [FAILURE] fdc.mf[%02x]", fdc.mf));
		fddlasterror = 0xc0;
		return(FAILURE);
	}
	TRACEOUT(("fdd_seeksector_common [SUCCESS]"));
	return(SUCCESS);
}

BRESULT fdd_readid_common(FDDFILE fdd)
{
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
	fdc.N = fdd->inf.xdf.n;
	return(SUCCESS);
}
// ----

BRESULT fdd_diskaccess(void)
{
	FDDFILE		fdd;
	FDDFUNC		fdd_fn;

	fdd = fddfile + fdc.us;
	fdd_fn = fddfunc + fdc.us;

	return(fdd_fn->diskaccess(fdd));
}

BRESULT fdd_seek(void)
{
	BRESULT		ret;
	FDDFILE		fdd;
	FDDFUNC		fdd_fn;

	ret = FAILURE;
	fdd = fddfile + fdc.us;
	fdd_fn = fddfunc + fdc.us;

	ret = fdd_fn->seek(fdd);

	fdc.treg[fdc.us] = fdc.ncn;
	return(ret);
}

BRESULT fdd_seeksector(void)
{
	FDDFILE		fdd;
	FDDFUNC		fdd_fn;

	fdd = fddfile + fdc.us;
	fdd_fn = fddfunc + fdc.us;

	return(fdd_fn->seeksector(fdd));
}


BRESULT fdd_read(void)
{
	FDDFILE		fdd;
	FDDFUNC		fdd_fn;

	sysmng_fddaccess(fdc.us);
	fdd = fddfile + fdc.us;
	fdd_fn = fddfunc + fdc.us;

	return(fdd_fn->read(fdd));
}

BRESULT fdd_write(void)
{
	FDDFILE		fdd;
	FDDFUNC		fdd_fn;

	sysmng_fddaccess(fdc.us);
	fdd = fddfile + fdc.us;
	fdd_fn = fddfunc + fdc.us;

	return(fdd_fn->write(fdd));
}

BRESULT fdd_readid(void)
{
	FDDFILE		fdd;
	FDDFUNC		fdd_fn;

	sysmng_fddaccess(fdc.us);
	fdd = fddfile + fdc.us;
	fdd_fn = fddfunc + fdc.us;

	return(fdd_fn->readid(fdd));
}

BRESULT fdd_formatinit(void)
{
	FDDFILE		fdd;
	FDDFUNC		fdd_fn;

	fdd = fddfile + fdc.us;
	fdd_fn = fddfunc + fdc.us;

	return(fdd_fn->formatinit(fdd));
}

BRESULT fdd_formating(const UINT8 *ID)
{
	FDDFILE		fdd;
	FDDFUNC		fdd_fn;

	sysmng_fddaccess(fdc.us);
	fdd = fddfile + fdc.us;
	fdd_fn = fddfunc + fdc.us;

	return(fdd_fn->formating(fdd, ID));
}

BOOL fdd_isformating(void)
{
	FDDFILE		fdd;
	FDDFUNC		fdd_fn;

	fdd = fddfile + fdc.us;
	fdd_fn = fddfunc + fdc.us;

	return(fdd_fn->formatinit(fdd));
}

BOOL fdd_fdcresult(void) {

	FDDFUNC		fdd_fn;

	fdd_fn = fddfunc + fdc.us;

	return(fdd_fn->fdcresult);
}

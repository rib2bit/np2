#include	"compiler.h"
#include	"strres.h"
#include	"textfile.h"
#include	"dosio.h"
#include	"sysmng.h"
//#include	"cpucore.h"
//#include	"pccore.h"
#include	"sxsi.h"

#include	"DiskImage/cddfile.h"
#include	"DiskImage/CD/cdd_iso.h"
#include	"DiskImage/CD/cdd_cue.h"
#include	"DiskImage/CD/cdd_ccd.h"
#include	"DiskImage/CD/cdd_mds.h"
#include	"DiskImage/CD/cdd_nrg.h"

BRESULT sxsicd_open(SXSIDEV sxsi, const OEMCHAR *fname) {

const OEMCHAR	*ext;

	//	�Ƃ肠�����g���q�Ŕ��f
	ext = file_getext(fname);
	if (!file_cmpname(ext, str_cue)) {			//	CUE�V�[�g(*.cue)
		return(opencue(sxsi, fname));
	}
	else if (!file_cmpname(ext, str_ccd)) {		//	CloneCD(*.ccd)�ɑΉ�
		return(openccd(sxsi, fname));
	}
	else if (!file_cmpname(ext, str_cdm)) {		//	CD Manipulator(*.cdm)�ɑΉ�(�ǂݕ���CloneCD�ƈꏏ)
		return(openccd(sxsi, fname));
//		return(opencdm(sxsi, fname));
	}
	else if (!file_cmpname(ext, str_mds)) {		//	Media Descriptor(*.mds)�ɑΉ�
		return(openmds(sxsi, fname));
	}
	else if (!file_cmpname(ext, str_nrg)) {		//	Nero(*.nrg)�ɑΉ�
		return(opennrg(sxsi, fname));
	}

	return(openiso(sxsi, fname));				//	�m��Ȃ��g���q�Ȃ�A�Ƃ肠����ISO�Ƃ��ĊJ���Ă݂�
}

CDTRK sxsicd_gettrk(SXSIDEV sxsi, UINT *tracks) {

	CDINFO	cdinfo;

	cdinfo = (CDINFO)sxsi->hdl;
	if (tracks) {
		*tracks = cdinfo->trks;
	}
	return(cdinfo->trk);
}
#if 0
BRESULT sxsicd_readraw(SXSIDEV sxsi, long pos, void *buf) {

	CDINFO	cdinfo;
	FILEH	fh;
	long	fpos;

	cdinfo = (CDINFO)sxsi->hdl;
	if (cdinfo->type != 2352) {
		return(FAILURE);
	}
	if (sxsi_prepare(sxsi) != SUCCESS) {
		return(FAILURE);
	}
	if ((pos < 0) || (pos >= sxsi->totals)) {
		return(FAILURE);
	}
	fh = ((CDINFO)sxsi->hdl)->fh;
	fpos = pos * 2352;
	if ((file_seek(fh, fpos, FSEEK_SET) != fpos) ||
		(file_read(fh, buf, 2352) != 2352)) {
		return(FAILURE);
	}
	return(SUCCESS);
}
#else

BRESULT sxsicd_readraw(SXSIDEV sxsi, long pos, void *buf) {

	CDINFO	cdinfo;
	FILEH	fh;
	INT64	fpos;
	UINT16	secsize;
	UINT	i;
	UINT32	secs;

	//	�͈͊O�͎��s
	if ((pos < 0) || (sxsi->totals < pos)) {
		return(FAILURE);
	}

	cdinfo = (CDINFO)sxsi->hdl;

	//	pos�ʒu�̃Z�N�^�T�C�Y���擾
	for (i = cdinfo->trks - 1; i > 0; i--) {
		if (cdinfo->trk[i].pos <= pos) {
			secsize = cdinfo->trk[i].sector_size;
			break;
		}
	}
	if (secsize == 2048) {
		return(FAILURE);
	}

	if (sxsi_prepare(sxsi) != SUCCESS) {
		return(FAILURE);
	}

	fh = ((CDINFO)sxsi->hdl)->fh;
	fpos = 0;
	secs = 0;
	for (i = 0; i < cdinfo->trks; i++) {
		if (cdinfo->trk[i].str_sec <= pos && pos <= cdinfo->trk[i].end_sec) {
			fpos += (pos - secs) * cdinfo->trk[i].sector_size;
			break;
		}
		fpos += cdinfo->trk[i].sectors * cdinfo->trk[i].sector_size;
		secs += cdinfo->trk[i].sectors;
	}
	fpos += cdinfo->trk[0].start_offset;
	if ((file_seeki64(fh, fpos, FSEEK_SET) != fpos) ||
		(file_read(fh, buf, 2352) != 2352)) {
		return(FAILURE);
	}

	return(SUCCESS);
}
#endif

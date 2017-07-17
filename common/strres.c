#include	"compiler.h"
#include	"strres.h"


const UINT8 str_utf8[3] = {0xef, 0xbb, 0xbf};
const UINT16 str_ucs2[1] = {0xfeff};


const OEMCHAR str_null[] = OEMTEXT("");
const OEMCHAR str_space[] = OEMTEXT(" ");
const OEMCHAR str_dot[] = OEMTEXT(".");

const OEMCHAR str_cr[] = OEMTEXT("\r");
const OEMCHAR str_crlf[] = OEMTEXT("\r\n");

const OEMCHAR str_ini[] = OEMTEXT("ini");
const OEMCHAR str_cfg[] = OEMTEXT("cfg");
const OEMCHAR str_sav[] = OEMTEXT("sav");
const OEMCHAR str_bmp[] = OEMTEXT("bmp");
const OEMCHAR str_d88[] = OEMTEXT("d88");
const OEMCHAR str_d98[] = OEMTEXT("d98");
const OEMCHAR str_88d[] = OEMTEXT("88d");
const OEMCHAR str_98d[] = OEMTEXT("98d");
const OEMCHAR str_thd[] = OEMTEXT("thd");
const OEMCHAR str_hdi[] = OEMTEXT("hdi");
const OEMCHAR str_fdi[] = OEMTEXT("fdi");
const OEMCHAR str_hdd[] = OEMTEXT("hdd");
const OEMCHAR str_nhd[] = OEMTEXT("nhd");

const OEMCHAR str_d[] = OEMTEXT("%d");
const OEMCHAR str_u[] = OEMTEXT("%u");
const OEMCHAR str_x[] = OEMTEXT("%x");
const OEMCHAR str_2d[] = OEMTEXT("%.2d");
const OEMCHAR str_2x[] = OEMTEXT("%.2x");
const OEMCHAR str_4x[] = OEMTEXT("%.4x");
const OEMCHAR str_4X[] = OEMTEXT("%.4X");

const OEMCHAR str_false[] = OEMTEXT("false");
const OEMCHAR str_true[] = OEMTEXT("true");

const OEMCHAR str_posx[] = OEMTEXT("posx");
const OEMCHAR str_posy[] = OEMTEXT("posy");
const OEMCHAR str_width[] = OEMTEXT("width");
const OEMCHAR str_height[] = OEMTEXT("height");

const OEMCHAR str_np2[] = OEMTEXT("Neko Project II");
const OEMCHAR str_resume[] = OEMTEXT("Resume");

const OEMCHAR str_VM[] = OEMTEXT("VM");
const OEMCHAR str_VX[] = OEMTEXT("VX");
const OEMCHAR str_EPSON[] = OEMTEXT("EPSON");

const OEMCHAR str_biosrom[] = OEMTEXT("bios.rom");
const OEMCHAR str_sasirom[] = OEMTEXT("sasi.rom");
const OEMCHAR str_scsirom[] = OEMTEXT("scsi.rom");

/*	Å´Ç±Ç±Ç©ÇÁ(Kaiî≈í«â¡ï™)	*/
/*	í«â¡(kai1)	*/
const OEMCHAR str_dcp[]		= OEMTEXT("dcp");
const OEMCHAR str_dcu[]		= OEMTEXT("dcu");
const OEMCHAR str_nfd[]		= OEMTEXT("nfd");
const OEMCHAR str_vfdd[]	= OEMTEXT("fdd");

const OEMCHAR str_cue[]		= OEMTEXT("cue");	/*	CUEÉVÅ[Ég										*/
const OEMCHAR str_ccd[]		= OEMTEXT("ccd");	/*	CloneCDÇ…ëŒâûÇµÇƒÇ›ÇÈ							*/
const OEMCHAR str_cdm[]		= OEMTEXT("cdm");	/*	CD ManipulatorÇ…ëŒâûÇµÇƒÇ›ÇÈ					*/
const OEMCHAR str_mds[]		= OEMTEXT("mds");	/*	Media Descriptor(Alcohol 52%ìô)Ç…ëŒâûÇµÇƒÇ›ÇÈ	*/
const OEMCHAR str_nrg[]		= OEMTEXT("nrg");	/*	NeroÇ…ëŒâûÇµÇƒÇ›ÇÈ								*/
const OEMCHAR str_iso[]		= OEMTEXT("iso");

/*	í«â¡(kai2)	*/
const OEMCHAR str_mhlt1[]	= OEMTEXT("2hd");
const OEMCHAR str_mhlt2[]	= OEMTEXT("2dd");
const OEMCHAR str_mhlt3[]	= OEMTEXT("2d");
/*	Å™Ç±Ç±Ç‹Ç≈(Kaiî≈í«â¡ï™)	*/


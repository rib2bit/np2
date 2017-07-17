enum {
	MHLT_HEADERSIZE	= 11,
};

#if defined(__GNUC__)
/*	ヘッダー情報	*/
typedef struct {
	SINT8	verID[10];	/*	データのバージョン番号など	*/
						/*	"ver 1.10  "   2HDのとき	*/
						/*	"2DD ver1.0"   2DDのとき	*/
						/*	"2D  ver1.0"   2D のとき	*/
	SINT8	cly[1];		/*	シリンダ数（既定値は77(2HD)	*/
						/*	or80(2DD)or40(2D)）			*/
						/*	ｼﾘﾝﾀﾞ数*2回、ﾄﾗｯｸ情報と		*/
						/*	ｾｸﾀ情報を繰返す				*/
} __attribute__ ((packed)) _MHLT_HEAD, *MHLT_HEAD;

/*	トラック情報	*/
typedef struct {
	UINT8	sectors;	/*	トラックあたりのセクタ数	*/
						/*	ゼロのときアンフォーマット	*/
						/*	ゼロ以外のとき、この回数	*/
						/*	だけセクタ情報を繰り返す	*/
	UINT8	flgMFM;		/*	ＦＭ／ＭＦＭ				*/
						/*	0 : FM   1 : MFM			*/
} __attribute__ ((packed)) _MHLT_TRK, *MHLT_TRK;

/*	セクタ情報		*/
typedef struct {
	UINT8	flgDAT;		/*	繰り返しデータ有無			*/
						/*	0 : 無し 1 : 有り			*/
	UINT8	DAT;		/*	繰り返しデータ				*/
						/*	繰り返し有りのとき有効		*/
	UINT8	C;			/*	セクタＩＤ（論理シリンダ）	*/
	UINT8	H;			/*	セクタＩＤ（論理ヘッド）	*/
	UINT8	R;			/*	セクタＩＤ（論理セクタ番号）*/
	UINT8	N;			/*	セクタＩＤ（セクタ長）		*/
						/*	0:128 1:256 2:512･････		*/
	UINT8	rN;			/*	実セクタ長（現在未使用）	*/
						/*	0:128 1:256 2:512･････		*/
	UINT8	DDAM;		/*	ＤＡＭ／ＤＤＡＭ			*/
						/*	0 : DAM  1 : DDAM			*/
} __attribute__ ((packed)) _MHLT_SEC, *MHLT_SEC;
#else
#pragma pack(push, 1)
/*	ヘッダー情報	*/
typedef struct {
	SINT8	verID[10];	/*	データのバージョン番号など	*/
						/*	"ver 1.10  "   2HDのとき	*/
						/*	"2DD ver1.0"   2DDのとき	*/
						/*	"2D  ver1.0"   2D のとき	*/
	SINT8	cly;		/*	シリンダ数（既定値は77(2HD)	*/
						/*	or80(2DD)or40(2D)）			*/
						/*	ｼﾘﾝﾀﾞ数*2回、ﾄﾗｯｸ情報と		*/
						/*	ｾｸﾀ情報を繰返す				*/
} _MHLT_HEAD, *MHLT_HEAD;

/*	トラック情報	*/
typedef struct {
	UINT8	sectors;	/*	トラックあたりのセクタ数	*/
						/*	ゼロのときアンフォーマット	*/
						/*	ゼロ以外のとき、この回数	*/
						/*	だけセクタ情報を繰り返す	*/
	UINT8	flgMFM;		/*	ＦＭ／ＭＦＭ				*/
						/*	0 : FM   1 : MFM			*/
} _MHLT_TRK, *MHLT_TRK;

/*	セクタ情報		*/
typedef struct {
	UINT8	flgDAT;		/*	繰り返しデータ有無			*/
						/*	0 : 無し 1 : 有り			*/
	UINT8	DAT;		/*	繰り返しデータ				*/
						/*	繰り返し有りのとき有効		*/
	UINT8	C;			/*	セクタＩＤ（論理シリンダ）	*/
	UINT8	H;			/*	セクタＩＤ（論理ヘッド）	*/
	UINT8	R;			/*	セクタＩＤ（論理セクタ番号）*/
	UINT8	N;			/*	セクタＩＤ（セクタ長）		*/
						/*	0:128 1:256 2:512･････		*/
	UINT8	rN;			/*	実セクタ長（現在未使用）	*/
						/*	0:128 1:256 2:512･････		*/
	UINT8	DDAM;		/*	ＤＡＭ／ＤＤＡＭ			*/
						/*	0 : DAM  1 : DDAM			*/
} _MHLT_SEC, *MHLT_SEC;
#pragma pack(pop)
#endif


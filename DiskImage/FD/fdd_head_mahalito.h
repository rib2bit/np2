enum {
	MHLT_HEADERSIZE	= 11,
};

#if defined(__GNUC__)
/*	�w�b�_�[���	*/
typedef struct {
	SINT8	verID[10];	/*	�f�[�^�̃o�[�W�����ԍ��Ȃ�	*/
						/*	"ver 1.10  "   2HD�̂Ƃ�	*/
						/*	"2DD ver1.0"   2DD�̂Ƃ�	*/
						/*	"2D  ver1.0"   2D �̂Ƃ�	*/
	SINT8	cly[1];		/*	�V�����_���i����l��77(2HD)	*/
						/*	or80(2DD)or40(2D)�j			*/
						/*	����ސ�*2��A�ׯ�����		*/
						/*	��������J�Ԃ�				*/
} __attribute__ ((packed)) _MHLT_HEAD, *MHLT_HEAD;

/*	�g���b�N���	*/
typedef struct {
	UINT8	sectors;	/*	�g���b�N������̃Z�N�^��	*/
						/*	�[���̂Ƃ��A���t�H�[�}�b�g	*/
						/*	�[���ȊO�̂Ƃ��A���̉�	*/
						/*	�����Z�N�^�����J��Ԃ�	*/
	UINT8	flgMFM;		/*	�e�l�^�l�e�l				*/
						/*	0 : FM   1 : MFM			*/
} __attribute__ ((packed)) _MHLT_TRK, *MHLT_TRK;

/*	�Z�N�^���		*/
typedef struct {
	UINT8	flgDAT;		/*	�J��Ԃ��f�[�^�L��			*/
						/*	0 : ���� 1 : �L��			*/
	UINT8	DAT;		/*	�J��Ԃ��f�[�^				*/
						/*	�J��Ԃ��L��̂Ƃ��L��		*/
	UINT8	C;			/*	�Z�N�^�h�c�i�_���V�����_�j	*/
	UINT8	H;			/*	�Z�N�^�h�c�i�_���w�b�h�j	*/
	UINT8	R;			/*	�Z�N�^�h�c�i�_���Z�N�^�ԍ��j*/
	UINT8	N;			/*	�Z�N�^�h�c�i�Z�N�^���j		*/
						/*	0:128 1:256 2:512�����		*/
	UINT8	rN;			/*	���Z�N�^���i���ݖ��g�p�j	*/
						/*	0:128 1:256 2:512�����		*/
	UINT8	DDAM;		/*	�c�`�l�^�c�c�`�l			*/
						/*	0 : DAM  1 : DDAM			*/
} __attribute__ ((packed)) _MHLT_SEC, *MHLT_SEC;
#else
#pragma pack(push, 1)
/*	�w�b�_�[���	*/
typedef struct {
	SINT8	verID[10];	/*	�f�[�^�̃o�[�W�����ԍ��Ȃ�	*/
						/*	"ver 1.10  "   2HD�̂Ƃ�	*/
						/*	"2DD ver1.0"   2DD�̂Ƃ�	*/
						/*	"2D  ver1.0"   2D �̂Ƃ�	*/
	SINT8	cly;		/*	�V�����_���i����l��77(2HD)	*/
						/*	or80(2DD)or40(2D)�j			*/
						/*	����ސ�*2��A�ׯ�����		*/
						/*	��������J�Ԃ�				*/
} _MHLT_HEAD, *MHLT_HEAD;

/*	�g���b�N���	*/
typedef struct {
	UINT8	sectors;	/*	�g���b�N������̃Z�N�^��	*/
						/*	�[���̂Ƃ��A���t�H�[�}�b�g	*/
						/*	�[���ȊO�̂Ƃ��A���̉�	*/
						/*	�����Z�N�^�����J��Ԃ�	*/
	UINT8	flgMFM;		/*	�e�l�^�l�e�l				*/
						/*	0 : FM   1 : MFM			*/
} _MHLT_TRK, *MHLT_TRK;

/*	�Z�N�^���		*/
typedef struct {
	UINT8	flgDAT;		/*	�J��Ԃ��f�[�^�L��			*/
						/*	0 : ���� 1 : �L��			*/
	UINT8	DAT;		/*	�J��Ԃ��f�[�^				*/
						/*	�J��Ԃ��L��̂Ƃ��L��		*/
	UINT8	C;			/*	�Z�N�^�h�c�i�_���V�����_�j	*/
	UINT8	H;			/*	�Z�N�^�h�c�i�_���w�b�h�j	*/
	UINT8	R;			/*	�Z�N�^�h�c�i�_���Z�N�^�ԍ��j*/
	UINT8	N;			/*	�Z�N�^�h�c�i�Z�N�^���j		*/
						/*	0:128 1:256 2:512�����		*/
	UINT8	rN;			/*	���Z�N�^���i���ݖ��g�p�j	*/
						/*	0:128 1:256 2:512�����		*/
	UINT8	DDAM;		/*	�c�`�l�^�c�c�`�l			*/
						/*	0 : DAM  1 : DDAM			*/
} _MHLT_SEC, *MHLT_SEC;
#pragma pack(pop)
#endif


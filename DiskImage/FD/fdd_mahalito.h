
#ifdef __cplusplus
extern "C" {
#endif

BRESULT	fdd_set_mhlt(FDDFILE fdd, FDDFUNC fdd_fn, const OEMCHAR *fname, int ro);

BRESULT fdd_seeksector_mhlt(FDDFILE fdd);
BRESULT	fdd_read_mhlt(FDDFILE fdd);
BRESULT	fdd_write_mhlt(FDDFILE fdd);
BRESULT fdd_readid_mhlt(FDDFILE fdd);

#ifdef __cplusplus
}
#endif


#ifndef XBRZ_SCALER_NP2
#define XBRZ_SCALER_NP2

#ifdef __cplusplus
#include <cstdint>
#endif

#ifdef __cplusplus
namespace xbrz_scaler
{
#endif
	uint32_t buffer[640 * 480 * 6 * 6];
#ifdef __cplusplus
	extern "C"
#endif
	void scaleXBRZ(uint32_t factor, const uint32_t * src, uint32_t * trg, uint32_t rowBytes);
#ifdef __cplusplus
}
#endif
#endif

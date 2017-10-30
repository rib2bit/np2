#include "xbrzscaler.h"
#include "xbrz.h"

#define USE_TASK_SCHEDULER_PPL
//#define USE_TASK_SCHEDULER_TBB


#ifdef USE_TASK_SCHEDULER_TBB
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#elif defined USE_TASK_SCHEDULER_PPL
#include <ppl.h>
#endif

using namespace xbrz_scaler;

namespace
{
	const int TASK_GRANULARITY = 16; //granularity 1 has noticeable overhead for xBRZ
}

void xbrz_scaler::scaleXBRZ(uint32_t factor, const uint32_t* src, uint32_t* trg, uint32_t rowBytes)
{
	if (factor > 6) return;

	concurrency::task_group tg;

	auto srcWidth = 640;
	auto srcHeight = 480;
	for (int i = 0; i < srcWidth; i += TASK_GRANULARITY)
		tg.run([=]
	{
		const int iLast = std::min(i + TASK_GRANULARITY, srcHeight);
		xbrz::scale(factor, src, buffer, srcWidth, srcWidth, xbrz::ColorFormat::RGB, xbrz::ScalerCfg(), i, iLast);
	});
	tg.wait();

	srcWidth *= factor;
	srcHeight *= factor;
	for (int i = 0; i < srcWidth; i += TASK_GRANULARITY)
		tg.run([=]
	{
		const int iLast = std::min(i + TASK_GRANULARITY, srcHeight);
		xbrz::nearestNeighborScale(buffer, srcWidth, srcHeight, srcWidth * sizeof(uint32_t), trg, srcWidth, srcHeight, rowBytes, xbrz::NN_SCALE_SLICE_TARGET, i, iLast);
	});
	tg.wait();
}


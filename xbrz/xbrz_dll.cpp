// ****************************************************************************
// * This file is part of the HqMAME project. It is distributed under         *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0          *
// * Copyright (C) Zenju (zenju AT gmx DOT de) - All Rights Reserved          *
// *                                                                          *
// * Additionally and as a special exception, the author gives permission     *
// * to link the code of this program with the MAME library (or with modified *
// * versions of MAME that use the same license as MAME), and distribute      *
// * linked combinations including the two. You must obey the GNU General     *
// * Public License in all respects for all of the code used other than MAME. *
// * If you modify this file, you may extend this exception to your version   *
// * of the file, but you are not obligated to do so. If you do not wish to   *
// * do so, delete this exception statement from your version.                *
// ****************************************************************************

#define USE_TASK_SCHEDULER_PPL
//#define USE_TASK_SCHEDULER_TBB

#include "xbrz_dll.h"
#include <cassert>
#include "../xBRZ/xbrz.h"

#ifdef USE_TASK_SCHEDULER_TBB
    #include <tbb/task_scheduler_init.h>
    #include <tbb/parallel_for.h>
#elif defined USE_TASK_SCHEDULER_PPL
    #include <ppl.h>
#endif

using namespace xbrz_dll;

namespace
{
const int TASK_GRANULARITY = 16; //granularity 1 has noticeable overhead for xBRZ

xbrz::ColorFormat convert(xbrz_dll::ColorFormat colFmt)
{
    switch (colFmt)
    {
        case COLOR_FORMAT_RGB:
            return xbrz::ColorFormat::RGB;
        case COLOR_FORMAT_ARGB:
            return xbrz::ColorFormat::ARGB;
    }
    assert(false);
    return xbrz::ColorFormat::ARGB;
}
}

struct xbrz_dll::ScalerData
{
#ifdef USE_TASK_SCHEDULER_TBB
    //ScalerData() : scheduler(1) {} //force single thread
    tbb::task_scheduler_init scheduler;
    //"To minimize time overhead, it is best to rely upon automatic creation of the task schedule"

#elif defined USE_TASK_SCHEDULER_PPL
#endif
};


Handle xbrz_dll::createScaler()
{
    return new (std::nothrow) ScalerData;
}


void xbrz_dll::releaseScaler(Handle hnd)
{
    delete hnd;
}


void xbrz_dll::scaleXBRZ(Handle hnd, size_t factor, const uint32_t* src, uint32_t* trg, int srcWidth, int srcHeight, xbrz_dll::ColorFormat colFmt, const xbrz::ScalerCfg& cfg)
{
#ifdef USE_TASK_SCHEDULER_TBB
    tbb::parallel_for(tbb::blocked_range<int>(0, srcHeight, TASK_GRANULARITY),
                      [=, &cfg](const tbb::blocked_range<int>& r)
    {
        xbrz::scale(factor, src, trg, srcWidth, srcHeight, convert(colFmt), cfg, r.begin(), r.end());
    });

#elif defined USE_TASK_SCHEDULER_PPL
    concurrency::task_group tg;

    for (int i = 0; i < srcHeight; i += TASK_GRANULARITY)
        tg.run([=, &cfg]
    {
        const int iLast = std::min(i + TASK_GRANULARITY, srcHeight);
        xbrz::scale(factor, src, trg, srcWidth, srcHeight, convert(colFmt), cfg, i, iLast);
    });
    tg.wait();
#endif
}


void xbrz_dll::scaleNearestNeighbor(Handle hnd,
                                    const uint32_t* src, int srcWidth, int srcHeight,
                                    uint32_t* trg, int trgWidth, int trgHeight, int trgPitch)
{
    //perf: for images of similar size going over target is much faster than going over source
#ifdef USE_TASK_SCHEDULER_TBB
    tbb::parallel_for(tbb::blocked_range<int>(0, trgHeight, TASK_GRANULARITY),
                      [=](const tbb::blocked_range<int>& r)
    {
        xbrz::nearestNeighborScale(src, srcWidth, srcHeight, srcWidth * sizeof(uint32_t),
                                   trg, trgWidth, trgHeight, trgPitch,
                                   xbrz::NN_SCALE_SLICE_TARGET, r.begin(), r.end());
    });

#elif defined USE_TASK_SCHEDULER_PPL
    concurrency::task_group tg;

    for (int i = 0; i < trgHeight; i += TASK_GRANULARITY)
        tg.run([=]
    {
        const int iLast = std::min(i + TASK_GRANULARITY, trgHeight);
        xbrz::nearestNeighborScale(src, srcWidth, srcHeight, srcWidth * sizeof(uint32_t), trg, trgWidth, trgHeight, trgPitch, xbrz::NN_SCALE_SLICE_TARGET, i, iLast);
    });
    tg.wait();
#endif
}


bool xbrz_dll::equalColor(uint32_t col1, uint32_t col2, xbrz_dll::ColorFormat colFmt, double luminanceWeight, double equalColorTolerance)
{
    return xbrz::equalColorTest(col1, col2, convert(colFmt), luminanceWeight, equalColorTolerance);
}

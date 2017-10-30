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

#ifndef XBRZ_DLL_HEADER_84230572609874
#define XBRZ_DLL_HEADER_84230572609874

#include <stdint.h> //<cstdint> is C++11 only!
#include "../xBRZ/config.h"

#ifdef XBRZ_DLL_EXPORTS
    #define DLL_FUNCTION_DECLARATION extern "C" __declspec(dllexport)
#else
    #define DLL_FUNCTION_DECLARATION extern "C" __declspec(dllimport)
#endif

//A multithreaded xBRZ implementation

namespace xbrz_dll
{
/*--------------
  |declarations|
  --------------*/

struct ScalerData;
typedef ScalerData* Handle;

enum ColorFormat //keep header compatible with pre-C++11 code, e.g. MAME!
{
    COLOR_FORMAT_ARGB,
    COLOR_FORMAT_RGB,
};

DLL_FUNCTION_DECLARATION
Handle createScaler(); //returns nullptr on out of memory

DLL_FUNCTION_DECLARATION
void releaseScaler(Handle hnd);

DLL_FUNCTION_DECLARATION void scaleXBRZ(Handle hnd, size_t factor, const uint32_t* src, uint32_t* trg, int srcWidth, int srcHeight, ColorFormat colFmt, const xbrz::ScalerCfg& cfg);

DLL_FUNCTION_DECLARATION void scaleNearestNeighbor(Handle hnd, const uint32_t* src, int srcWidth, int srcHeight,
                                                   uint32_t* trg, int trgWidth, int trgHeight, int trgPitch); //pitch in bytes!

DLL_FUNCTION_DECLARATION bool equalColor(uint32_t col1, uint32_t col2, ColorFormat colFmt, double luminanceWeight, double equalColorTolerance);

#undef DLL_FUNCTION_DECLARATION
//##########################################################################################


/*----------
  |typedefs|
  ----------*/
typedef Handle (*FunType_createScaler)();
typedef void (*FunType_releaseScaler)(Handle hnd);
typedef void (*FunType_scaleXBRZ)(Handle hnd, size_t factor, const uint32_t* src, uint32_t* trg, int srcWidth, int srcHeight, ColorFormat colFmt, const xbrz::ScalerCfg& cfg);
typedef void (*FunType_scaleNearestNeighbor)(Handle hnd, const uint32_t* src, int srcWidth, int srcHeight, uint32_t* trg, int trgWidth, int trgHeight, int trgPitch);
typedef bool (*FunType_equalColor)(uint32_t col1, uint32_t col2, double luminanceWeight, double equalColorTolerance);

/*--------------
  |symbol names|
  --------------*/
//(use const arrays to ensure internal linkage)
const char funName_createScaler [] = "createScaler";
const char funName_releaseScaler[] = "releaseScaler";
const char funName_scaleXBRZ    [] = "scaleXBRZ";
const char funName_scaleNearestNeighbor[] = "scaleNearestNeighbor";
const char funName_equalColor   [] = "equalColor";

/*---------------
  |library names|
  ---------------*/
/* -> avoid zen-dependency
inline
const wchar_t* getDllName() { return zen::is64BitBuild ? L"xBRZ_x64.dll" : L"xBRZ_Win32.dll"; }
*/
}

#endif //XBRZ_DLL_HEADER_84230572609874

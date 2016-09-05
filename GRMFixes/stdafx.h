#pragma once

#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <algorithm>
#include "detours.h"
#include <map>
#include <string>

#pragma comment(lib, "winmm.lib")

enum zTRnd_TextureStageState
{
	zRND_TSS_COLOROP = 0,
	zRND_TSS_COLORARG1 ,
	zRND_TSS_COLORARG2 ,
	zRND_TSS_ALPHAOP ,
	zRND_TSS_ALPHAARG1 ,
	zRND_TSS_ALPHAARG2 ,
	zRND_TSS_BUMPENVMAT00 ,
	zRND_TSS_BUMPENVMAT01 ,
	zRND_TSS_BUMPENVMAT10 ,
	zRND_TSS_BUMPENVMAT11 ,
	zRND_TSS_TEXCOORDINDEX ,
	zRND_TSS_ADDRESS ,
	zRND_TSS_ADDRESSU ,
	zRND_TSS_ADDRESSV ,
	zRND_TSS_BORDERCOLOR ,
	zRND_TSS_MAGFILTER ,
	zRND_TSS_MINFILTER ,
	zRND_TSS_MIPFILTER ,
	zRND_TSS_MIPMAPLODBIAS ,
	zRND_TSS_MAXMIPLEVEL ,
	zRND_TSS_MAXANISOTROPY ,
	zRND_TSS_BUMPENVLSCALE ,
	zRND_TSS_BUMPENVLOFFSET ,
	zRND_TSS_TEXTURETRANSFORMFLAGS ,

	zRND_TSS_COUNT ,
};
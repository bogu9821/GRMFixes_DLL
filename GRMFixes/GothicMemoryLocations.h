#pragma once
#include "zCArray.h"

#pragma warning(disable: 4731) // Change of ebp from inline assembly

// -- call macro from GothicX (thx, Zerxes!)
#define XCALL(uAddr)                    \
        __asm { mov esp, ebp    }       \
        __asm { pop ebp                 }       \
        __asm { mov eax, uAddr  }       \
        __asm { jmp eax                 }

#define THISPTR_OFFSET(x) (((char *)this) + (x))

#ifdef GAME
#define GothicMemoryLocations GothicMemoryLocations_Game
#else
#define GothicMemoryLocations GothicMemoryLocations_Spacer
#endif

struct float3
{
	float x,y,z;
};

struct float2
{
	float x,y,z;
};

struct zCVertex
{
	float3 Position;

	int TransformedIndex;
	int MyIndex;
};

struct zCVertFeature
{
	float3 normal;
	DWORD lightStatic;
	DWORD lightDynamic;
	float2 texCoord;
};

struct zTPlane
{
	float Distance;
	float3 Normal;
};

class zCMaterial;
class zCLightMap;
struct zCPolygon
{
	zCVertex** Vertices;
	int LastTimeDrawn;
	zTPlane	PolyPlane;
	zCMaterial* Material;
	zCLightMap* Lightmap;
	zCVertex** ClipVertices;
	zCVertFeature** ClipFeatures;
	int NumClipVert;	
	zCVertFeature** Features;
	unsigned char PolyNumVert;
};

struct zCMesh
{
	char data[0x34];

	int				numPoly;
	int				numVert;
	int             numFeat;

	zCVertex		**vertList;
	zCPolygon		**polyList;
	zCVertFeature   **featList;

	zCVertex        *vertArray;
	zCPolygon       *polyArray;
	zCVertFeature   *featArray;
};

struct GothicMemoryLocations_Game
{
	struct LoadBin
	{
		static const int ASM_BSP_ERROR_START = 0x00525452;
		static const int ASM_BSP_ERROR_END = 0x005254D8-1;
	};

	struct SaveMSH
	{
		static const int ASM_INDEX_CAST = 0x0055704A;
		static const int ASM_INDEX_SIZE_PUSH = 0x0055704E-1;
	};

	struct LoadMSH
	{
		static const int ASM_INDEX_STRUCT_SIZE_ADD = 0x0055826F;
		static const int ASM_BLOCK_OFFSET_LEA_FIRST = 0x005583A2;
		static const int ASM_FEATINDEX_OFFSET = 0x00558226;
		static const int ASM_INDEX_CAST = 0x00558236;
	};

	struct zCArchiverFactory
	{
		static const int ReadLineArg = 0x00509A40;
		static const int WriteLine = 0x0050A420;
	};

	struct zCBspTree
	{
		static const int LoadBIN = 0x00525330;
	};

	struct zSTRING
	{	
		static const int ConstructorCharPtr = 0x004013A0;
		static const int ToChar = 0x0045E2E0;
	};

	struct zCParser
	{
		static const int inst_parser = 0x008DCE08;
		static const int getSymbol_STR = 0x006EA520;
	};

	struct zCParSymbol
	{
		static const int setValue_INT = 0x006F8530;
		static const int getValue_INT = 0x006F86E0;
	};

	struct oCWorld
	{
		static const int enterWorld = 0x0063EAD0;
	};

	struct oCAniCtrl_Human
	{
		static const int SetScriptValues = 0x0061CAF0;
	};

	struct oTGilValues
	{
		static const int instance = 0x008D8C50;
	};

	struct zCModel
	{
		static const int Render = 0x00560770;
		static const unsigned int Offset_ModelFatness = 0x118;
		static const unsigned int Offset_ModelScale = 0x11C;
	};
};

struct GothicMemoryLocations_Spacer
{
	struct SaveBin
	{
		static const int ASM_BSP_ERROR_START = 0x004CABA3;
		static const int ASM_BSP_ERROR_END = 0x004CAC5E-1;
	};

	struct LoadBin
	{
		static const int ASM_BSP_ERROR_START = 0x004CB1B8;
		static const int ASM_BSP_ERROR_END = 0x004CB259-1;
	};

	struct SaveMSH
	{
		static const int ASM_INDEX_CAST = 0x004FDF53;
		static const int ASM_INDEX_SIZE_PUSH = 0x004FDF57;
		static const int ASM_VERTEX_PROC_START = 0x004FCCEC;
		static const int ASM_VERTEX_PROC_END = 0x004FD4C1-1;
	};

	struct LoadMSH
	{
		static const int ASM_INDEX_STRUCT_SIZE_ADD = 0x004FEF3F;
		static const int ASM_BLOCK_OFFSET_LEA_FIRST = 0x004FF06B;
		static const int ASM_FEATINDEX_OFFSET = 0x004FEEFD;
		static const int ASM_INDEX_CAST = 0x004FEF10;
	};

	struct zCArchiverFactory
	{
		static const int ReadLineArg = 0x004AE740;
		static const int WriteLine = 0x004AF2E0;
		static const int WriteLine_char = 0x004AF3F0;
	};

	struct zCBspTree
	{
		static const int LoadBIN = 0x004CB080;
		static const int SaveBIN = 0x004CA220;
	};

	struct zSTRING
	{
		static const int ConstructorCharPtr = 0x004010C0;
		static const int ToChar = 0x005E2EB0;
	};

	// Don't need these in spacer
	struct zCParser
	{
		static const int inst_parser = 0;
		static const int getSymbol_STR = 0;
	};

	struct zCParSymbol
	{
		static const int setValue_INT = 0;
		static const int getValue_INT = 0;
	};

	struct oCWorld
	{
		static const int enterWorld = 0;
	};

	struct oTGilValues
	{
		static const int instance = 0;
	};

	struct zCModel
	{
		static const int Render = 0;
		static const unsigned int Offset_ModelFatness = 0x118;
		static const unsigned int Offset_ModelScale = 0x11C;
	};
};

class zSTRING
{
public:
	zSTRING(const char* str)
	{
		XCALL(GothicMemoryLocations::zSTRING::ConstructorCharPtr);
	}

	const char* ToChar() const
	{
		XCALL(GothicMemoryLocations::zSTRING::ToChar);
	}

	char data[20];
};
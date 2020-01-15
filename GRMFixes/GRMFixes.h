#pragma once

#include "stdafx.h"
#include "GothicMemoryLocations.h"

// Whether this acts as a plugin or not
#define USE_PLUGIN_API

/* Hook functions and patch the games memory */
void ApplyHooks();

struct float3
{
	float x, y, z;
};

struct float2
{
	float x, y, z;
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

class zCVob;
class zCVisual;
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

	zCVertex** vertList;
	zCPolygon** polyList;
	zCVertFeature** featList;

	zCVertex* vertArray;
	zCPolygon* polyArray;
	zCVertFeature* featArray;
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

typedef int zBOOL;
typedef unsigned char zBYTE;

enum zTMus_TransType
{
	zMUS_TR_DEFAULT = 0,
	zMUS_TR_NONE = 1,
	zMUS_TR_GROOVE = 2,
	zMUS_TR_FILL = 3,
	zMUS_TR_BREAK = 4,
	zMUS_TR_INTRO = 5,
	zMUS_TR_END = 6,
	zMUS_TR_ENDANDINTRO = 7
};

enum zTMus_TransSubType
{
	zMUS_TRSUB_DEFAULT = 0,
	zMUS_TRSUB_IMMEDIATE = 1,
	zMUS_TRSUB_BEAT = 2,
	zMUS_TRSUB_MEASURE = 3
};

struct zCMusicTheme
{
	int _vtbl;
	zSTRING fileName;
	float vol;
	zBOOL loop;
	float reverbMix;
	float reverbTime;
	zTMus_TransType trType;
	zTMus_TransSubType trSubType;
	zBYTE dScriptEnd;
	zSTRING name;
};

struct zCSoundFX
{
};

typedef int zTSoundHandle;

const int zSND_FREQ_DEFAULT = -1;
const float zSND_PAN_DEFAULT = -2;
const float zSND_PAN_LEFT = -1.0F;
const float zSND_PAN_CENTER = 0.0F;
const float zSND_PAN_RIGHT = 1.0F;
const float zSND_PAN_SURROUND = 100.0F;
const float zSND_VOL_DEFAULT = -1.0F;
const float zSND_RADIUS_DEFAULT = -1.0F;
const float zSND_PITCH_DEFAULT = -999999.0F;
const int zSND_SLOT_NONE = 0;
const int zSND_SLOT_MAX = 8;

enum FadeMode
{
	Out = 0,
	In = 1,
};

struct Transition
{
	std::string fileName;
	zTSoundHandle handle;
	FadeMode mode;
	int duration;
	float volume;
};
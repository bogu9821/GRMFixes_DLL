// dllmain.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
#include "stdafx.h"
#include "GothicMemoryLocations.h"
#include "GRMFixes.h"
#include "scriptAPI.h"

#define debugPrint printf

#ifdef GAME
#define DLL_TYPE_STR "Game"
#else
#define DLL_TYPE_STR "Spacer"
#endif

/* Map of adresses and original assemblercode bytes */
std::map<unsigned int, unsigned char> g_OriginalZENLoadSaveAsmBytes;
std::map<unsigned int, unsigned char>& g_ActiveOriginalAsmBytes = g_OriginalZENLoadSaveAsmBytes;

/* Level of Anisotropic Filtering */
int g_MaxAnisotropy = 0;

/* Whether we are in an XZEN right now */
bool g_IsLoadingXZEN = false;

std::unordered_map<std::string, int> g_vobMapTwoPass;
int g_vobRenderPass = 0;

/* Original forms of hooked functions */
typedef void (__thiscall* zCArchiverFactoryReadLineArg)(void*, zSTRING&, zSTRING&, struct zCBuffer*, struct zFILE*);
zCArchiverFactoryReadLineArg g_OriginalzCArchiverFactoryReadLineArg;

typedef void (__thiscall* zCArchiverFactoryWriteLine)(void*, const char*, struct zCBuffer*, struct zFILE*);
zCArchiverFactoryWriteLine g_OriginalzCArchiverFactoryWriteLine_char;

typedef int (__thiscall* zCBspTreeLoadBIN)(void*, class zCFileBIN &, int);
zCBspTreeLoadBIN g_OriginalzCBspTreeLoadBIN;

typedef int (__thiscall* zCBspTreeSaveBIN)(void*, class zCFileBIN &);
zCBspTreeSaveBIN g_OriginalzCBspTreeSaveBIN;

typedef void (__thiscall* oCAniCtrl_HumanSetScriptValues)(void*);
oCAniCtrl_HumanSetScriptValues g_OriginaloCAniCtrl_HumanSetScriptValues;

typedef int (__thiscall* zCModel_Render)(void*, struct zTRenderContext&);
zCModel_Render g_OriginalzCModel_Render;

typedef int (__thiscall* zCRND_D3DSetTextureStageState)(void*, DWORD, D3D7TEXTURESTAGESTATETYPE, DWORD);
zCRND_D3DSetTextureStageState g_OriginalzCRND_D3DSetTextureStageState;

typedef int(__thiscall* zCRND_D3DXD3D_SetRenderState)(void*, D3D7RENDERSTATETYPE, DWORD);
zCRND_D3DXD3D_SetRenderState g_OriginalzCRND_D3DXD3D_SetRenderState;

typedef int(__thiscall* zCTexture_HasAlpha)(void*);
zCTexture_HasAlpha g_OriginalzCTexture_HasAlpha;

typedef int(__fastcall* zCVob_Render)(void*, struct zTRenderContext&);
zCVob_Render g_OriginalzCVob_Render;

/* Restores the modified bytes from the given map to their original state */
void RestoreOriginalCodeBytes(const std::map<unsigned int, unsigned char>& originalMap)
{
	for each (const std::pair<unsigned int, unsigned char>& c in originalMap)
	{
		*(unsigned char *)c.first = c.second;
	}
}

void ReplaceCodeBytes(const char* bytes, int numBytes, unsigned int addr)
{
	// Unprotect the given code-area
	DWORD dwProtect;
	VirtualProtect((void *)addr, numBytes, PAGE_EXECUTE_READWRITE, &dwProtect);

	// Add original code to map
	auto it = g_ActiveOriginalAsmBytes.find(addr);
	if(it == g_ActiveOriginalAsmBytes.end())
	{
		for(int i = 0; i < numBytes; i++)
		{
			g_ActiveOriginalAsmBytes[addr + i] = *(unsigned char *)(addr + i);
		}
	}

	// Overwrite the code
	memcpy((void*)addr, bytes, numBytes);
}

void ReplaceCodeRange(unsigned char inst, unsigned int start, unsigned int end)
{
	// Unprotect the given code-area
	DWORD dwProtect;
	VirtualProtect((void *)start, end - start, PAGE_EXECUTE_READWRITE, &dwProtect);

	// Add original code to map
	auto it = g_ActiveOriginalAsmBytes.find(start);
	if(it == g_ActiveOriginalAsmBytes.end())
	{
		for(unsigned int i = start; i <= end; i++)
		{
			g_ActiveOriginalAsmBytes[i] = *(unsigned char *)(i);
		}
	}

	// Write to memory
	memset((void *)start, inst, (end - start) + 1);
}

void OpenDebugConsole()
{
	/*AllocConsole();

	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
	FILE* hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, NULL, _IONBF, 128);
	*stdin = *hf_in;*/
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
}

#ifndef GAME
void __stdcall FasterVertexProcessingHook(zCMesh* mesh, zCArray<zCVertex*>* vertexList, zCArray<zCVertFeature*>* featureList, int numPolys, zCPolygon** polyList)
{
	debugPrint("Saving Mesh...\n");

	DWORD startTime = timeGetTime();

	// [VERSION 6] Need to overwrite the poiter here with new memory, as this isn't always valid as it seems. 
	// It's a minor memory, leak, but since it's only for the spacer, I'm not going to bother
	vertexList->Array = new zCVertex*[1];
	featureList->Array = new zCVertFeature*[1];

	// Need to allocate at least *some* data or PushBack will fail.
	vertexList->Reallocate(1);
	featureList->Reallocate(1);

	if(mesh)
	{
		// Get vertices from mesh

		debugPrint("Got mesh data, generating vertexlist... (%d vertices) ", mesh->numVert);
		// Generate vertexlist
		for(int i=0;i<mesh->numVert;i++)
			vertexList->PushBackFast(mesh->vertList[i]);
		debugPrint("done.\n");

		// Create featurelist from polygons
		debugPrint("Generating featurelist from polygons... (%d polygons)\n", numPolys);		

		std::vector<zCVertFeature*> featureVec;
		for(int i = 0; i < numPolys; i++)
		{
			if((i%10000) == 0)
				debugPrint(" - %d polygons processed.\n", i);

			zCPolygon* p = polyList[i];

			// Iterate over poly-vertices
			for(int j = 0; j < p->PolyNumVert; j++)
			{
				// PB for each feature if it is already in that list. It's faster to add them all and then do a std::unique.
				featureVec.push_back(p->Features[j]);
			}
		}

		int numFeat = featureVec.size();
		debugPrint(" -- Got %d features\n", featureVec.size());

		// Remove doubles
		std::sort(featureVec.begin(),featureVec.end());
		std::unique(featureVec.begin(), featureVec.end());

		debugPrint(" -- Removed %d doubles\n", numFeat - featureVec.size());

		debugPrint(" -- Copying... ");

		// Copy to the games internal data structure
		featureList->FromVector(featureVec);

		debugPrint("done.\n");	
	}
	else
	{
		debugPrint("Getting vertexdata from shared polygons...\n");	

		// Get vertices from polygons
		std::vector<zCVertex*> vertexVec;
		std::vector<zCVertFeature*> featureVec;

		for(int i = 0; i < numPolys; i++)
		{
			if((i % 10000) == 0)
				debugPrint(" - %d polygons processed.\n", i);

			zCPolygon* p = polyList[i];

			// Iterate over poly-vertices
			for(int j = 0; j < p->PolyNumVert; j++)
			{
				// PB for each feature if it is already in that list. It's faster to add them all and then do a std::unique.
				vertexVec.push_back(p->Vertices[j]);
				featureVec.push_back(p->Features[j]);
			}
		}

		int numVerts = vertexVec.size();
		int numFeat = featureVec.size();
		debugPrint(" -- Got %d vertices\n", vertexVec.size());
		debugPrint(" -- Got %d features\n", featureVec.size());

		// Remove doubles
		std::sort(featureVec.begin(),featureVec.end());
		std::unique(featureVec.begin(), featureVec.end());

		std::sort(vertexVec.begin(),vertexVec.end());
		std::unique(vertexVec.begin(), vertexVec.end());

		debugPrint(" -- Removed %d double vertices and %d double features\n", numVerts - vertexVec.size(), numFeat - featureVec.size());

		debugPrint(" -- Copying... ");

		// Copy to the games internal data structure
		featureList->FromVector(featureVec);
		vertexList->FromVector(vertexVec);

		debugPrint("done.\n");	
	}

	float time = (timeGetTime() - startTime) / 1000.0f;
	debugPrint("Process took %.2f seconds. Going back to gamecode now.\n", time);
}

void FixVertexProcessing()
{
	/*	mov eax, [ebp+0xC] # PolyList
		push eax

		mov eax, [ebp+0x10] # NumPolys
		push eax

		lea eax, [ebp-0x7C] # FeatList
		push eax

		lea eax, [ebp-0x58] # VertList
		push eax

		mov eax, [ebp+0x14] # Mesh
		push eax

		call 0x10203040*/ 
	
	// Remove old vertex-collection
	ReplaceCodeRange(0x90, GothicMemoryLocations::SaveMSH::ASM_VERTEX_PROC_START, GothicMemoryLocations::SaveMSH::ASM_VERTEX_PROC_END);
	debugPrint(" - Removed old vertex-collection code\n");

	// Patch-code
	unsigned char hookCode[] = {	0x8B, 0x45, 0x0C, 0x50, 0x8B, 0x45, 0x10, 0x50, 0x8D, 
									0x45, 0x84, 0x50, 0x8D, 0x45, 0xA8, 0x50, 0x8B, 0x45, 0x14, 0x50, 
									0xFF, 0x15};

	// Patch the game
	ReplaceCodeBytes((char *)hookCode, ARRAYSIZE(hookCode), GothicMemoryLocations::SaveMSH::ASM_VERTEX_PROC_START);
	debugPrint(" - Inserted function call to new vertex-collection code\n");

	// Put our call-address into the code
	static UINT32 s_fnaddr = (UINT32)FasterVertexProcessingHook;
	UINT32 addr2 = (UINT32)&s_fnaddr;
	ReplaceCodeBytes((char *)&addr2, sizeof(UINT32), GothicMemoryLocations::SaveMSH::ASM_VERTEX_PROC_START + ARRAYSIZE(hookCode));
}
#endif

void FixZENLoadSave()
{
	if(!g_OriginalZENLoadSaveAsmBytes.empty())
		return; // Already patched

	// Indices are only 2 bytes in Gothic 1. Patch some bytes to bump that to 4.
#ifdef GAME
	debugPrint("Patching Game...\n");
#else
	debugPrint("Patching Spacer...\n");
#endif

	// SaveMSH: push 2 -> push 4  -- fwrite-size
	const char* push4 = "\x6A\x04";
	ReplaceCodeBytes(push4, strlen(push4), GothicMemoryLocations::SaveMSH::ASM_INDEX_SIZE_PUSH);
	debugPrint(" - push 2 -> push 4\n");

	// SaveMSH: mov ax, [edx+0Ch] -> mov eax, [edx+0x0C] -- Cast index to 16-bit
	// Also appended a 0x90 (nop) at the end, since the former instruction is only 3 bytes long.
	const char* cast32 = "\x8B\x42\x0C\x90";
	ReplaceCodeBytes(cast32, strlen(cast32), GothicMemoryLocations::SaveMSH::ASM_INDEX_CAST);
	debugPrint(" - mov ax, [edx+0Ch] -> mov eax, [edx+0x0C]\n");

#ifdef GAME
	// LoadMSH: add [esp+214h+var_1FC], 6 -> add [esp+214h+var_1FC], 8 -- Offset to the next vertex/feature-index pair. 
	// Feature-Indices are 4bytes, so with a 2byte vertexindex it gives 6. We need to make it 8.
	const char* offset8 = "\x83\x44\x24\x18\x08";
	ReplaceCodeBytes(offset8, strlen(offset8), GothicMemoryLocations::LoadMSH::ASM_INDEX_STRUCT_SIZE_ADD);
	debugPrint(" - add [esp+214h+var_1FC], 6 -> add [esp+214h+var_1FC], 8\n");

	// LoadMSH: movzx eax, word ptr [edx] -> mov eax, [edx] -- Remove the cast from 32 to 16-bit when getting the index out of the datastructure
	// Also add a NOP, since a regular mov is only 2 bytes instead of 3 for movzx.
	const char* loadCast = "\x8B\x02\x90";
	ReplaceCodeBytes(loadCast, strlen(loadCast), GothicMemoryLocations::LoadMSH::ASM_INDEX_CAST);
	debugPrint(" - movzx eax, word ptr [edx] -> mov eax, [edx]\n");

#else
	// LoadMSH: add ecx, 6 -> add ecx, 8 -- Offset to the next vertex/feature-index pair. 
	// Feature-Indices are 4bytes, so with a 2byte vertexindex it gives 6. We need to make it 8.
	const char* offset8 = "\x83\xC1\x08";
	ReplaceCodeBytes(offset8, strlen(offset8), GothicMemoryLocations::LoadMSH::ASM_INDEX_STRUCT_SIZE_ADD);
	debugPrint(" - add ecx, 6 -> add ecx, 8\n");

	// LoadMSH: mov dx, [eax] -> mov edx, [eax] -- Remove the cast from 32 to 16-bit when getting the index out of the datastructure
	// Also add a NOP, since a regular mov is only 2 bytes instead of 3 for mov to dx.
	const char* loadCast = "\x8B\x10\x90";
	ReplaceCodeBytes(loadCast, strlen(loadCast), GothicMemoryLocations::LoadMSH::ASM_INDEX_CAST);
	debugPrint(" - mov dx, [eax] -> mov edx, [eax]\n");

	// SaveBIN: Mute the annoying warning about a "too large" chunk
	ReplaceCodeRange(0x90, GothicMemoryLocations::SaveBin::ASM_BSP_ERROR_START, GothicMemoryLocations::SaveBin::ASM_BSP_ERROR_END);
	debugPrint(" - Muted \"Too large chunk\"-warning on ZEN-Save.\n");
#endif

	// LoadBIN: Mute the annoying warning about a "too large" chunk
	ReplaceCodeRange(0x90, GothicMemoryLocations::LoadBin::ASM_BSP_ERROR_START, GothicMemoryLocations::LoadBin::ASM_BSP_ERROR_END);
	debugPrint(" - Muted \"Too large chunk\"-warning on ZEN-Load.\n");

	// LoadMSH: mov esi, [ecx+2] -> mov esi, [ecx+4] -- Need to offset the feature by 4, not by 2
	const char* featOffset = "\x8B\x71\x04";
	ReplaceCodeBytes(featOffset, strlen(featOffset), GothicMemoryLocations::LoadMSH::ASM_FEATINDEX_OFFSET);
	debugPrint(" - mov esi, [ecx+2] -> mov esi, [ecx+4]\n");



	// LoadMSH: lea edx, [eax+eax*2] -> imul edx, eax, 4 -- In the next line, edx will be multiplyed by 2, so since edx = 3 * edx it becomes 6 * eax in the next line.
	// Replace this with code that multiplies eax by 4 so we get 8 with the next line.
	const char* imul = "\x6B\xD0\x04";
	ReplaceCodeBytes(imul, strlen(imul), GothicMemoryLocations::LoadMSH::ASM_BLOCK_OFFSET_LEA_FIRST);
	debugPrint(" - lea edx, [eax+eax*2] -> imul edx, eax, 4\n");

#ifndef GAME
	//FixVertexProcessing();
#endif

	debugPrint("Done!\n");
}

void AdjustMagicFrontier(const float* bytes, int numBytes, unsigned int addr)
{
	// Unprotect the given code-area
	DWORD dwProtect;
	VirtualProtect((void *)addr, numBytes, PAGE_EXECUTE_READWRITE, &dwProtect);

	// Write to memory
	memcpy((void*)addr, bytes, numBytes);
}

/* Leave a note in the ZEN-File so we know it has been modified data */
void __fastcall HookedzCArchiverFactoryWriteLine_char(void* thisptr, void* unknwn, const char* line, struct zCBuffer* buffer, struct zFILE* file)
{
	std::string ln = line;

	//debugPrint("Written: %s\n", ln.c_str());

	// Check if we are currently writing the "user" argument
	if(ln.substr(0, 4) == "user")
	{
		debugPrint("Writing modified 'user'-argument: XZEN\n");

		// Write our own user-line
		g_OriginalzCArchiverFactoryWriteLine_char(thisptr, "user XZEN", buffer, file);
		return;
	}
	
	// Just write the line
	g_OriginalzCArchiverFactoryWriteLine_char(thisptr, line, buffer, file);
}

/* Just proxy this to the char* version */
void __fastcall HookedzCArchiverFactoryWriteLine(void* thisptr, void* unknwn, const zSTRING& line, struct zCBuffer* buffer, struct zFILE* file)
{
	HookedzCArchiverFactoryWriteLine_char(thisptr, unknwn, line.ToChar(), buffer, file);	
}



/* Check for properties on top of a ZEN-File */
void __fastcall HookedzCArchiverFactoryReadLineArg(void* thisptr, void* unknwn, zSTRING& line, zSTRING& arg, struct zCBuffer* buffer, struct zFILE* file)
{
	// Call original function
	g_OriginalzCArchiverFactoryReadLineArg(thisptr, line, arg, buffer, file);

	// Check arg
	//debugPrint("Got ZEN-Line: %s", line.ToChar());
	//debugPrint(" - Arg: %s\n", arg.ToChar());

	std::string ln = line.ToChar();
	std::string ag = arg.ToChar();

	// We are using the "user" argument on top of each ZEN-File to determine if this is an enhanced ZEN.
	// That should be fine, since nobody really cares about that argument.
	if(ln == "user")
		g_IsLoadingXZEN = ag == "XZEN";
}

/** Only apply fixes while loading the worldmesh */
void __fastcall HookedzCBspTreeLoadBin(void* thisptr, void* edx, class zCFileBIN & a1, int a2)
{
	bool &isLoadingXZEN = g_IsLoadingXZEN;

	// Apply fixes if the file is right
	if(isLoadingXZEN)
	{
		debugPrint("Loading an enhanced ZEN. Applying code modifications.\n");
		FixZENLoadSave();
	}

	// Call game function
	g_OriginalzCBspTreeLoadBIN(thisptr, a1, a2);

	// Reset everything, worldmesh is done
	if(isLoadingXZEN)
	{
		isLoadingXZEN = false;

		debugPrint("Done loading Mesh! Restoring original code... \n");

		// Go back to normal at the start
		RestoreOriginalCodeBytes(g_OriginalZENLoadSaveAsmBytes);

		// Reset this for the next time
		g_OriginalZENLoadSaveAsmBytes.clear();
	}
}

/** Only apply fixes while saving the worldmesh */
void __fastcall HookedzCBspTreeSaveBin(void* thisptr, void* edx, class zCFileBIN & a1)
{
	// Apply fixes if we're saving a world
	debugPrint("Saving an enhanced ZEN. Applying code modifications.\n");
	FixZENLoadSave();
	
	// Call game function
	g_OriginalzCBspTreeSaveBIN(thisptr, a1);

	// Go back to default code
	debugPrint("Done saving Mesh! Restoring original code... \n");

	// Go back to normal at the start
	RestoreOriginalCodeBytes(g_OriginalZENLoadSaveAsmBytes);

	// Reset this for the next time
	g_OriginalZENLoadSaveAsmBytes.clear();
}

/**
 * Fix the climbing-angle
 * This is called more than once, but meh. Safety!
 */
void __fastcall HookedoCAniCtrl_HumanSetScriptValues(void* thisptr, void* edx)
{
	// Call this first, to initialize the instance
	g_OriginaloCAniCtrl_HumanSetScriptValues(thisptr);

	//debugPrint("Patching climbing-angle\n");

	
	const int GIL_HUMAN = 1;
	Daedalus::oTGilValues* gil = Daedalus::oTGilValues::get();
	gil->CLIMB_GROUND_ANGLE[GIL_HUMAN] = 50;
	gil->CLIMB_HEADING_ANGLE[GIL_HUMAN] = 50;
	gil->CLIMB_HORIZ_ANGLE[GIL_HUMAN] = 50;

	//gil->SURFACE_ALIGN[GIL_HUMAN] = 1;

	// Call another time, to set our new values
	g_OriginaloCAniCtrl_HumanSetScriptValues(thisptr);
}

/**
 * Workaround for the new finger-models breaking on some fatness values
 */
void __fastcall HookedzCModel_Render(void* thisptr, void* edx, struct zTRenderContext& ctx)
{
	// Get fatness value and save it
	float* pFatness = (float*)(((char*)thisptr) + GothicMemoryLocations::zCModel::Offset_ModelFatness);

	// Set fatness value accordingly
	if (*pFatness >= 2.0f)
		*pFatness = 0.50f;
	else if (*pFatness == 1.0f)
		*pFatness = 0.25f;
	else if (*pFatness <= -1.0f)
		*pFatness = -0.25f;

	// Continue rendering...
	g_OriginalzCModel_Render(thisptr, ctx);
}

int __fastcall HookedzCRND_D3DSetTextureStageState(void* thisptr, void* edx, DWORD stage, D3D7TEXTURESTAGESTATETYPE state, DWORD value)
{
	if (state == D3D7TSS_MAGFILTER)
	{
		g_OriginalzCRND_D3DSetTextureStageState(thisptr, stage, state, D3D7TFG_ANISOTROPIC);
		g_OriginalzCRND_D3DSetTextureStageState(thisptr, stage, D3D7TSS_MAXANISOTROPY, g_MaxAnisotropy);

		return S_OK;
	}
	else if(state == D3D7TSS_MINFILTER)
	{
		g_OriginalzCRND_D3DSetTextureStageState(thisptr, stage, state, D3D7TFN_ANISOTROPIC);
		g_OriginalzCRND_D3DSetTextureStageState(thisptr, stage, D3D7TSS_MAXANISOTROPY, g_MaxAnisotropy);

		return S_OK;
	}
	else
	{
		return g_OriginalzCRND_D3DSetTextureStageState(thisptr, stage, state, value);
	}	
}

int __fastcall HookedzCRND_D3DXD3D_SetRenderState(void* thisptr, void* edx, D3D7RENDERSTATETYPE state, DWORD value)
{
	g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, state, value);

	// Two Pass Rendering Technique (https://blogs.msdn.microsoft.com/shawnhar/2009/02/18/depth-sorting-alpha-blended-objects/)
	if (g_vobRenderPass == 1)
	{
		g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, D3D7RENDERSTATE_ALPHATESTENABLE, true);
		g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, D3D7RENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
		g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, D3D7RENDERSTATE_ALPHAREF, 160);
		g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, D3D7RENDERSTATE_ALPHABLENDENABLE, false);
		g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, D3D7RENDERSTATE_ZENABLE, true);
	}
	else if (g_vobRenderPass == 2)
	{
		g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, D3D7RENDERSTATE_ALPHATESTENABLE, true);
		g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, D3D7RENDERSTATE_ALPHAFUNC, D3DCMP_LESS);
		g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, D3D7RENDERSTATE_ALPHAREF, 160);
		g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, D3D7RENDERSTATE_ALPHABLENDENABLE, true);
		g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, D3D7RENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, D3D7RENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
		g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, D3D7RENDERSTATE_ZENABLE, true);
		g_OriginalzCRND_D3DXD3D_SetRenderState(thisptr, D3D7RENDERSTATE_ZWRITEENABLE, false);
	}

	return S_OK;
}

int __fastcall HookedzCTexture_HasAlpha(void* thisptr, void* edx)
{
	if (g_vobRenderPass > 0)
		return 1;
	else
		return g_OriginalzCTexture_HasAlpha(thisptr);
}

void __fastcall HookedzCVob_Render(void* thisptr, struct zTRenderContext& ctx)
{
	g_vobRenderPass = 1;
	g_OriginalzCVob_Render(thisptr, ctx);

	if (g_vobMapTwoPass.empty())
	{
		g_vobRenderPass = 2;
		g_OriginalzCVob_Render(thisptr, ctx);
	}
	else
	{
		zCVob* vob = (zCVob*)thisptr;
		zCVisual* visual = *(zCVisual**)((char*)vob + GothicMemoryLocations::zCVob::Offset_Visual);

		if (visual)
		{
			zSTRING* zstr = (zSTRING*)((char*)visual + GothicMemoryLocations::zCObject::Offset_Name);
			if (zstr)
			{
				if (g_vobMapTwoPass.find(zstr->ToChar()) != g_vobMapTwoPass.end())
				{
					g_vobRenderPass = 2;
					g_OriginalzCVob_Render(thisptr, ctx);
				}
			}
		}
	}

	g_vobRenderPass = 0;
}

/* Hook functions */
void ApplyHooks()
{
	debugPrint("-------- GRM-Fix-Collection by Degenerated - Version 8 for " DLL_TYPE_STR " --------\n");
	debugPrint("Applying hook to 'zCArchiverFactory::ReadLineArg'\n");
	g_OriginalzCArchiverFactoryReadLineArg = (zCArchiverFactoryReadLineArg)DetourFunction((byte*)GothicMemoryLocations::zCArchiverFactory::ReadLineArg, (byte*)HookedzCArchiverFactoryReadLineArg);

	if(g_OriginalzCArchiverFactoryReadLineArg)
		debugPrint(" - Success!\n");
	else
		debugPrint(" - Failure!\n");

	debugPrint("Applying hook to 'zCBspTree::LoadBIN'\n");
	g_OriginalzCBspTreeLoadBIN =  (zCBspTreeLoadBIN)DetourFunction((byte*)GothicMemoryLocations::zCBspTree::LoadBIN, (byte*)HookedzCBspTreeLoadBin);
	if(g_OriginalzCBspTreeLoadBIN)
		debugPrint(" - Success!\n");
	else
		debugPrint(" - Failure!\n");

	// Only hook the write-ones in the spacer. Savegames shouldn't have that flag as they don't contain the worldmesh
#ifndef GAME

	debugPrint("Applying hook to 'zCBspTree::SaveBIN'\n");
	g_OriginalzCBspTreeSaveBIN = (zCBspTreeSaveBIN)DetourFunction((byte*)GothicMemoryLocations::zCBspTree::SaveBIN, (byte*)HookedzCBspTreeSaveBin);
	if (g_OriginalzCBspTreeSaveBIN)
		debugPrint(" - Success!\n");
	else
		debugPrint(" - Failure!\n");

	debugPrint("Applying hook to 'zCArchiverFactory::WriteLine'\n");

	// Just overwrite the zSTRING-Version and proxy to the char* overload
	DetourFunction((byte*)GothicMemoryLocations::zCArchiverFactory::WriteLine, (byte*)HookedzCArchiverFactoryWriteLine);
	g_OriginalzCArchiverFactoryWriteLine_char = (zCArchiverFactoryWriteLine)DetourFunction((byte*)GothicMemoryLocations::zCArchiverFactory::WriteLine_char, (byte*)HookedzCArchiverFactoryWriteLine_char);

	if (g_OriginalzCArchiverFactoryWriteLine_char)
		debugPrint(" - Success!\n");
	else
		debugPrint(" - Failure!\n");
#else

	debugPrint("Applying hook to 'zCRND_D3D::SetTextureStageState'\n");
	g_OriginalzCRND_D3DSetTextureStageState =  (zCRND_D3DSetTextureStageState)DetourFunction((byte*)GothicMemoryLocations::zCRND_D3D::SetTextureStageState, (byte*)HookedzCRND_D3DSetTextureStageState);
	if(g_OriginalzCRND_D3DSetTextureStageState)
		debugPrint(" - Success!\n");
	else
		debugPrint(" - Failure!\n");

	debugPrint("Applying hook to 'zCRND_D3D::XD3D_SetRenderState'\n");
	g_OriginalzCRND_D3DXD3D_SetRenderState = (zCRND_D3DXD3D_SetRenderState)DetourFunction((byte*)GothicMemoryLocations::zCRND_D3D::XD3D_SetRenderState, (byte*)HookedzCRND_D3DXD3D_SetRenderState);
	if(g_OriginalzCRND_D3DXD3D_SetRenderState)
		debugPrint(" - Success!\n");
	else
		debugPrint(" - Failure!\n");

	debugPrint("Applying hook to 'zCTexture::HasAlpha'\n");
	g_OriginalzCTexture_HasAlpha = (zCTexture_HasAlpha)DetourFunction((byte*)GothicMemoryLocations::zCTexture::HasAlpha, (byte*)HookedzCTexture_HasAlpha);
	if (g_OriginalzCTexture_HasAlpha)
		debugPrint(" - Success!\n");
	else
		debugPrint(" - Failure!\n");

	debugPrint("Applying hook to 'zCVob::Render'\n");
	g_OriginalzCVob_Render = (zCVob_Render)DetourFunction((byte*)GothicMemoryLocations::zCVob::Render, (byte*)HookedzCVob_Render);
	if (g_OriginalzCVob_Render)
		debugPrint(" - Success!\n");
	else
		debugPrint(" - Failure!\n");

	debugPrint("Applying hook to 'oCAniCtrl_Human::SetScriptValues'\n");
	g_OriginaloCAniCtrl_HumanSetScriptValues =  (oCAniCtrl_HumanSetScriptValues)DetourFunction((byte*)GothicMemoryLocations::oCAniCtrl_Human::SetScriptValues, (byte*)HookedoCAniCtrl_HumanSetScriptValues);
	if(g_OriginaloCAniCtrl_HumanSetScriptValues)
		debugPrint(" - Success!\n");
	else
		debugPrint(" - Failure!\n");

	// Get path to CGP mod volume
	char exe[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], mod[MAX_PATH];
	GetModuleFileName(NULL, exe, MAX_PATH);
	_splitpath(exe, drive, dir, NULL, NULL);
	strcpy_s(mod, drive);
	strcat_s(mod, dir);
	strcat_s(mod, "..\\Data\\Carnage_Graphics_Patch.mod");
	_fullpath(mod, mod, MAX_PATH);

	// Is CGP installed?
	if (GetFileAttributes(mod) != INVALID_FILE_ATTRIBUTES)
	{
		debugPrint("Applying hook to 'zCModel::Render'\n");
		g_OriginalzCModel_Render = (zCModel_Render)DetourFunction((byte*)GothicMemoryLocations::zCModel::Render, (byte*)HookedzCModel_Render);
		if (g_OriginalzCModel_Render)
			debugPrint(" - Success!\n");
		else
			debugPrint(" - Failure!\n");
	}

	// Read settings from INI
	char ini[MAX_PATH];
	strcpy_s(ini, drive);
	strcat_s(ini, dir);
	strcat_s(ini, "Gothic_Reloaded_Mod.ini");
	g_MaxAnisotropy  = GetPrivateProfileInt("OVERRIDES", "ENGINE.zAnisotropicFiltering", 0, ini);

	float pointsWorld[38][2] = { { 57939.2f, 1280.28f }, { 55954.4f, 5421.51f }, { 52856.8f, 10047.0f }, { 49451.9f, 14908.2f }, { 44199.8f, 20513.3f }, { 37684.2f, 26271.2f }, { 30434.0f, 31462.4f }, { 25573.6f, 32692.7f }, { 21248.3f, 35176.1f }, { 19450.7f, 35205.0f }, { 16263.1f, 32799.6f }, { 10755.6f, 34744.4f }, { 9736.9f, 37990.5f }, { 8218.6f, 38393.1f }, { 4065.0f, 39018.4f }, { 839.9f, 39079.3f }, { -9312.9f, 38694.2f }, { -26219.2f, 40844.2f }, { -34576.0f, 43032.3f }, { -44458.8f, 43099.2f }, { -49763.7f, 37384.8f }, { -54137.3f, 26761.7f }, { -62089.3f, 21598.1f }, { -66193.7f, 12999.2f }, { -66132.3f, 6204.0f }, { -63855.2f, -5700.8f }, { -59385.1f, -10081.5f }, { -56013.8f, -22393.4f }, { -47250.3f, -28502.0f }, { -37136.5f, -38319.2f }, { -24664.7f, -46687.9f }, { -7860.6f, -48966.6f }, { 4876.6f, -49691.0f }, { 23147.8f, -47875.1f }, { 48722.3f, -39488.8f }, { 55902.4f, -31909.8f }, { 61238.6f, -23412.8f }, { 60230.1f, -6641.9f } };
	debugPrint("Adjusting magic frontier...\n");
	AdjustMagicFrontier((float *)pointsWorld, sizeof(pointsWorld), GothicMemoryLocations::oCMagFrontier::pointsWorld);
	debugPrint("Done!\n");

	// Read alpha VOB names from txt
	char alpha[MAX_PATH];
	strcpy_s(alpha, drive);
	strcat_s(alpha, dir);
	strcat_s(alpha, "AlphaTwoPass.txt");

	debugPrint("Reading alpha VOB list...\n");
	std::ifstream file(alpha);
	std::string line;
	while (std::getline(file, line))
		g_vobMapTwoPass[line] = 1;
	if (g_vobMapTwoPass.empty())
		debugPrint("File not found or empty, two pass rendering will be applied to all VOBs!\n");
	else
		debugPrint("Done!\n");
#endif
}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		OpenDebugConsole();

#ifndef USE_PLUGIN_API
		ApplyHooks();
#endif
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


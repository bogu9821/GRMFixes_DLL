#pragma once
#include "stdafx.h"
#include "VersionCheck.h"
#include <Windows.h>
#include <ImageHlp.h>

#pragma comment(lib, "Imagehlp.lib")

namespace VersionCheck
{
	/** Checks if the exe that loaded this DLL matches the given type */
	bool CheckExecutable(EGothicVersion version)
	{
		DWORD checksum;
		DWORD headersum;
		char file[1024];

		GetModuleFileNameA(NULL, file, 1024);
		
		// Get checksum from header
		if(MapFileAndCheckSum(file, &headersum, &checksum) != 0)
		{
			printf("Failed to get checksum of %s\n", file);
			return false;
		}

		//printf("Checksums of %s: Header: 0x%08x Checksum: 0x%08x\n", file, headersum, checksum);

		return checksum == version;
	}
};
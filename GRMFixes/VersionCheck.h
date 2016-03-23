#pragma once

namespace VersionCheck
{
	enum EGothicVersion
	{
		GV_Gothic_1_08k_mod = 0x008246d2,
		GV_Gothic_1_11f = 0x0085d9c3,
		GV_Spacer_1_41 = 0x0091de75,
		GV_Spacer_1_5 = -1,
		GV_Gothic_2_6_fix = 0x008a3e89,
		GV_Spacer_2_6_fix = 0x00c54fbb
	};

	/** Checks if the exe that loaded this DLL matches the given type */
	bool CheckExecutable(EGothicVersion version);
};
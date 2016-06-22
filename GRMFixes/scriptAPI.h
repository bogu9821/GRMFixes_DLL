#pragma once
#include "mem.h"
#include "GothicMemoryLocations.h"

namespace Daedalus
{
	struct oTGilValues
	{
		enum {NPC_GIL_MAX = 42};
		int WATER_DEPTH_KNEE[NPC_GIL_MAX];
		int WATER_DEPTH_CHEST[NPC_GIL_MAX];
		int JUMPUP_HEIGHT[NPC_GIL_MAX];
		int SWIM_TIME[NPC_GIL_MAX];
		int DIVE_TIME[NPC_GIL_MAX];
		int STEP_HEIGHT[NPC_GIL_MAX];
		int JUMPLOW_HEIGHT[NPC_GIL_MAX];
		int JUMPMID_HEIGHT[NPC_GIL_MAX];
		int SLIDE_ANGLE1[NPC_GIL_MAX];
		int SLIDE_ANGLE2[NPC_GIL_MAX];
		int DISABLE_AUTOROLL[NPC_GIL_MAX];
		int SURFACE_ALIGN[NPC_GIL_MAX];
		int CLIMB_HEADING_ANGLE[NPC_GIL_MAX];
		int CLIMB_HORIZ_ANGLE[NPC_GIL_MAX];
		int CLIMB_GROUND_ANGLE[NPC_GIL_MAX];
		int FIGHT_RANGE_BASE[NPC_GIL_MAX];
		int FIGHT_RANGE_FIST[NPC_GIL_MAX];
		int FIGHT_RANGE_1HS[NPC_GIL_MAX];
		int FIGHT_RANGE_1HA[NPC_GIL_MAX];
		int FIGHT_RANGE_2HS[NPC_GIL_MAX];
		int FIGHT_RANGE_2HA[NPC_GIL_MAX];
		int FALLDOWN_HEIGHT[NPC_GIL_MAX];
		int FALLDOWN_DAMAGE_PERM[NPC_GIL_MAX];
		int BLOOD_DISABLED[NPC_GIL_MAX];
		int BLOOD_MAX_DISTANCE[NPC_GIL_MAX];
		int BLOOD_AMOUNT[NPC_GIL_MAX];
		int BLOOD_FLOW[NPC_GIL_MAX];
		zSTRING BLOOD_EMITTER[NPC_GIL_MAX];
		zSTRING BLOOD_TEXTURE[NPC_GIL_MAX];
		int TURN_SPEED[NPC_GIL_MAX];

		static oTGilValues* get()
		{
			return (oTGilValues*)GothicMemoryLocations::oTGilValues::instance;
		}
	};

	struct zCParSymbol
	{
		/**
		 * Sets the value held by this symbol at the given array-index
		 */
		void setValue(int value, int index)
		{
			XCALL(GothicMemoryLocations::zCParSymbol::setValue_INT);
		}

		void getValue(int& value, int index)
		{
			XCALL(GothicMemoryLocations::zCParSymbol::getValue_INT);
		}
	};

	class zCParser
	{
	public:
		static zCParser* getParser()
		{
			return (zCParser*)GothicMemoryLocations::zCParser::inst_parser;
		}

		/**
		 * @return Symbol of the given name. (case insensitive)
		 */
		zCParSymbol* getSymbol(const char* sym)
		{
			return __getSymbol(zSTRING(sym));
		}

	private:
		zCParSymbol* __getSymbol(const zSTRING& str)
		{
			XCALL(GothicMemoryLocations::zCParser::getSymbol_STR);
		}
	};
}
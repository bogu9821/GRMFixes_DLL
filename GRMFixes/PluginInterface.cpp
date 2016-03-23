#include "stdafx.h"
#include <PluginInterface.h>
#include "GRMFixes.h"
#include "VersionCheck.h"

#ifdef GAME
#define GAME_VERSION VersionCheck::EGothicVersion::GV_Gothic_1_08k_mod
#else
#define GAME_VERSION VersionCheck::EGothicVersion::GV_Spacer_1_41
#endif

namespace GPlugin
{
	class GRMFixesPlugin : public IPlugin
	{
	public:
		/** Called right after the game started */
		virtual bool OnStartup()
		{
#ifdef USE_PLUGIN_API
			if(!VersionCheck::CheckExecutable(GAME_VERSION))
				return false; // Wrong version, exit out and unload

			ApplyHooks();
			return true;
#else
			return false;
#endif
		}

		/** Called right before the game closes */
		virtual bool OnShutdown()
		{
			return true;
		}

	private:
	};

	/** Creates an instance of the plugin */
	GPLUGIN_API GPlugin::IPlugin* InitPlugin()
	{
		return new GRMFixesPlugin;
	}

	/** Deletes the instance of the plugin */
	GPLUGIN_API void ClosePlugin(GPlugin::IPlugin* plugin)
	{
		delete plugin;
	}

	/** Returns the API-Version of this plugins interface */
	GPLUGIN_API APIVersion GetAPIVersion()
	{
		return PLUGIN_API_VERSION;
	}
};
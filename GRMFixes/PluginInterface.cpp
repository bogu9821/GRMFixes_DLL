#include "stdafx.h"
#include "PluginInterface.h"
#include "GRMFixes.h"
#include "VersionCheck.h"

namespace GPlugin
{
	class GRMFixesPlugin : public IPlugin
	{
	public:
		/** Called right after the game started */
		virtual bool OnStartup()
		{
#ifndef USE_PLUGIN_API
			return false;
#endif

#ifdef GAME
			if (!VersionCheck::CheckExecutable(VersionCheck::EGothicVersion::GV_Gothic_1_08k_mod) && !VersionCheck::CheckExecutable(VersionCheck::EGothicVersion::GV_Gothic_1_08k_fps))
				return false;

			std::string cmnd = GetCommandLine();
			std::transform(cmnd.begin(), cmnd.end(), cmnd.begin(), ::toupper);

			if (cmnd.find(TEXT("-GAME:GOTHIC_RELOADED_MOD.INI")) == std::string::npos)
				return false;
#else
			if (!VersionCheck::CheckExecutable(VersionCheck::EGothicVersion::GV_Spacer_1_41))
				return false;
#endif

			ApplyHooks();
			return true;
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
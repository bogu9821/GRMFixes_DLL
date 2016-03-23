#pragma once

/** Current API-Version. Modify only on API-Change! */
#define PLUGIN_API_VERSION 1

#ifdef COMPILE_LOADER_
#define GPLUGIN_API __declspec(dllimport)
#else
#define GPLUGIN_API __declspec(dllexport)
#endif

namespace GPlugin
{
	typedef unsigned int APIVersion;

	class IDirect3DDevice7;
	class IPlugin
	{
	public:
		/** Called right after the game started. 
			Return false to cancel the loading process and to free
			the library again. */
		virtual bool OnStartup(){return true;}

		/** Called right before the game closes */
		virtual bool OnShutdown(){return true;}
	};

	/** Functiondeclerations for the core-functions */
	typedef IPlugin* (__stdcall* InitPlugin_t)();
	typedef void (__stdcall* ClosePlugin_t)(IPlugin*);

	extern "C" {
		/** Returns the API-Version of this plugins interface */
		GPLUGIN_API APIVersion GetAPIVersion();

		/** Creates an instance of the plugin */
		GPLUGIN_API IPlugin* InitPlugin();

		/** Deletes the instance of the plugin */
		GPLUGIN_API void ClosePlugin(IPlugin* plugin);
	};

}
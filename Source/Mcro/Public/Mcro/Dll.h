/** @noop License Comment
 *  @file
 *  @copyright
 *  This Source Code is subject to the terms of the Mozilla Public License, v2.0.
 *  If a copy of the MPL was not distributed with this file You can obtain one at
 *  https://mozilla.org/MPL/2.0/
 *  
 *  @author David Mórász
 *  @date 2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Mcro/Modules.h"
#include "Interfaces/IPluginManager.h"

#include "Mcro/Concepts.h"

namespace Mcro::Dll
{
	using namespace Mcro::Concepts;
	using namespace Mcro::Modules;
	
	/** @brief RAII wrapper around PushDllDirectory / PopDllDirectory */
	struct MCRO_API FScopedSearchPath
	{
		FScopedSearchPath(FString const& path);
		~FScopedSearchPath();
		
	private:
		FString Path;
	};
	
	/** @brief RAII wrapper around GetDllHandle / FreeDllHandle */
	struct MCRO_API FScopedDll
	{
		FScopedDll(const TCHAR* fileName);
		~FScopedDll();
		
	private:
		void* Handle;
	};

	/** @brief Handle multiple DLL files in one set and an optional base path for them */
	struct MCRO_API FScopedDllSet
	{
		FScopedDllSet() {}

		/**
		 *	@param pushPath  The absolute path to be pushed for the basis of finding given DLL's 
		 *	@param dllFiles  The list of DLL file names
		 */
		template <CConvertibleTo<const TCHAR*>... DllFiles>
		FScopedDllSet(FString const& pushPath, DllFiles... dllFiles)
		{
			FScopedSearchPath pathContext(pushPath);
			(Dlls.Emplace(dllFiles), ...);
		}

		/**
		 *	@param   plugin  The plugin this set of DLL's will be relative to 
		 *	@param pushPath  The relative path to the plugin base where to find given DLL's
		 *	@param dllFiles  The list of DLL file names
		 */
		template <CConvertibleTo<const TCHAR*>... DllFiles>
		FScopedDllSet(TSharedPtr<IPlugin> plugin, FString const& pushPath, DllFiles... dllFiles)
		{
			ASSERT_CRASH(plugin);
			FString absPushPath = plugin->GetBaseDir() / pushPath;
			FScopedSearchPath pathContext(absPushPath);
			(Dlls.Emplace(dllFiles), ...);
		}
		
	private:

		TArray<FScopedDll> Dlls;
	};

	/**
	 *	@brief  List DLL's which is used by a specific module and its owning plugin.
	 *
	 *	Goes well with `McroBuild.ModuleRuleExtensions.UseRuntimeDependencies` or
	 *	`McroBuild.ModuleRuleExtensions.PrepareRuntimeDependencies` like so:
	 *	@code
	 *	TModuleBoundDlls<FMyAwesomeModule> GMyLibraryDlls {MYLIBRARY_DLL_PATH, MYLIBRARY_DLL_FILES};
	 *	@endcode
	 *	Where `MyLibrary` is an external module type importing a third-party library with this snippet contained in its
	 *	rules:
	 *	@code
	 *	this.PrepareRuntimeDependencies(this.ModulePath() / "lib" / "x64");
	 *	@endcode
	 */
	template <CObservableModule M>
	struct TModuleBoundDlls : TModuleBoundObject<M, FScopedDllSet>
	{
		using ModuleBoundObject = TModuleBoundObject<M, FScopedDllSet>;
		
		template <CConvertibleTo<const TCHAR*>... DllFiles>
		TModuleBoundDlls(const TCHAR* pushPath, DllFiles... dllFiles)
			: ModuleBoundObject(
			{
				[pushPath, dllFiles...]
				{
					auto thisPlugin = IPluginManager::Get().GetModuleOwnerPlugin(*InferModuleName<M>());
					return new FScopedDllSet(thisPlugin, pushPath, dllFiles...);
				}
			})
		{}
	};
}

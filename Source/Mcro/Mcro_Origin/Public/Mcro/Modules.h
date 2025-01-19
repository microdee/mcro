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
#include "Mcro/TypeName.h"
#include "Mcro/Delegates/EventDelegate.h"

/** @brief Namespace for utilities handling Unreal modules */
namespace Mcro::Modules
{
	using namespace Mcro::Delegates;
	using namespace Mcro::Concepts;
	using namespace Mcro::TypeName;

	/** @brief Add this interface to your module class if other things can listen to module startup or shutdown */
	class MCRO_API IObservableModule : public IModuleInterface
	{
	public:
		
		/**
		 *	@brief
		 *	Event broadcasted on module startup or immediately executed upon subscription if module has already been
		 *	started up.
		 */
		TBelatedEventDelegate<void()> OnStartupModule;
		
		/**
		 *	@brief
		 *	Event broadcasted on module shutdown or immediately executed upon subscription if module has already been
		 *	shut down.
		 */
		TBelatedEventDelegate<void()> OnShutdownModule;

		virtual void StartupModule() override;
		virtual void ShutdownModule() override;
	};

	/** @brief Use this in global variables to automatically do things on module startup or shutdown */
	template <CDerivedFrom<IObservableModule> M>
	struct TObserveModule
	{
		/**
		 *	@brief
		 *	Default constructor will try to infer module name from type name. Given convention
		 *	`(F|I)Foobar(Module(Interface)?)?` the extracted name will be `Foobar`. If your module doesn't follow this
		 *	naming use the constructor accepting an FName
		 */
		TObserveModule()
		{
			auto moduleName = TTypeString<M>.Mid(1);
			moduleName.RemoveFromEnd(TEXT("Module"));
			moduleName.RemoveFromEnd(TEXT("ModuleInterface"));
			ObserveModule(moduleName);
		}

		/** @brief This constructor provides an explicit FName for getting the module */
		TObserveModule(FName const& moduleName)
		{
			ObserveModule(moduleName);
		}

		/**
		 *	@brief
		 *	Event broadcasted on module startup or immediately executed upon subscription if module has already been
		 *	started up.
		 */
		TBelatedEventDelegate<void()> OnStartupModule;

		/**
		 *	@brief
		 *	Event broadcasted on module shutdown or immediately executed upon subscription if module has already been
		 *	shut down.
		 */
		TBelatedEventDelegate<void()> OnShutdownModule;

		/** @brief Specify function to be executed on startup */
		TObserveModule& OnStartup(TFunction<void()>&& func)
		{
			OnStartupModule.Add(InferDelegate::From(func));
			return *this;
		}
		
		/** @brief Specify function to be executed on shutdown */
		TObserveModule& OnShutdown(TFunction<void()>&& func)
		{
			OnShutdownModule.Add(InferDelegate::From(func));
			return *this;
		}
		
	private:
		M* Module = nullptr;

		void ObserveModule(FName const& moduleName)
		{
			decltype(auto) manager = FModuleManager::Get();
			M* module = static_cast<M*>(manager.GetModule(moduleName));
			if (!module)
			{
				manager.OnModulesChanged().AddLambda([this, moduleName](FName name, EModuleChangeReason changeReason)
				{
					if (changeReason == EModuleChangeReason::ModuleLoaded && moduleName == name)
						ObserveModule(moduleName);
				});
			}
			else
			{
				Module = module;
				module->OnStartupModule.Add(OnStartupModule.Delegation());
				module->OnShutdownModule.Add(OnShutdownModule.Delegation());
			}
		}
	};
}

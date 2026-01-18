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
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Mcro/AssertMacros.h"
#include "Mcro/TextMacros.h"
#include "Mcro/TypeName.h"
#include "Mcro/Delegates/EventDelegate.h"
#include "Mcro/Error.h"
#include "Mcro/Enums.h"

/** @brief Namespace for utilities handling Unreal modules */
namespace Mcro::Modules
{
	using namespace Mcro::Delegates;
	using namespace Mcro::Concepts;
	using namespace Mcro::TypeName;
	using namespace Mcro::Error;
	using namespace Mcro::Enums;

	/**
	 *	@brief
	 *	Infer the module name from an input type. This exists because there's a very reliable naming convention for
	 *	class names representing modules. The inference is removing the first letter Hungarian type notation and
	 *	removing "Module" or "ModuleInterface" from the end.
	 *	
	 *	@tparam M  the supposed type of the module
	 *	@return    The module name inferrable from its type name
	 */
	template <CDerivedFrom<IModuleInterface> M>
	FString InferModuleName()
	{
		auto moduleName = TTypeString<M>().Mid(1);
		moduleName.RemoveFromEnd(TEXT_"Module");
		moduleName.RemoveFromEnd(TEXT_"ModuleInterface");
		return moduleName;
	}

	/**
	 *	@brief
	 *	Try to load a module and return an IError when that fails for any reason. Module name is inferred from type
	 *	described in @see InferModuleName.
	 */
	template <CDerivedFrom<IModuleInterface> M>
	TMaybe<M*> TryLoadUnrealModule()
	{
		auto loadResult = EModuleLoadResult::Success;
		auto name = InferModuleName<M>();
		auto moduleInterface = FModuleManager::Get().LoadModuleWithFailureReason(*name, loadResult);
		ASSERT_RETURN(loadResult == EModuleLoadResult::Success && moduleInterface)
			->AsFatal()
			->WithMessageF(TEXT_"Couldn't load module {0} inferred from type {1}",
				name, TTypeName<M>
			)
			->WithAppendix(TEXT_"EModuleLoadResult", EnumToStringCopy(loadResult));
		return static_cast<M*>(moduleInterface);
	}

	/**
	 *	@brief
	 *	Load a module or crash with IError if it cannot be done for any reason Module name is inferred from type
	 *	described in @see InferModuleName.
	 */
	template <CDerivedFrom<IModuleInterface> M>
	M& LoadUnrealModule()
	{
		auto result = TryLoadUnrealModule<M>();
		ASSERT_CRASH(result,
			->WithError(result.GetErrorRef())
		);
		return *result.GetValue();
	}

	/** @brief Shorthand for FModuleManager::GetModulePtr with the name inferred from given type. @see InferModuleName */
	template <CDerivedFrom<IModuleInterface> M>
	M* GetUnrealModulePtr()
	{
		return FModuleManager::GetModulePtr<M>(*InferModuleName<M>());
	}

	/** @brief Get an already loaded unreal module. If for any reason it's not loaded, crash the app. @see InferModuleName */
	template <CDerivedFrom<IModuleInterface> M>
	M& GetUnrealModule()
	{
		M* result = FModuleManager::GetModulePtr<M>(*InferModuleName<M>());
		ASSERT_CRASH(result,
			->WithMessageF(TEXT_"Couldn't get module {0} inferred from type {1}",
				InferModuleName<M>(),
				TTypeName<M>
			)
		);
		return *result;
	}

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

	template <typename T>
	concept CObservableModule = CDerivedFrom<T, IModuleInterface>
		&& requires(T&& t)
		{
			{ t.OnStartupModule } -> CSameAsDecayed<TBelatedEventDelegate<void()>>;
			{ t.OnShutdownModule } -> CSameAsDecayed<TBelatedEventDelegate<void()>>;
		}
	;

	/** @brief A record for the module event listeners */
	struct MCRO_API FObserveModuleListener
	{
		TFunction<void()> OnStartup;
		TFunction<void()> OnShutdown;
	};

	/** @brief Use this in global variables to automatically do things on module startup or shutdown */
	template <CObservableModule M>
	struct TObserveModule
	{
		/**
		 *	@brief
		 *	Default constructor will try to infer module name from type name. Given convention
		 *	`(F|I)Foobar(Module(Interface)?)?` the extracted name will be `Foobar`. If your module doesn't follow this
		 *	naming use the constructor accepting an FName
		 */
		TObserveModule(FObserveModuleListener&& listeners)
		{
			BindListeners(FWD(listeners));
			
			ObserveModule(*InferModuleName<M>());
			
		}

		/** @brief This constructor provides an explicit FName for getting the module */
		TObserveModule(FName const& moduleName, FObserveModuleListener&& listeners)
		{
			BindListeners(FWD(listeners));
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

		void BindListeners(FObserveModuleListener&& listeners)
		{
			if (listeners.OnStartup) OnStartupModule.Add(InferDelegate::From(listeners.OnStartup));
			if (listeners.OnShutdown) OnShutdownModule.Add(InferDelegate::From(listeners.OnShutdown));
		}

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

	/** @brief A wrapper around a given object which lifespan is bound to given module. */
	template <CObservableModule M, typename T>
	struct TModuleBoundObject
	{
		using StorageType = std::conditional_t<
			CSharedFromThis<T>,
			TSharedPtr<T>,
			TUniquePtr<T>
		>;
		
		struct FObjectFactory
		{
			TFunction<T*()> Create;
			TFunction<void(T&)> OnAfterCreated;
			TFunction<void(T&)> OnShutdown;
		};

		TModuleBoundObject(FObjectFactory&& factory = {})
			: Observer({
				[this, factory]
				{
					T* newObject = !factory.Create ? new T() : factory.Create();
					CreateObject(newObject);
					if (factory.OnAfterCreated) factory.OnAfterCreated(*Storage.Get());
				},
				[this, factory]
				{
					if (factory.OnShutdown) factory.OnShutdown(*Storage.Get());
					Storage.Reset();
				}
			})
		{}

		T& GetChecked()
		{
			ASSERT_CRASH(Storage,
				->WithMessage(TEXT_"Module bound object was not available")
				->WithAppendix(TEXT_"Module type", TTypeString<M>())
				->WithAppendix(TEXT_"Object type", TTypeString<T>())
			);
			return *Storage.Get();
		}

		T const& GetChecked() const
		{
			ASSERT_CRASH(Storage,
				->WithMessage(TEXT_"Module bound object was not available")
				->WithAppendix(TEXT_"Module type", TTypeString<M>())
				->WithAppendix(TEXT_"Object type", TTypeString<T>())
			);
			return *Storage.Get();
		}
		
		      T* TryGet()       { return Storage.Get(); }
		const T* TryGet() const { return Storage.Get(); }

	private:

		void CreateObject(T* newObject)
		{
			if constexpr (CSharedInitializeable<T>)
				Storage = MakeShareableInit(newObject);
			else if constexpr (CSharedFromThis<T>)
				Storage = MakeShareable<T>(newObject);
			else
				Storage = TUniquePtr<T>(newObject);
		}
		
		StorageType Storage {};
		TObserveModule<M> Observer;
	};
}

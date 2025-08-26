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
#include "Features/IModularFeatures.h"
#include "Async/Future.h"
#include "Mcro/TypeName.h"
#include "Mcro/Delegates/EventDelegate.h"

DECLARE_LOG_CATEGORY_CLASS(LogAutoModularFeature, Log, Log);

namespace Mcro::AutoModularFeature
{
	using namespace Mcro::Delegates;
	using namespace Mcro::TypeName;

	/** @brief Tagging an auto feature (DO NOT USE MANUALLY, inherited by TAutoModularFeature) */
	class IAutoModularFeature {};
	
	/** @brief Tagging an implementation of a feature */
	class IFeatureImplementation {};
	
	/**
	 *	@brief
	 *	Auto Modular Features are a workflow with Modular Features where the developer doesn't have to rely on string
	 *	identifiers. TAutoModularFeature template take care of naming the feature and introduces some common
	 *	functionality, like getter functions and runtime validations.
	 *
	 *	First a feature is defined with its interface class like so:
	 *	@code
	 *	class IMyModularFeature : public TAutoModularFeature<IMyModularFeature>
	 *	{
	 *		// ...
	 *	}
	 *	@endcode
	 *	Then each implementation of this feature are defined like so:
	 *	@code
	 *	class FMyFeatureImplementation : public IMyModularFeature, public IFeatureImplementation
	 *	{
	 *		FMyFeatureImplementation()
	 *		{
	 *			// See the inline docs for why this needs to be done
	 *			Register()
	 *		}
	 *	}
	 *	@endcode
	 *	Then instantiate the feature implementation when needed:
	 *	@code
	 *	class FMyModule
	 *	{
	 *		TPimplPtr<FMyFeatureImplementation> MyImplementation;
	 *	}
	 *
	 *	if (...)
	 *	{
	 *		MyImplementation = MakePimpl<FMyFeatureImplementation>();
	 *	}
	 *	@endcode
	 *	To access the feature implementations then just use
	 *	@code
	 *	if (IMyModularFeature::ImplementationCount() > 0)
	 *	{
	 *		IMyModularFeature::Get().MyStuff();
	 *	}
	 *	@endcode
	 *	
	 *	Internally the feature name will be identical to the class name. In this case `IMyModularFeature` will register
	 *	as "IMyModularFeature". Technically one can get it via
	 *	@code
	 *	IModularFeatures::Get().GetModularFeature<IMyModularFeature>(TEXT_"IMyModularFeature")
	 *	@endcode
	 *	but it is strongly discouraged for type safety and keeping code simple.
	 *
	 *	For globally available features you may use just simply a global variable, or if it's important to have the
	 *	owning module fully initialized, `TModuleBoundObject` is recommended.
	 *	@code
	 *	TModuleBoundObject<FFoobarModule, FMyFeatureImplementation> GMyFeatureImplementation;
	 *	@endcode 
	 *	
	 *	@remarks
	 *	IMyModularFeature::FeatureName() and TTypeFName<FMyFeatureImplementation>() can be used for runtime
	 *	comparison / validation.
	 *
	 *	@tparam FeatureIn  Curiously Recurring Template argument of the feature
	 */
	template<typename FeatureIn>
	class TAutoModularFeature : public IAutoModularFeature, public IModularFeature
	{
	public:
		using Feature = FeatureIn;
		using AutoModularFeature = TAutoModularFeature;

		/** @brief This event is triggered when an implementation of this feature is created */
		static auto OnRegistered() -> TBelatedEventDelegate<void(Feature*)>&
		{
			static TBelatedEventDelegate<void(Feature*)> event;
			return event;
		}

		/** @brief Get the name of the feature */
		static FORCEINLINE FName FeatureName()
		{
			return TTypeFName<Feature>();
		}

		/** @return The number of implementations created for this feature */
		static FORCEINLINE int32 ImplementationCount()
		{
			return IModularFeatures::Get().GetModularFeatureImplementationCount(FeatureName());
		}

		/**
		 *	@brief
		 *	Get the first existing implementation of this feature. If there are no implementations a check will fail.
		 */
		static FORCEINLINE Feature& Get()
		{
			return IModularFeatures::Get().GetModularFeature<Feature>(FeatureName());
		}
		
		/** @brief Get the first existing implementation of this feature. Return nullptr If there are no implementations. */
		static FORCEINLINE Feature* TryGet(const int32 index)
		{
			return static_cast<Feature*>(IModularFeatures::Get().GetModularFeatureImplementation(FeatureName(), index));
		}

		/** @return An array of all implementations of this feature */
		static FORCEINLINE TArray<Feature*> GetAll()
		{
			return IModularFeatures::Get().GetModularFeatureImplementations<Feature>(FeatureName());
		}

		/**
		 *	@brief
		 *	Call this function in implementation constructors.
		 *
		 *	This is a necessary boilerplate to maintain polymorphism of implementations. Otherwise, if the native
		 *	registration function would be called directly in TAutoModularFeature default constructor, virtual function
		 *	overrides are not yet known, and "deducing this" is not meant for constructors.
		 *	
		 *	@tparam Implementation  Derived type of the implementation
		 *	@param            self  Pointer to implementation registering itself
		 */
		template<typename Implementation> requires CDerivedFrom<Implementation, Feature>
		void Register(this Implementation&& self)
		{
			UE_LOG(
				LogAutoModularFeature, Log,
				TEXT_"Registering %s as %s feature",
				*TTypeString<Implementation>(),
				*TTypeString<Feature>()
			);
			IModularFeatures::Get().RegisterModularFeature(FeatureName(), &self);
			OnRegistered().Broadcast(&self);
		}
		
		virtual ~TAutoModularFeature()
		{
			IModularFeatures::Get().UnregisterModularFeature(FeatureName(), this);
		}

		/**
		 *	@brief
		 *	Get the first implementation once it is registered, or return the first implementation immediately if
		 *	there's already one registered.
		 *	
		 *	@return  A future completed when the first implementation becomes available, or there's already one
		 *
		 *	@warning
		 *	If no implementations were created throughout the entire execution of the program, it will crash on exit
		 *	with an unfulfilled promise error. When using this function either handle this scenario early, or
		 *	use `OnRegistered()` if your particular feature may never be created.
		 */
		static FORCEINLINE TFuture<Feature*> GetBelated()
		{
			// Shared promise is required as delegate lambdas are copyable
			TSharedRef<TPromise<Feature*>> promise = MakeShared<TPromise<Feature*>>();
			TFuture<Feature*> result = promise->GetFuture();

			OnRegistered().Add(InferDelegate::From([promise](Feature* feature) mutable
			{
				promise->SetValue(feature);
			}), {.Once = true});
			
			return result;
		}
	};
}

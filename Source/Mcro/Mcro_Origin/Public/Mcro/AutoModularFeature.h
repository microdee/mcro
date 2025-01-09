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
#include "Once.h"
#include "Async/Future.h"
#include "Mcro/TypeName.h"
#include "Mcro/Concepts.h"

DECLARE_LOG_CATEGORY_CLASS(LogAutoModularFeature, Log, Log);

namespace Mcro::AutoModularFeature
{
	using namespace Mcro::Concepts;
	using namespace Mcro::TypeName;

	/** Tagging an auto feature (DO NOT USE MANUALLY, inherited by TAutoModularFeature) */
	class IAutoModularFeature {};
	/** Tagging an implementation of a feature */
	class IFeatureImplementation {};
	
	/**
	 *	Auto Modular Features are a workflow with Modular Features where the developer doesn't have to rely on string
	 *	identifiers. TAutoModularFeature and TFeatureImplementation templates take care of naming the feature and
	 *	introduces some common functionality, like getter functions and runtime validations.
	 *
	 *	@remarks
	 *	First a feature is defined with its interface class like so:
	 *	@code
	 *	class IMyModularFeature : public TAutoModularFeature<IMyModularFeature>
	 *	{
	 *		// ...
	 *	}
	 *	@endcode
	 *	Then each implementations of this feature are defined like so:
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
	 *	Internally the feature name will be identical to the class name. In this case IMyModularFeature will register
	 *	as "IMyModularFeature". Technically one can get it via
	 *	@code
	 *	IModularFeatures::Get().GetModularFeature<IMyModularFeature>(TEXT("IMyModularFeature"))
	 *	@endcode
	 *	but it is strongly discouraged for type safety and keeping code simple.
	 *	
	 *	@remarks
	 *	IMyModularFeature::FeatureName() and TTypeName<FMyFeatureImplementation>() can be used for runtime
	 *	comparison / validation. See TFeatureImplementation::CastChecked which helps handling implementation specific
	 *	structures and their runtime polymorphism.
	 *
	 *	@tparam FeatureIn Curiously Recurring Template argument of the feature
	 */
	template<typename FeatureIn>
	class TAutoModularFeature : public IAutoModularFeature, public IModularFeature
	{
	public:
		using Feature = FeatureIn;
		using AutoModularFeature = TAutoModularFeature;

		/** Gert the name of the feature */
		static FORCEINLINE const FName& FeatureName()
		{
			return TTypeFName<Feature>;
		}

		/**
		 *	@return The number of implementations created for this feature
		 */
		static FORCEINLINE int32 ImplementationCount()
		{
			return IModularFeatures::Get().GetModularFeatureImplementationCount(FeatureName());
		}

		/**
		 *	Get the first existing implementation of this feature. If there are no implementations a check will fail.
		 */
		static FORCEINLINE Feature& Get()
		{
			return IModularFeatures::Get().GetModularFeature<Feature>(FeatureName());
		}
		
		/**
		 *	Get the first existing implementation of this feature. Return nullptr If there are no implementations.
		 */
		static FORCEINLINE Feature* TryGet(const int32 index)
		{
			return static_cast<Feature*>(IModularFeatures::Get().GetModularFeatureImplementation(FeatureName(), index));
		}

		/**
		 *	@return An array of all implementations of this feature
		 */
		static FORCEINLINE TArray<Feature*> GetAll()
		{
			return IModularFeatures::Get().GetModularFeatureImplementations<Feature>(FeatureName());
		}

		/**
		 *	Call this function in implementation constructors. This is a necessary boilerplate to maintain polymorphism
		 *	of implementations. Otherwise, if the native registration function would be called directly in
		 *	TAutoModularFeature default constructor, virtual function overrides are not yet known, and "deducing this"
		 *	is not meant for constructors.
		 *	
		 *	@tparam Implementation Derived type of the implementation
		 *	@param self Pointer to implementation registering itself
		 */
		template<typename Implementation> requires CDerivedFrom<Implementation, Feature>
		void Register(this Implementation&& self)
		{
			UE_LOG(
				LogAutoModularFeature, Log,
				TEXT("Registering %s as %s feature"),
				*TTypeString<Implementation>,
				*TTypeString<Feature>
			);
			IModularFeatures::Get().RegisterModularFeature(FeatureName(), &self);
		}
		
		virtual ~TAutoModularFeature()
		{
			IModularFeatures::Get().UnregisterModularFeature(FeatureName(), this);
		}

		/**
		 *	Get the first implementation once it is registered, or return the first implementation immediately if
		 *	there's already one registered.
		 *	
		 *	@return  A future completed when the first implementation becomes available, or there's already one
		 */
		static FORCEINLINE TFuture<Feature*> GetBelated()
		{
			using namespace Mcro::Once;

			if (ImplementationCount())
			{
				return MakeFulfilledPromise<Feature*>(&Get()).GetFuture();
			}

			// Shared promise is required as delegate lambdas are copyable
			TSharedRef<TPromise<Feature*>> promise = MakeShared<TPromise<Feature*>>();
			TFuture<Feature*> result = promise->GetFuture();
			IModularFeatures::Get().OnModularFeatureRegistered().AddLambda(
				[promise, once = FOnce()](const FName& type, IModularFeature*) mutable 
				{
					if (type == FeatureName() && once)
						promise->SetValue(&Get());
				}
			);
			return result;
		}
	};
}

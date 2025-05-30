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
#include "Mcro/FunctionTraits.h"

namespace Mcro::UObjects::Init
{
	using namespace Mcro::FunctionTraits;

	template <typename T, typename... Args>
	concept CUObjectInitializable = CUObject<T> && requires(T* t, Args... args)
	{
		t->Initialize(args...);
	};

	/**
	 *	@brief
	 *	Mirror of `FStaticConstructObjectParameters` but it's a plain C++ object and doesn't have a constructor so
	 *	designated initialization with named members is possible
	 */
	struct MCRO_API FConstructObjectParameters
	{
		/**
		 *	@brief
		 *	The object to create this object within (the Outer property for the new object will be set to the value
		 *	specified here).
		 */
		UObject* Outer = (UObject*)GetTransientPackage();
		
		/** @brief The class of the object to create */
		const UClass* Class;

		/** @brief
		 *	The name to give the new object. If no value (`NAME_None`) is specified, the object will be given a unique
		 *	name in the form of `ClassName_#`.
		 */
		FName Name = NAME_None;

		/**
		 *	@brief
		 *	The ObjectFlags to assign to the new object. some flags can affect the behavior of constructing the object.
		 */
		EObjectFlags Flags = RF_NoFlags;

		/**
		 *	@brief The `InternalObjectFlags` to assign to the new object. Some flags can affect the behavior of
		 *	constructing the object.
		 */
		EInternalObjectFlags InternalSetFlags = EInternalObjectFlags::None;

		/**
		 *	@brief
		 *	If true, copy transient from the class defaults instead of the pass in archetype ptr (often these are the same)
		 */
		bool bCopyTransientsFromClassDefaults = false;

		/** @brief If true, Template is guaranteed to be an archetype */
		bool bAssumeTemplateIsArchetype = false;

		/**
		 *	@brief 
		 *	If specified, the property values from this object will be copied to the new object, and the new object's
		 *	`ObjectArchetype` value will be set to this object. If nullptr, the class default object is used instead.
		 */
		UObject* Template = nullptr;

		/** @brief Contains the mappings of instanced objects and components to their templates */
		FObjectInstancingGraph* InstanceGraph = nullptr;

		/** @brief Assign an external Package to the created object if non-null */
		UPackage* ExternalPackage = nullptr;

		/** @brief Callback for custom code to initialize properties before `PostInitProperties` runs */
		TFunction<void()> PropertyInitCallback;
	};

	namespace Detail
	{
		template <CUObject T, typename... Args>
		void InitObject(T* object, Args&&... args) {}
		
		template <CUObject T, typename... Args>
		requires CUObjectInitializable<T, Args...>
		void InitObject(T* object, Args&&... args)
		{
			object->Initialize(Forward<Args>(args)...);
		}
	}
	
	/**
	 *	@brief
	 *	Create a new object which can also be initialized with an `Initialize` function if it has one.
	 *	In case it has an `Initialize` function the `args` parameters should match them. This is an equivalent to
	 *	the template Mcro::SharedObjects::MakeShareableInit
	 *	
	 *	@tparam       T  Type of initializable UObject
	 *	@tparam    Args  Arguments for the Initialize function
	 *	@param   params  Parameters for every new object
	 *	@param     args  Arguments for the Initialize function
	 *	@return  The new object
	 */
	template <CUObject T, typename... Args>
	T* NewInit(FConstructObjectParameters&& params, Args&&... args)
	{
		T* result = NewObject<T>(
			params.Outer,
			params.Class ? params.Class : T::StaticClass(),
			params.Name,
			params.Flags,
			params.Template,
			params.bCopyTransientsFromClassDefaults,
			params.InstanceGraph,
			params.ExternalPackage
		);
		Detail::InitObject(result, Forward<Args>(args)...);
		return result;
	}

	/**
	 *	@brief
	 *	Equivalent to Mcro::Construct::Construct but for UObjects.
	 *	
	 *	Usage:
	 *	@code
	 *	using namespace Mcro::UObjects::Init;
	 *	
	 *	auto myObject = Construct({}, [](UMyObject& _)
	 *	{
	 *		_.Foo = 42;
	 *		_.Bar();
	 *		// etc...
	 *	});
	 *	static_assert(std::is_same_v<decltype(myObject), UMyObject*>);
	 *	@endcode
	 *
	 *	Notice how the object type is deduced from the argument of the initializer.
	 *	
	 *	@tparam  Initializer  Initializer function type
	 *	@param        params  Parameters for every new object
	 *	@param          init  A setup function for the newly created UObject
	 *	@return  The new object
	 */
	template <
		CFunctorObject Initializer,
		typename TArg = TFunction_Arg<Initializer, 0>,
		CUObject T = std::decay_t<TArg>
	>
	requires std::is_lvalue_reference_v<TArg>
	T* Construct(FConstructObjectParameters&& params, Initializer&& init)
	{
		T* result = NewObject<T>(
			params.Outer,
			params.Class ? params.Class : T::StaticClass(),
			params.Name,
			params.Flags,
			params.Template,
			params.bCopyTransientsFromClassDefaults,
			params.InstanceGraph,
			params.ExternalPackage
		);
		init(*result);
		return result;
	}

	/**
	 *	@brief
	 *	Equivalent to Mcro::Construct::Construct but for `UObjects`. If the constructed `UObject` type also has an
	 *	`Initialize` function call that too after the lambda initializer. The `args` parameters should match the
	 *	signature of `Initialize` in that case.
	 *	
	 *	Usage:
	 *	@code
	 *	using namespace Mcro::UObjects::Init;
	 *	
	 *	auto myObject = Construct({}, [](UMyObject& _)
	 *	{
	 *		_.Foo = 42;
	 *		_.Bar();
	 *		// etc...
	 *	});
	 *	static_assert(std::is_same_v<decltype(myObject), UMyObject*>);
	 *	@endcode
	 *
	 *	Notice how the object type is deduced from the argument of the initializer.
	 *	
	 *	@tparam  Initializer  Initializer function type
	 *	@tparam         Args  Arguments for the Initialize function
	 *	@param        params  Parameters for every new object
	 *	@param          init  A setup function for the newly created UObject
	 *	@param          args  Arguments for the Initialize function
	 *	@return  The new object
	 */
	template <
		CFunctorObject Initializer,
		typename... Args,
		typename TArg = TFunction_Arg<Initializer, 0>,
		CUObject T = std::decay_t<TArg>
	>
	requires std::is_lvalue_reference_v<TArg>
	T* ConstructInit(FConstructObjectParameters&& params, Initializer&& init, Args&&... args)
	{
		T* result = NewObject<T>(
			params.Outer,
			params.Class ? params.Class : T::StaticClass(),
			params.Name,
			params.Flags,
			params.Template,
			params.bCopyTransientsFromClassDefaults,
			params.InstanceGraph,
			params.ExternalPackage
		);
		init(*result);
		Detail::InitObject(result, Forward<Args>(args)...);
		return result;
	}
}

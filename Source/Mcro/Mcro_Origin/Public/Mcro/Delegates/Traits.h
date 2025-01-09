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
#include "Mcro/Void.h"

namespace Mcro::Delegates
{
	using namespace Mcro::FunctionTraits;

	/** Constraint given type to a dynamic delegate class */
	template <typename T>
	concept CDynamicDelegate = 
		requires() { typename std::decay_t<T>::ThreadSafetyMode; }
		&& CDerivedFrom<std::decay_t<T>, TScriptDelegate<typename std::decay_t<T>::ThreadSafetyMode>>
	;

	/** Constraint given type to a dynamic multicast delegate class */
	template <typename T>
	concept CDynamicMulticastDelegate =
		requires() { typename std::decay_t<T>::ThreadSafetyMode; }
		&& CDerivedFrom<std::decay_t<T>, TMulticastScriptDelegate<typename std::decay_t<T>::ThreadSafetyMode>>
	;
	
	template <typename>
	struct TDynamicMethodPtr_Struct {};

	/** Get the native function pointer type compatible with given dynamic delegate */
	template <CDynamicDelegate Dynamic>
	struct TDynamicMethodPtr_Struct<Dynamic>
	{
		using Type = typename Dynamic::template TMethodPtrResolver<FDeclareOnly>::FMethodPtr;
	};

	/** Get the native function pointer type compatible with given dynamic multicast delegate */
	template <CDynamicMulticastDelegate Dynamic>
	struct TDynamicMethodPtr_Struct<Dynamic>
	{
		using Type = typename TDynamicMethodPtr_Struct<typename Dynamic::FDelegate>::Type;
	};

	/** Get the native function pointer type compatible with given dynamic (multicast) delegate */
	template <typename Dynamic>
	using TDynamicMethodPtr = typename TDynamicMethodPtr_Struct<std::decay_t<Dynamic>>::Type;
	
	template <typename>
	struct TDynamicSignature_Struct {};

	/** Get the native function signature type compatible with given dynamic delegate */
	template <CDynamicDelegate Dynamic>
	struct TDynamicSignature_Struct<Dynamic>
	{
		using Type = TFunction_Signature<TDynamicMethodPtr<Dynamic>>;
	};

	/** Get the native function signature type compatible with given dynamic multicast delegate */
	template <CDynamicMulticastDelegate Dynamic>
	struct TDynamicSignature_Struct<Dynamic>
	{
		using Type = typename TDynamicSignature_Struct<typename Dynamic::FDelegate>::Type;
	};

	/** Get the native function signature type compatible with given dynamic (multicast) delegate */
	template <typename Dynamic>
	using TDynamicSignature = typename TDynamicSignature_Struct<std::decay_t<Dynamic>>::Type;
	
	template <typename>
	struct TNative_Struct {};
	
	/** Map the input dynamic delegate to a conceptually compatible native delegate type */
	template <CDynamicDelegate Dynamic>
	struct TNative_Struct<Dynamic>
	{
		using Type = TDelegate<TDynamicSignature<Dynamic>, FDefaultDelegateUserPolicy>;
	};

	/** Map the input dynamic multicast delegate to a conceptually compatible native multicast delegate type */
	template <CDynamicMulticastDelegate Dynamic>
	struct TNative_Struct<Dynamic>
	{
		using Type = TMulticastDelegate<TDynamicSignature<Dynamic>, FDefaultDelegateUserPolicy>;
	};
	
	/** Map the input dynamic (multicast) delegate to a conceptually compatible native (multicast) delegate type */
	template<typename Dynamic>
	using TNative = typename TNative_Struct<std::decay_t<Dynamic>>::Type;
}

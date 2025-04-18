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
#include "Mcro/Delegates/Traits.h"

namespace Mcro::Delegates
{
	using namespace Mcro::FunctionTraits;
	
	/**
	 *	@brief
	 *	Creates a native delegate that is bound to the same UFunction as the specified dynamic delegate.
	 *	
	 *	@note This function can only convert non-multicast delegates.
	 *	
	 *	The signatures of the functions are checked, so that it is not possible to compile a conversion that would crash
	 *	at runtime.
	 *	
	 *	Example Usage:
	 *
	 *	@code
	 *	using FMyNativeDelegate = TDelegate<void(int32 someParam)>;
	 *	DECLARE_DYNAMIC_DELEGATE_OneParam(FMyBlueprintDelegate, int32, someParam);
	 *	
	 *	void MyNativeDelegateFunction(FMyNativeDelegate delegate)
	 *	{
	 *		...
	 *	}
	 *	
	 *	void MyBlueprintDelegateFunction(FMyBlueprintDelegate delegate)
	 *	{
	 *		MyNativeDelegateFunction(Delegates::AsNative(delegate));
	 *	}
	 *	@endcode
	 *	
	 *	@tparam            Dynamic  The origin type, i.e. the dynamic delegate. Will be auto-deduced.
	 *	@tparam NativeDelegateType  The target type, i.e. the delegate that you want to produce. Will be auto-deduced from Dynamic.
	 *	@param     dynamicDelegate  The dynamic delegate that will be converted
	 */
	template <
		CDynamicDelegate Dynamic,
		typename MethodPtrTypeDynamic = TDynamicMethodPtr<Dynamic>,
		typename NativeDelegateType = TDelegate<TFunction_Signature<MethodPtrTypeDynamic>>,
		typename MethodPtrTypeNative = typename TMemFunPtrType<
			false,
			FDeclareOnly,
			typename NativeDelegateType::TFuncType
		>::Type
	>
	requires CSameAsDecayed<MethodPtrTypeDynamic, MethodPtrTypeNative>
	NativeDelegateType AsNative(Dynamic&& dynamicDelegate)
	{
		// The TBaseUFunctionDelegateInstance constructor asserts if the function name is NAME_None. We therefore must check
		// to see if the dynamic delegate is bound at all. If it isn't, return an unbound native delegate.
		if (!dynamicDelegate.IsBound())
		{
			return NativeDelegateType();
		}

		return NativeDelegateType::CreateUFunction(dynamicDelegate.GetUObject(), dynamicDelegate.GetFunctionName());
	}
}

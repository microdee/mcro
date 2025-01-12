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
#include "McroWindows/Error/WindowsError.h"
#include "Microsoft/COMPointer.h"

namespace Mcro::Windows::COM
{
	using namespace Mcro::Windows::Error;
	
	template <typename From, typename To>
	FCanFail ComCast(From* from, TComPtr<To>& to, bool fastError = false)
	{
		HRESULT hr = from->QueryInterface(IID_PPV_ARGS(&to));
		if (UNLIKELY(hr != S_OK))
			return IError::Make(new FHresultError(hr, fastError))
				->AsRecoverable()
				->WithMessageF(
					TEXT("Object of type %s did not implement %s"),
					*TTypeString<From>,
					*TTypeString<To>
				);
		return Success();
	}
	
	template <typename From, typename To>
	FCanFail ComCast(TComPtr<From> const& from, TComPtr<To>& to, bool fastError = false)
	{
		return ComCast(from.Get(), to, fastError);
	}
	
	template <typename From, typename To>
	TMaybe<TComPtr<To>> ComCast(From* from, bool fastError = false)
	{
		TComPtr<To> to;
		ComCast(from, to, fastError);
		return MoveTemp(to);
	}
	
	template <typename From, typename To>
	TMaybe<TComPtr<To>> ComCast(TComPtr<From> const& from, bool fastError = false)
	{
		return ComCast(from.Get(), fastError);
	}
}

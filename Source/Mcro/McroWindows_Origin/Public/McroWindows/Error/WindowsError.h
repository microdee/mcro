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
#include "Mcro/Error.h"

namespace Mcro::Windows::Error
{
	using namespace Mcro::Error;

	/**
	 *	An error wrapping the returned code of GetLastError and attempts to get a string description of it
	 */
	MCROWINDOWS_API class FLastError : public IError
	{
	public:
		FLastError(int32 errorCode);

		/** The result code wrapped by this error */
		int32 ErrorCode;
		
		/** The message what the Windows API communicates to us */
		FString SystemMessage;
	};

	/**
	 *	An error wrapping HRESULT code returned by many Microsoft APIs. It will also collect human readable metadata.
	 */
	MCROWINDOWS_API class FHresultError : public  IError
	{
	public:
		/**
		 *	@param result  The input HRESULT
		 *	
		 *	@param fastMode
		 *	Set it true to not gather human readable information about the error and just display the HRESULT code as is.
		 *	Use it only in cursed situations where something may fail very often but it still needs a full IError
		 *	somehow. Or just avoid such cursed situations in the first place.
		 */
		FHresultError(HRESULT result, bool fastMode = false);

		void SetHumanReadable();

		/** The result code wrapped by this error */
		HRESULT Result;

		/** The message what the Windows API communicates to us */
		FString SystemMessage;

		/** Stores the language-dependent programmatic ID (ProgID) for the class or application that raised the error. */
		FString ProgramID;

		/** A textual description of the error (might be different from Message) */
		FString Description;
	};
}

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

#include "McroWindows/Error/WindowsError.h"
#include "Windows/WindowsPlatformMisc.h"

#include "Mcro/LibraryIncludes/Start.h"
#include "comdef.h"
#include "Mcro/LibraryIncludes/End.h"

namespace Mcro::Windows::Error
{
	FLastError::FLastError(int32 errorCode) : ErrorCode(errorCode)
	{
		TCHAR errorTextBuffer[2048];
		FWindowsPlatformMisc::GetSystemErrorMessage(errorTextBuffer, 2048, errorCode);
		SystemMessage = errorTextBuffer;
		IError::AddAppendix(TEXT("SystemMessage"), SystemMessage);
		IError::AddAppendix(TEXT("ErrorCode"), FString::FromInt(errorCode));
	}

	FHresultError::FHresultError(HRESULT result, bool fastMode) : Result(result)
	{
		if (fastMode)
		{
			SystemMessage = FString::FromInt(result);
			IError::AddAppendix(TEXT("SystemMessage"), SystemMessage);
		}
		else SetHumanReadable();
	}

	void FHresultError::SetHumanReadable()
	{
		_com_error comError(Result);
		SystemMessage = comError.ErrorMessage();
		ProgramID = comError.Source();
		Description = comError.Description();
		IError::AddAppendix(TEXT("SystemMessage"), SystemMessage);
		IError::AddAppendix(TEXT("Description"), Description);
		IError::AddAppendix(TEXT("ProgramID"), ProgramID);
		IError::AddAppendix(TEXT("ErrorCode"), FString::FromInt(Result));
	}
}

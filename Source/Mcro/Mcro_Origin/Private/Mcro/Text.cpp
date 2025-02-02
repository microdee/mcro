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

#include "Mcro/Text.h"
#include "Mcro/FunctionTraits.h"
#include "Mcro/SharedObjects.h"

namespace Mcro::Text
{
	using namespace Mcro::FunctionTraits;
	using namespace Mcro::SharedObjects;

	FString UnrealCopy(const FStdStringView& stdStr)
	{
		return FString(stdStr.data(), stdStr.size());
	}

	FName UnrealNameCopy(FStdStringView const& stdStr)
	{
		return FName(stdStr.length(), stdStr.data());
	}
	FName UnrealNameConvert(std::string_view const& stdStr)
	{
		return FName(stdStr.length(), stdStr.data());
	}
	FName UnrealNameConvert(std::wstring_view const& stdStr)
	{
		return FName(stdStr.length(), stdStr.data());
	}

	FStdString StdCopy(const FStringView& unrealStr)
	{
		return FStdString(unrealStr.GetData(), unrealStr.Len());
	}
	
	FStdString StdCopy(FName const& unrealStr)
	{
		return StdCopy(unrealStr.ToString());
	}

	FString DynamicPrintf(const TCHAR* fmt, ...)
	{
		int32  BufferSize     = 512;
		TCHAR  StartingBuffer  [512];
		TCHAR* Buffer         = StartingBuffer;
		int32  Result         = -1;

		// First try to print to a stack allocated location 
		GET_TYPED_VARARGS_RESULT( TCHAR, Buffer, BufferSize, BufferSize-1, fmt, fmt, Result );

		// If that fails, start allocating regular memory
		if( Result == -1 )
		{
			Buffer = nullptr;
			while(Result == -1)
			{
				BufferSize *= 2;
				Buffer = (TCHAR*) FMemory::Realloc( Buffer, BufferSize * sizeof(TCHAR) );
				GET_TYPED_VARARGS_RESULT( TCHAR, Buffer, BufferSize, BufferSize-1, fmt, fmt, Result );
			};
		}

		Buffer[Result] = CHARTEXT(TCHAR, '\0');

		FString ResultString(Buffer);

		if( BufferSize != 512 )
		{
			FMemory::Free( Buffer );
		}

		return ResultString;
	}
}

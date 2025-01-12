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
	
	template <
		typename CharFrom, typename CharOutput, typename CharConv,
		typename StringFrom,
		CFunctionLike PtrGetter, CFunctionLike LengthGetter, CFunctionLike Constructor,
		typename StringTo = TFunction_Return<Constructor>
	>
	requires(sizeof(CharOutput) == sizeof(CharConv))
	StringTo HighLevelStringCast(const StringFrom& strIn, PtrGetter&& getPtr, LengthGetter&& getLength, Constructor&& construct)
	{
		// only consider character length differences, if we would precisely consider encoding
		// I would be busy with this forever
		if constexpr (sizeof(CharFrom) == sizeof(CharOutput))
		{
			return construct(
				reinterpret_cast<const CharOutput*>(getPtr()),
				getLength()
			);
		}
		else
		{
			auto conversion = StringCast<CharConv>(
				reinterpret_cast<const CharFrom*>(getPtr()),
				getLength()
			);
			return construct(
				reinterpret_cast<const CharOutput*>(conversion.Get()),
				conversion.Length()
			);
		}
	}

	FStringView UnrealView(const FStdStringView& stdStr)
	{
		return FStringView(stdStr.data(), stdStr.size());
	}

	FUtf8StringView UnrealViewUtf8(const std::string_view& stdStr)
	{
		return FUtf8StringView(stdStr.data(), stdStr.size());
	}

	FUtf16StringView UnrealViewUtf16(const std::wstring_view& stdStr)
	{
		static_assert(sizeof(wchar_t) == 2);
		return FUtf16StringView(
			reinterpret_cast<const UTF16CHAR*>(stdStr.data()),
			stdStr.size()
		);
	}

	FStdStringView StdView(const FString& unrealStr)
	{
		return FStdStringView(*unrealStr, unrealStr.Len());
	}

	FStdStringView StdView(const FStringView& unrealStr)
	{
		return FStdStringView(unrealStr.GetData(), unrealStr.Len());
	}

	std::string_view StdView(const FUtf8StringView& unrealStr)
	{
		return std::string_view(
			reinterpret_cast<const char*>(unrealStr.GetData()),
			unrealStr.Len()
		);
	}

	std::wstring_view StdView(const FUtf16StringView& unrealStr)
	{
		return std::wstring_view(
			reinterpret_cast<const wchar_t*>(unrealStr.GetData()),
			unrealStr.Len()
		);
	}

	FString UnrealCopy(const FStdStringView& stdStr)
	{
		return FString(stdStr.data(), stdStr.size());
	}

	FString UnrealConvert(const std::string_view& stdStr)
	{
		return HighLevelStringCast<char, TCHAR, TCHAR>(
			stdStr,
			[&] { return stdStr.data(); },
			[&] { return stdStr.length(); },
			[](const TCHAR* ptr, int32 len) { return FString(ptr, len); }
		);
	}

	FString UnrealConvert(const std::wstring_view& stdStr)
	{
		return HighLevelStringCast<wchar_t, TCHAR, TCHAR>(
			stdStr,
			[&] { return stdStr.data(); },
			[&] { return stdStr.length(); },
			[](const TCHAR* ptr, int32 len) { return FString(ptr, len); }
		);
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

	std::string StdConvertUtf8(const FStringView& unrealStr)
	{
		return HighLevelStringCast<TCHAR, char, UTF8CHAR>(
			unrealStr,
			[&] { return unrealStr.GetData(); },
			[&] { return unrealStr.Len(); },
			[](const char* ptr, int32 len) { return std::string(ptr, len); }
		);
	}

	std::wstring StdConvertWide(const FStringView& unrealStr)
	{
		return HighLevelStringCast<TCHAR, wchar_t, UTF16CHAR>(
			unrealStr,
			[&] { return unrealStr.GetData(); },
			[&] { return unrealStr.Len(); },
			[](const wchar_t* ptr, int32 len) { return std::wstring(ptr, len); }
		);
	}

	std::string StdConvertUtf8(const FStdStringView& stdStr)
	{
#if PLATFORM_TCHAR_IS_UTF8CHAR
		return std::string(stdStr);
#else
		return HighLevelStringCast<wchar_t, char, UTF8CHAR>(
			stdStr,
			[&] { return stdStr.data(); },
			[&] { return stdStr.length(); },
			[](const char* ptr, int32 len) { return std::string(ptr, len); }
		);
#endif
	}

	std::wstring StdConvertWide(const FStdStringView& stdStr)
	{
#if PLATFORM_TCHAR_IS_UTF8CHAR
		return HighLevelStringCast<char, wchar_t, UTF16CHAR>(
			stdStr,
			[&] { return stdStr.data(); },
			[&] { return stdStr.length(); },
			[](const wchar_t* ptr, int32 len) { return std::wstring(ptr, len); }
		);
#else
		return std::wstring(stdStr);
#endif
	}

	std::string StdConvertUtf8(FName const& unrealName)
	{
		return StdConvertUtf8(unrealName.ToString());
	}
	
	std::wstring StdConvertWide(FName const& unrealName)
	{
		return StdConvertWide(unrealName.ToString());
	}

	FString DynamicPrintf(const TCHAR* fmt, ...)
	{
		int32		BufferSize	= 512;
		TCHAR	StartingBuffer[512];
		TCHAR*	Buffer		= StartingBuffer;
		int32		Result		= -1;

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

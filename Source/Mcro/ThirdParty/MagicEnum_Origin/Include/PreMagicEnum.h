//

/**
 *	@file
 *	Prepare magic enum includes which are compatible with Unreal Engine
 */

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_TCHAR_IS_UTF8CHAR
#define MAGIC_ENUM_USING_ALIAS_STRING_VIEW using string_view = std::string_view;
#define MAGIC_ENUM_USING_ALIAS_STRING      using string      = std::string;
#else
#define MAGIC_ENUM_USING_ALIAS_STRING_VIEW using string_view = std::wstring_view;
#define MAGIC_ENUM_USING_ALIAS_STRING      using string      = std::wstring;
#endif

#define MAGIC_ENUM_RANGE_MIN -128
#define MAGIC_ENUM_RANGE_MAX 127
//

/**
 *	@file
 *	Prepare magic enum includes which are compatible with Unreal Engine
 */

#pragma once

#include "CoreMinimal.h"

#define MAGIC_ENUM_USING_ALIAS_STRING_VIEW using string_view = std::basic_string_view<TCHAR>;
#define MAGIC_ENUM_USING_ALIAS_STRING      using string      = std::basic_string<TCHAR>;

#define MAGIC_ENUM_RANGE_MIN -128
#define MAGIC_ENUM_RANGE_MAX 127
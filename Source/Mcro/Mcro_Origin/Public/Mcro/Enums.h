
#pragma once

#include "CoreMinimal.h"
#include "Mcro/Concepts.h"
#include "Mcro/Text.h"

#include "Mcro/LibraryIncludes/Start.h"
#include "PreMagicEnum.h"
#include "magic_enum.hpp"
#include "Mcro/LibraryIncludes/End.h"

/** @brief Contains utilities for handling enums as strings or vice-versa */
namespace Mcro::Enums
{
	using namespace Mcro::Concepts;
	using namespace Mcro::Text;
	
	template <CEnum Enum>
	FString EnumToStringCopy(Enum input)
	{
		return UnrealCopy(magic_enum::enum_name(input));
	}
	
	template <CEnum Enum>
	FStringView EnumToStringView(Enum input)
	{
		return UnrealView(magic_enum::enum_name(input));
	}
	
	template <CEnum Enum>
	FName EnumToName(Enum input)
	{
		return UnrealNameCopy(magic_enum::enum_name(input));
	}
	
	template <CEnum Enum, CStringOrView String>
	Enum StringToEnum(String const& input)
	{
		return magic_enum::enum_cast<Enum>(StdView(input));
	}
	
	template <CEnum Enum>
	Enum NameToEnum(FName const& input)
	{
		return magic_enum::enum_cast<Enum>(StdCopy(input));
	}
}

namespace Mcro::Text
{
	template <CEnum Operand>
	struct TAsFormatArgument<Operand>
	{
		FStringView operator () (Operand left) const
		{
			return Mcro::Enums::EnumToStringView(left);
		}
	};
}

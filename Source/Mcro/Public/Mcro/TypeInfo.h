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
#include "Mcro/Inheritance.h"
#include "Mcro/TextMacros.h"
#include "Mcro/TypeName.h"

namespace Mcro::TypeInfo
{
	using namespace Mcro::TypeName;
	using namespace Mcro::Inheritance;
	
	/**
	 *	@brief
	 *	Group together type info for identification. Can have an invalid state when no type is specified.
	 *
	 *	If given type also explicitly list its inheritance (through `TInherit` for example) base types are also stored
	 *	for type safety checks.
	 */
	struct MCRO_API FType
	{
		static constexpr int32 MaxBaseCount = 64;
		
		template <typename T>
		struct TTag {};
		
		template <typename T>
		constexpr FType(TTag<T>&&)
			: Name(
				GetCompileTimeTypeName<std::decay_t<T>>().data(),
				GetCompileTimeTypeName<std::decay_t<T>>().size()
			)
			, Hash(GetCompileTimeTypeHash<std::decay_t<T>>())
		{
			if constexpr (CHasBases<T>)
			{
				ForEachExplicitBase<T>([this] <typename Base> ()
				{
					if (BaseCount >= MaxBaseCount) return;
					BaseTypeHashes[BaseCount] = TTypeHash<Base>;
					++BaseCount;
				});
			}
		}
		
		constexpr FType() {}
		
		FStringView Name;
		FTypeHash Hash = 0;

		constexpr FStringView ToString() const { return Name; }
		FORCEINLINE FString ToStringCopy() const { return FString(Name); }

		constexpr bool IsValid() const { return Hash != 0; }
		constexpr operator bool() const { return IsValid(); }

		/** @brief check to see if pointers of this and the other types are safe to cast between */
		constexpr bool IsCompatibleWith(FType const& other) const
		{
			if (Hash == other.Hash) return true;
			
			for (const FTypeHash base : other)
				if (base == Hash) return true;
			
			for (const FTypeHash base : *this)
				if (base == other.Hash) return true;
			
			return false;
		}

		/** @brief check to see if pointers of this and the other types are safe to cast between */
		template <typename Other>
		constexpr bool IsCompatibleWith() const;
		
		friend constexpr bool operator == (FType const& left, FType const& right) { return left.Hash == right.Hash; }
		friend constexpr bool operator != (FType const& left, FType const& right) { return left.Hash != right.Hash; }

		friend constexpr uint32 GetTypeHash(FType const& self)
		{
			return static_cast<uint32>(self.Hash) ^ static_cast<uint32>(self.Hash >> 32);
		}

		constexpr const FTypeHash* begin() const { return BaseTypeHashes; }
		constexpr const FTypeHash* end() const { return BaseTypeHashes + BaseCount; }
		constexpr size_t size() const { return BaseCount; }
		
	private:
		FTypeHash BaseTypeHashes[MaxBaseCount] {};
		int32 BaseCount = 0;
	};

	template <typename T>
	constexpr FType TTypeOf = FType(FType::TTag<T>());

	template <typename Other>
	constexpr bool FType::IsCompatibleWith() const
	{
		return IsCompatibleWith(TTypeOf<Other>);
	}
}

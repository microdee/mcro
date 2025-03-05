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
#include "Mcro/TypeName.h"
#include "Mcro/Concepts.h"

namespace Mcro::Any
{
	using namespace Mcro::TypeName;
	using namespace Mcro::Concepts;

	/**
	 *	@brief
	 *	This template is used in `IComponent|FAny::With(TAlias<...>)` so it can have deduced this type and explicit
	 *	variadic template arguments when specifying multiple aliases.
	 */
	template <typename...>
	struct TAlias {};

	struct FAny;

	/**
	 *	@brief
	 *	Give the opportunity to customize object lifespan operations for `FAny` by either specializing this template
	 *	or just providing functors in-place
	 *	
	 *	@tparam T  The type being set for an FAny
	 */
	template <typename T>
	struct TAnyTypeFacilities
	{
		TFunction<void(T*)> Destruct {[](T* object)
		{
			delete object;
		}};
		
		TFunction<T*(T const&)> CopyConstruct {[](T const& object)
		{
			return new T(object);
		}};
		
		TFunction<T*(T&&)> MoveConstruct {[](T&& object)
		{
			return new T(Forward<T>(object));
		}};
	};

	/**
	 *	@brief
	 *	A simplistic but type-safe and RAII compliant storage for anything. Enclosed data is owned by this type.
	 *
	 *	Use this with care, the underlying data can be only accessed with the same type as it has been constructed with,
	 *	or with types provided by `ValidAs`. This means derived classes cannot be accessed with their base types safely
	 *	and implicitly.
	 *
	 *	Enclosed value must be copy and move constructible and assignable.
	 *
	 *	@todo
	 *	C++ 26 has promising proposal for static value-based reflection, which can gather metadata from classes
	 *	or even emit them. The best summary I found so far is a stack-overflow answer https://stackoverflow.com/a/77477029
	 *	Once that's available we can gather base classes in compile time, and do dynamic casting of objects without
	 *	the need for intrusive extra syntax, or extra work at construction.
	 *	Currently GCC's `__bases` would be perfect for the job, but other popular compilers don't have similar
	 *	intrinsics. Once such a feature becomes widely available base classes can be automatically added as aliases for
	 *	types wrapped in FAny.
	 */
	struct MCRO_API FAny
	{
		template <CCopyable T>
		FAny(T* newObject, TAnyTypeFacilities<T> const& facilities = {})
			: Storage(newObject)
			, MainType(TTypeOf<T>)
			, Destruct([facilities](FAny* self)
			{
				T* object = static_cast<T*>(self->Storage);
				facilities.Destruct(object);
				self->Storage = nullptr;
			})
			, CopyConstruct([facilities](FAny* self, FAny const& other)
			{
				const T* object = static_cast<const T*>(other.Storage);
				self->Storage = facilities.CopyConstruct(*object);
				
				CopyTypeInfo(self, &other);
			})
			, MoveConstruct([facilities](FAny* self, FAny&& other)
			{
				T& object = *static_cast<T*>(other.Storage);
				self->Storage = facilities.MoveConstruct(MoveTemp(object));
				
				CopyTypeInfo(self, &other);
			})
		{
			ValidTypes.Add(MainType);
		}

		FORCEINLINE FAny() {}
		FAny(FAny const& other);
		FAny(FAny&& other);
		~FAny();

		template <typename T>
		const T* TryGet() const
		{
			return ValidTypes.Contains(TTypeOf<T>)
				? static_cast<const T*>(Storage)
				: nullptr;
		}

		template <typename T>
		T* TryGet()
		{
			return ValidTypes.Contains(TTypeOf<T>)
				? static_cast<T*>(Storage)
				: nullptr;
		}
		
		/** @brief Specify one type the enclosed value can be safely cast to, and is valid to be used with `TryGet`. */
		template <typename T, typename Self>
		decltype(auto) WithAlias(this Self&& self)
		{
			self.ValidTypes.Add(TTypeOf<T>);
			return Forward<Self>(self);
		}

		/** @brief Specify multiple types the enclosed value can be safely cast to, and are valid to be used with `TryGet`. */
		template <typename Self, typename... T>
		decltype(auto) With(this Self&& self, TAlias<T...>&&)
		{
			(self.ValidTypes.Add(TTypeOf<T>), ...);
			return Forward<Self>(self);
		}

		FORCEINLINE bool IsValid() const { return static_cast<bool>(Storage); }
		FORCEINLINE FType GetType() const { return MainType; }
		FORCEINLINE TSet<FType> const& GetValidTypes() const { return ValidTypes; }
		
	private:
		static void CopyTypeInfo(FAny* self, const FAny* other);
		
		void* Storage = nullptr;
		FType MainType {};
		
		TFunction<void(FAny* self)> Destruct {};
		TFunction<void(FAny* self, FAny const& other)> CopyConstruct {};
		TFunction<void(FAny* self, FAny&& other)> MoveConstruct {};
		
		TSet<FType> ValidTypes {};
	};
}
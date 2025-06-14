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
#include "Mcro/Ansi/Allocator.h"
#include "Mcro/Ansi/New.h"
#include "Mcro/TypeName.h"
#include "Mcro/TextMacros.h"
#include "Mcro/Templates.h"
#include "Mcro/FunctionTraits.h"

namespace Mcro::Any
{
	using namespace Mcro::TypeName;
	using namespace Mcro::Templates;
	using namespace Mcro::FunctionTraits;

	/**
	 *	@brief
	 *	Some MCRO utilities allow for intrusive method of declaring inheritance which can be later used to reflect
	 *	upon base classes of a derived type.
	 */
	template <typename T>
	concept CHasBases = CIsTypeList<typename T::Bases>;

	/**
	 *	@brief
	 *	Inherit via this template to allow other API to reflect upon the base types of deriving class. Base types are
	 *	inherited as public. If you want privately inherited base classes, just inherit them as normal.
	 *
	 *	Usage:
	 *	@code
	 *	class FMyThing : public TInherit<IFoo, IBar, IEtc>
	 *	{
	 *		// ...
	 *	}
	 *	@endcode
	 *
	 *	@todo
	 *	Currently abstract classes are not supported due to a call to `DeclVal` in `TTupleElement`.
	 */
	template <typename... BaseTypes>
	class TInherit : public BaseTypes...
	{
	public:
		using Bases = TTypes<BaseTypes...>;
	};

	namespace Detail
	{
		template <CIsTypeList Bases, typename Function>
		void ForEachExplicitBase(Function&& function);

		template <typename T, typename Function>
		void ForEachExplicitBase_Body(Function&& function)
		{
			function(TTypes<T>());
			if constexpr (CHasBases<T>)
				ForEachExplicitBase<typename T::Bases>(FWD(function));
		}

		template <CIsTypeList Bases, typename Function, size_t... Indices>
		void ForEachExplicitBase_Impl(Function&& function, std::index_sequence<Indices...>&&)
		{
			(ForEachExplicitBase_Body<TTypes_Get<Bases, Indices>>(FWD(function)), ...);
		}

		template <CIsTypeList Bases, typename Function>
		void ForEachExplicitBase(Function&& function)
		{
			ForEachExplicitBase_Impl<Bases>(
				FWD(function),
				std::make_index_sequence<Bases::Count>()
			);
		}
	}

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
		TFunction<void(T*)> Destruct {[](T* object) { delete object; }};
		TFunction<T*(T const&)> CopyConstruct {[](T const& object)
		{
			if constexpr (CCopyConstructible<T>) return new T(object); 
			else return nullptr;
		}};
	};

	/** @brief Type facilities for `FAny` enforcing standard memory allocations */
	template <typename T>
	inline TAnyTypeFacilities<T> AnsiAnyFacilities = {
		.Destruct = [](T* object) { Ansi::Delete(object); },
		.CopyConstruct = [](T const& object)
		{
			if constexpr (CCopyConstructible<T>) return Ansi::New<T>(object);
			else return nullptr;
		}
	};

	/**
	 *	@brief
	 *	A simplistic but type-safe and RAII compliant storage for anything. Enclosed data is owned by this type.
	 *
	 *	Use this with care, the underlying data can be only accessed with the same type as it has been constructed with,
	 *	or with types provided by `ValidAs`. This means derived classes cannot be accessed with their base types safely
	 *	and implicitly. MCRO however provides methods for classes to allow them exposing base types to FAny (and
	 *	other facilities):
	 *	@code
	 *	class FMyThing : public TInherit<IFoo, IBar, IEtc>
	 *	{
	 *		// ...
	 *	}
	 *	@endcode
	 *	`TInherit` has a member alias `using Bases = TTypes<...>` and that can be used by FAny to automatically register
	 *	base classes as compatible ones.
	 *
	 *	Enclosed value is recommended to be copy constructible. It may yield a runtime error otherwise. Moving an FAny
	 *	will just transfer ownership of the wrapped object but will not move construct a new object. The source FAny
	 *	will be reset to an invalid state.
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
		template <typename T>
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
				checkf(self->Storage, TEXT_"Copy constructor failed for %s. Is it deleted?", *TTypeString<T>());
				
				CopyTypeInfo(self, &other);
			})
		{
			ValidTypes.Add(MainType);
			
			if constexpr (CHasBases<T>)
			{
				Detail::ForEachExplicitBase<typename T::Bases>([this] <typename Base> (TTypes<Base>&&)
				{
					AddAlias(TTypeOf<Base>);
				});
			}
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
			self.AddAlias(TTypeOf<T>);
			
			if constexpr (CHasBases<T>)
			{
				Detail::ForEachExplicitBase<typename T::Bases>([&] <typename Base> (TTypes<Base>&&)
				{
					self.AddAlias(TTypeOf<Base>);
				});
			}
			return FWD(self);
		}

		/** @brief Specify multiple types the enclosed value can be safely cast to, and are valid to be used with `TryGet`. */
		template <typename Self, typename... T>
		decltype(auto) With(this Self&& self, TTypes<T...>&&)
		{
			(self.AddAlias(TTypeOf<T>), ...);
			return FWD(self);
		}

		FORCEINLINE bool IsValid() const { return static_cast<bool>(Storage); }
		FORCEINLINE FType GetType() const { return MainType; }
		FORCEINLINE TSet<FType> const& GetValidTypes() const { return ValidTypes; }
		
	private:
		void AddAlias(FType const& alias);
		static void CopyTypeInfo(FAny* self, const FAny* other);
		void Reset();
		
		void* Storage = nullptr;
		FType MainType {};
		
		TFunction<void(FAny* self)> Destruct {};
		TFunction<void(FAny* self, FAny const& other)> CopyConstruct {};
		
		TSet<FType> ValidTypes {};
	};
}
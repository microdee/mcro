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
#include "Mcro/Templates.h"
#include "Mcro/FunctionTraits.h"

namespace Mcro::Inheritance
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

	namespace Detail
	{
		template <CIsTypeList Bases, typename Function>
		void ForEachExplicitBase(Function&& function);

		template <typename T, typename Function>
		void ForEachExplicitBase_Body(Function&& function)
		{
			function.template operator()<T> ();
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

	/**
	 *	@brief
	 *	Operate on each of the explicitly listed base classes of a given type with a lambda function template. If any
	 *	given type also happens to match `CHasBases`, iterate on their base types too recursively. Works directly with
	 *	`CHasBases` or type lists (`CIsTypeList`).
	 *
	 *	Usage:
	 *	@code
	 *	class FMyClass : public TInherit<FFoo, FBar>
	 *	{...}
	 *
	 *	ForEachExplicitBase<FMyClass>([] <typename T> ()
	 *	{
	 *		UE_LOG(LogTemp, Display, TEXT_"%s", *TTypeString<T>())
	 *	});
	 *	@endcode
	 *	
	 *	@tparam        T  A type which uses TInherit or has a TTypes list member-aliased as Bases
	 *	@tparam Function  Type of per-base operation
	 *	@param  function  Per-base operation
	 */
	template <typename T, typename Function>
	requires (CHasBases<T> || CIsTypeList<T>)
	void ForEachExplicitBase(Function&& function)
	{
		if constexpr (CHasBases<T>)
			Detail::ForEachExplicitBase<typename T::Bases>(FWD(function));
		if constexpr (CIsTypeList<T>)
			Detail::ForEachExplicitBase<T>(FWD(function));
	}

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
	 */
	template <typename... BaseTypes>
	class TInherit : public BaseTypes...
	{
		public:
		using Bases = TTypes<BaseTypes...>;
		
		/**
		 *	@brief
		 *	Operate on each of the explicit base class of this type with a lambda function template. If any given type
		 *	also happens to match CHasBases, iterate on their base types too recursively
		 *
		 *	This is the same as `ForEachExplicitBase` but for object instances of this type. 
		 *
		 *	Usage:
		 *	@code
		 *	class FMyClass : public TInherit<FFoo, FBar>
		 *	{...}
		 *
		 *	FMyClass value;
		 *
		 *	value.ForEachBase([] <typename T> ()
		 *	{
		 *		UE_LOG(LogTemp, Display, TEXT_"%s", *TTypeString<T>())
		 *	});
		 *	@endcode
		 *	
		 *	@tparam     Self  deducing this
		 *	@tparam Function  Type of per-base operation
		 *	@param  function  Per-base operation
		 */
		template <typename Self, typename Function>
		void ForEachBase(this Self&&, Function&& function)
		{
			Detail::ForEachExplicitBase<Bases>(FWD(function));
		}
	};
}

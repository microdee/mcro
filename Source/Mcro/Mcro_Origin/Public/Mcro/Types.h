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

#include <string>

#include "CoreMinimal.h"
#include "Mcro/TypeName.h"
#include "Mcro/Templates.h"

/** C++ native static reflection utilities, not to be confused with reflection of UObjects */
namespace Mcro::Types
{
	using namespace Mcro::TypeName;
	using namespace Mcro::Templates;
	
	class IHaveType;

	template <typename T>
	concept CHasTypeName = CDerivedFrom<T, IHaveType>;

	/**
	 *	@brief  A barebones base class for types which may store their type-name as a string
	 *
	 *	@todo
	 *	C++ 26 has promising proposal for static value-based reflection, which can gather metadata from classes
	 *	or even emit them. The best summary I found so far is a stack-overflow answer https://stackoverflow.com/a/77477029
	 *	Once that's available we can gather base classes in compile time, and do dynamic casting of objects without
	 *	the need for intrusive extra syntax, or extra work at construction.
	 *	Currently GCC's `__bases` would be perfect for the job, but other popular compilers don't have similar intrinsics.
	 */
	class IHaveType : public TSharedFromThis<IHaveType>
	{
	public:
		virtual ~IHaveType() = default;
		
	protected:
		FName TypeName;

		/** @brief This function needs to be called on top level derived type for runtime reflection to work */
		template <typename Self>
		void SetType(this Self&& self)
		{
			self.TypeName = TTypeFName<Self>;
		}
		
	public:
		template <typename Self>
		using SelfRef = TSharedRef<std::decay_t<Self>>;

		/** @brief Fluent API for setting tpye for deferred initialization (for example in factory functions) */
		template <typename Self>
		SelfRef<Self> WithType(this Self&& self)
		{
			self.SetType();
			return SharedThis(&self);
		}

		FORCEINLINE FName const& GetType() const { return TypeName; }

		/**
		 *	@brief  Very simple dynamic casting of this object to a derived top-level type.
		 *
		 *	@tparam Derived
		 *	Only return the desired type when the current object is exactly that type, and doesn't have deeper
		 *	inheritance. Proper dynamic casting regarding the entire inheritance tree still without RTTI will come once
		 *	proposed C++26 value-typed reflection becomes available.
		 *
		 *	@return  Object cast to desired type when that's possible (see `Derived`) or nullptr;
		 */
		template <typename Derived, typename Self>
		TSharedPtr<Derived> AsExactly(this Self&& self)
		{
			if constexpr (CDerivedFrom<Derived, Self>)
				if (self.TypeName == TTypeFName<Derived>)
					return StaticCastSharedPtr<Derived>(
						SharedThis(AsMutablePtr(&self)).ToSharedPtr()
					);
			return {};
		}
	};
}

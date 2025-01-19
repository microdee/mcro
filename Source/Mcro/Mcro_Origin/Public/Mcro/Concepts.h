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

/**
 *	@file 
 *	This header exists because STL headers in Android doesn't define STL concepts (other than same_as which is weird),
 *	despite the fact that in `<concepts>` the synopsis contains all of them (but only as a comment).
 *	@file
 *	However, <type_traits> seems to be implemented properly so many concepts here will be just encapsulating equivalent
 *	type traits (like CConstType = std::is_const_v<T>).
 *	Using MSVC implementation as reference
 */

#if !defined(__cpp_concepts) || !__cpp_concepts
#error "This code requires C++20 concepts support"
#endif

#include <type_traits>
#include "CoreMinimal.h"

#define MCRO_THIS_TYPE decltype(Mcro::Concepts::DecayPtr(this))

namespace Mcro::Concepts
{
	//// Concepts ported from STL

	template<typename A, typename B>
	concept CSameAs = std::is_same_v<A, B>;

	template<typename A, typename B>
	concept CSameAsDecayed = std::is_same_v<std::decay_t<A>, std::decay_t<B>>;

	template<typename From, typename To>
	concept CConvertibleTo = std::is_convertible_v<From, To>;

	template<typename From, typename To>
	concept CConvertibleToDecayed = std::is_convertible_v<std::decay_t<From>, std::decay_t<To>>;

	template<typename Left, typename Right>
	concept CAssignableFrom = std::is_assignable_v<Left, Right>;

	template<typename Left, typename Right>
	concept CAssignableFromDecayed = std::is_assignable_v<std::decay_t<Left>, std::decay_t<Right>>;

	template<typename Derivative, typename Base>
	concept CDerivedFrom =
		std::is_base_of_v<
			std::remove_pointer_t<std::decay_t<Base>>,
			std::remove_pointer_t<std::decay_t<Derivative>>
		>
		&& CConvertibleTo<const volatile std::decay_t<Derivative>*, const volatile std::decay_t<Base>*>
	;

	template<typename T>
	concept CIntegral = std::is_integral_v<std::decay_t<T>>;

	template<typename T>
	concept CSignedIntegral = CIntegral<T> && std::is_signed_v<std::decay_t<T>>;

	template<typename T>
	concept CUnsignedIntegral = CIntegral<T> && !std::is_signed_v<std::decay_t<T>>;

	template<typename T>
	concept CFloatingPoint = std::is_floating_point_v<std::decay_t<T>>;

	template<typename T>
	concept CScalar = CIntegral<T> || CFloatingPoint<std::decay_t<T>>;

	template<typename T>
	concept CBooleanTestable = CConvertibleToDecayed<T, bool>
		&& requires(T&& t)
		{
			{ !Forward<T>(t) } -> CConvertibleTo<bool>;
		}
	;

	template<typename T>
	concept CPlainEnum = std::is_enum_v<std::decay_t<T>>;

	template<typename T>
	concept CScopedEnum = std::is_scoped_enum_v<std::decay_t<T>>;

	template<typename T>
	concept CEnum = CPlainEnum<T> || CScopedEnum<T>;

	template<typename A, typename B>
	concept CCoreHalfEqualityComparable =
		requires(const std::remove_reference_t<A>& a, const std::remove_reference_t<B>& b)
	{
		{ a == b } -> CBooleanTestable;
		{ a != b } -> CBooleanTestable;
	}
	;

	template<typename A, typename B>
	concept CWeaklyEqualityComparableWith = CCoreHalfEqualityComparable<A, B> && CCoreHalfEqualityComparable<B, A>;

	template<typename T>
	concept CCoreEqualityComparable = CCoreHalfEqualityComparable<T, T>;

	template<typename A, typename B>
	concept CHalfOrdered = requires(const std::remove_reference_t<A>& a, const std::remove_reference_t<B>& b)
	{
		{ a < b } -> CBooleanTestable;
		{ a > b } -> CBooleanTestable;
		{ a <= b } -> CBooleanTestable;
		{ a >= b } -> CBooleanTestable;
	};

	template<typename A, typename B>
	concept CPartiallyOrderedWith = CHalfOrdered<A, B> && CHalfOrdered<B, A>;

	template<typename T>
	concept CTotallyOrdered = CCoreEqualityComparable<T> && CHalfOrdered<T, T>;

	template<typename T>
	concept CConstType = std::is_const_v<T>;

	template<typename T>
	concept CMutableType = !std::is_const_v<T>;

	template<typename T>
	concept CDestructible = std::is_nothrow_destructible_v<std::decay_t<T>>;

	template<typename T, typename... From>
	concept CConstructibleFrom = requires(From&&... from) { std::decay_t<T>(Forward<From>(from)...); };

	template<typename T>
	concept CDefaultInitializable = CConstructibleFrom<T> && requires { std::decay_t<T>{}; };

	template<typename T>
	concept CMoveConstructible = CConstructibleFrom<T, T> && CConvertibleTo<T, T>;

	template<typename T>
	concept CCopyConstructible =
		CConstructibleFrom<T, T&> && CConvertibleTo<T&, T>
		&& CConstructibleFrom<T, const T&> && CConvertibleTo<const T&, T>
		&& CConstructibleFrom<T, const T> && CConvertibleTo<const T, T>
	;

	template<typename T>
	concept CMovable = std::is_object_v<T>
		&& CMoveConstructible<T>
		&& CAssignableFrom<T&, T>
		&& CAssignableFrom<T&, T&&>
	;

	template<typename T>
	concept CCopyable = CCopyConstructible<T>
		&& CMovable<T>
		&& CAssignableFrom<T&, T&>
		&& CAssignableFrom<T&, const T&>
		&& CAssignableFrom<T&, const T>
	;

	template<typename T>
	concept CSemiRegular = CCopyable<T> && CDefaultInitializable<T>;

	template<typename T>
	concept CRegular = CSemiRegular<T> && CCoreEqualityComparable<T>;

	template<typename Function, typename... Args>
	concept CInvocable = requires(std::decay_t<Function>&& function, Args&&... args)
	{
		std::invoke(Forward<std::decay_t<Function>&&>(function), Forward<Args&&>(args)...);
	};

	template<typename Function, typename... Args>
	concept CPredicate = CInvocable<Function, Args...> && CBooleanTestable<std::invoke_result_t<Function, Args...>>;

	template<typename Function, typename A, typename B>
	concept CRelation =
		CPredicate<Function, A, A>
		&& CPredicate<Function, B, B>
		&& CPredicate<Function, A, B>
		&& CPredicate<Function, B, A>
	;

	template<typename Function, typename A, typename B>
	concept CEquivalenceRelation = CRelation<Function, A, B>;

	template<typename Function, typename A, typename B>
	concept CStrictWeakOrder = CRelation<Function, A, B>;

	/** Use Unreal's own Concept templating constraint as a C++20 concept */
	template<typename Concept, typename... Args>
	concept CModels = TModels_V<Concept, Args...>;

	//// TSharedPtr, TSharedRef and TWeakPtr concepts

	template<typename T>
	concept CSharedPtr = CSameAsDecayed<T, TSharedPtr<typename std::decay_t<T>::ElementType, std::decay_t<T>::Mode>>;

	template<typename T>
	concept CSharedRef = CSameAsDecayed<T, TSharedRef<typename std::decay_t<T>::ElementType, std::decay_t<T>::Mode>>;

	template<typename T>
	concept CWeakPtr = CSameAsDecayed<T, TWeakPtr<typename std::decay_t<T>::ElementType, std::decay_t<T>::Mode>>;

	template<typename T>
	concept CSharedRefOrPtr = CSharedRef<T> || CSharedPtr<T>;

	template<typename T>
	concept CSharedOrWeak = CSharedRefOrPtr<T> || CWeakPtr<T>;

	template<typename T>
	concept CSharedFromThis = requires(std::decay_t<T>& t) { { t.AsShared() } -> CSharedRefOrPtr; };

	//// UClasses constraining concepts

	template<typename T>
	concept CUObject = CDerivedFrom<T, UObject>;

	template<typename T>
	concept CInterface = TIsIInterface<std::decay_t<T>>::Value > 0;

	template<typename T>
	concept CInterfaceUClass = CDerivedFrom<T, UInterface>;

	//// String/Text concepts

	template<typename T>
	concept CCharPtr = CSameAs<std::remove_const_t<T>, TCHAR*>;

	template<typename T>
	concept CNonCharPtr = !CCharPtr<T>;

	template<typename T>
	concept CChar = CSameAs<std::remove_const_t<T>, TCHAR>;

	template<typename T>
	concept CNonChar = !CChar<T>;

	//// Simople objects

	template <typename T>
	concept CPointer = std::is_pointer_v<std::decay_t<T>>;

	template <typename T>
	concept CClass = std::is_class_v<std::decay_t<T>>;

	template <typename T>
	concept CUnion = std::is_union_v<std::decay_t<T>>;
	
	template <typename T>
	concept CPlainClass = CClass<T>
		&& !CUObject<T>
		&& !CSharedFromThis<T>
	;

	//// Is member pointer

	namespace Detail
	{
		template<typename T>
		struct TExtractClassName;

		// Given a pointer to a member, extract the class type (e.g. "int MyClass::*" -> Type will contain "MyClass")
		template<typename ClassType, typename MemberType>
		struct TExtractClassName<MemberType ClassType::*>
		{
			using Type = ClassType;
		};

		// Given a pointer to a member, extract the class type (e.g. "int MyClass::*" -> returns "MyClass")
		template<typename MemberPointer>
		using TExtractedClassName = typename TExtractClassName<std::decay_t<MemberPointer>>::Type;
	}
	
	/**
	 *	Concept that returns true if the given member pointer belongs to the class.
	 *	
	 *	Example:
	 *	@code
	 *	class MyClass
	 *	{
	 *		int myMember;
	 *	}
	 *	
	 *	CMemberPointerOf<MyClass, &MyClass::myMember> // --> true;
	 *	
	 *	class MyOtherClass
	 *	{
	 *	}
	 *	
	 *	CMemberPointerOf<MyOtherClass, &MyClass::myMember> // --> false;
	 *	@endcode
	 */
	template <typename OwnerObject, typename MemberPointerType>
	concept CMemberPointerOf =
		std::is_member_pointer_v<MemberPointerType>
		&& CSameAsDecayed<OwnerObject, Detail::TExtractedClassName<MemberPointerType>>
	;

	//// Misc

	template <typename T>
	concept CNonVoid = !std::is_void_v<T>;

	template<typename T>
	concept CRefCounted = requires(std::decay_t<T>& t)
	{
		t.AddRef();
		t.Release();
	};

	template <typename T>
	concept CRange = requires(std::decay_t<T> const& t)
	{
		t.begin();
		t.end();
	};

	// use in decltype
	template <typename T> T DecayPtr(T*);
}

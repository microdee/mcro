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

#include "CoreMinimal.h"
#include "Mcro/CommonCore.h"

using namespace Mcro::Common;

struct FFunctionTestType
{
	void MemberMethodConst(bool, char) const {}
	void MemberMethod() {}
	static void StaticFunction() {}

	FString MemberMethodReturns(bool boolean, TCHAR character) const
	{
		return FString::Printf(TEXT_"boolean %d character %c", boolean, character);
	}
};

struct FDerivedFunctionTestType : FFunctionTestType
{
	void MethodOfDerivedClass() {}
};

FFunctionTestType&& SomeGlobalFunction(FFunctionTestType&& arg, int) { return MoveTemp(arg); }

using FSomeGlobalFunctionPtr = decltype(&SomeGlobalFunction);

struct FExplicitFunctor
{
	int operator () () const { return 0; }
};

static_assert(!CFunctionLike<FFunctionTestType>);  // CFunctionLike should be false on some random type
static_assert(!CFunctorObject<FFunctionTestType>); // CFunctorObject should be false on some random type
static_assert(!CFunctionPtr<FFunctionTestType>);   // CFunctionPtr should be false on some random type

static_assert(CFunctionLike<FSomeGlobalFunctionPtr>);                     // CFunctionLike should accept global functions
static_assert(CFunctionLike<decltype(&FFunctionTestType::MemberMethod)>); // CFunctionLike should accept member functions
static_assert(CFunctionLike<FExplicitFunctor>);                           // CFunctionLike should accept functors
static_assert(CFunctionLike<FExplicitFunctor const&>);                    // CFunctionLike should not get stuck up on CV-ref qualifiers
static_assert(CFunctionLike<decltype([]{})>);                             // CFunctionLike should accept lambda functions

static_assert(CFunctorObject<FExplicitFunctor>);        // CFunctorObject should accept functors
static_assert(CFunctorObject<FExplicitFunctor const&>); // CFunctorObject should not get stuck up on CV-ref qualifiers
static_assert(CFunctorObject<decltype([]{})>);          // CFunctorObject should accept lambda functions

static_assert(!CFunctionPtr<FExplicitFunctor>);                          // CFunctionPtr should be false for functor objects
static_assert(CFunctionPtr<FSomeGlobalFunctionPtr>);                     // CFunctionPtr should accept global function pointers
static_assert(CFunctionPtr<decltype(&FFunctionTestType::MemberMethod)>); // CFunctionPtr should accept member function pointers

// TFunction_Arguments should produce a tuple from input function's arguments, and preserve qualifiers
static_assert(CSameAs<
	TFunction_Arguments<FSomeGlobalFunctionPtr>,
	TTuple<FFunctionTestType&&, int>
>);

// TFunction_ArgumentsDecay should produce a tuple from input function's arguments, and discard qualifiers
static_assert(CSameAs<
	TFunction_ArgumentsDecay<FSomeGlobalFunctionPtr>,
	TTuple<FFunctionTestType, int>
>);

// TFunction_Arg should select an argument at specified index, and preserve qualifiers
static_assert(CSameAs<TFunction_Arg<FSomeGlobalFunctionPtr, 0>, FFunctionTestType&&>);

// TFunction_Arg should select an argument at specified index, and discard qualifiers
static_assert(CSameAs<TFunction_ArgDecay<FSomeGlobalFunctionPtr, 0>, FFunctionTestType>);

// TFunction_ArgCount should return the correct argument count of a function
static_assert(TFunction_ArgCount<decltype([](int, int, int){})> == 3);

// TFunction_Return should produce the return type of a function, and preserve qualifiers
static_assert(CSameAs<TFunction_Return<FSomeGlobalFunctionPtr>, FFunctionTestType&&>);

// TFunction_Signature should produce a pure function signature type, discarding function qualifiers and class membership
static_assert(CSameAs<
	TFunction_Signature<decltype(&FFunctionTestType::MemberMethodConst)>,
	void(bool, char)
>);

// TFunction_Signature should produce a pure function signature type from functors.
static_assert(CSameAs<
	TFunction_Signature<decltype([](bool, char){})>,
	void(bool, char)
>);

// TFunction_Class should produce the class that is containing the input function
static_assert(CSameAs<
	TFunction_Class<decltype(&FFunctionTestType::MemberMethodConst)>,
	FFunctionTestType
>);

// CFunction_IsMember should be true for function bound to a class
static_assert(CFunction_IsMember<decltype(&FFunctionTestType::MemberMethodConst)>);

// CFunction_IsMember should be false for global functions
static_assert(!CFunction_IsMember<FSomeGlobalFunctionPtr>);

// CFunction_IsConst should be true for const qualified functions
static_assert(CFunction_IsConst<decltype(&FFunctionTestType::MemberMethodConst)>);

// CFunction_IsConst should be false for functions not qualified for const
static_assert(!CFunction_IsConst<decltype(&FFunctionTestType::MemberMethod)>);

// CHasFunction should accept functions of inherited classes
static_assert(CHasFunction<FDerivedFunctionTestType, decltype(&FFunctionTestType::MemberMethodConst)>);

// CHasFunction should reject functions not member of any class in the inheritance tree of specified class
static_assert(!CHasFunction<FDerivedFunctionTestType, FSomeGlobalFunctionPtr>);

// TFunctionFromTuple should produce a function signature from given tuple of arguments and the given return type
static_assert(CSameAs<
	TFunctionFromTuple<void, TTuple<bool, char, int>>,
	void(bool, char, int)
>);

// TSetReturnDecay should produce a function signature which returns given type decayed with the arguments of another function
static_assert(CSameAs<
	TSetReturnDecay<float, void(bool, char, int)>,
	float(bool, char, int)
>);

// TSetReturnDecay should work with functors as well
static_assert(CSameAs<
	TSetReturnDecay<float, decltype([](bool, char, int) {})>,
	float(bool, char, int)
>);

// TCopyReturn should set the return type of input function to the return type of another function while preserving qualifiers
static_assert(CSameAs<
	TCopyReturn<FSomeGlobalFunctionPtr, decltype([](bool, char, int) {})>,
	FFunctionTestType&&(bool, char, int)
>);

// TCopyReturnDecay should set the return type of input function to the return type of another function while discarding qualifiers
static_assert(CSameAs<
	TCopyReturnDecay<FSomeGlobalFunctionPtr, decltype([](bool, char, int) {})>,
	FFunctionTestType(bool, char, int)
>);

// CInstanceMethod should accept a function pointer value which is a non-static member method of a class
static_assert(CInstanceMethod<&FFunctionTestType::MemberMethodConst>);

// CInstanceMethod should reject a function pointer value which is a static member method of a class
static_assert(!CInstanceMethod<&FFunctionTestType::StaticFunction>);

DEFINE_SPEC(
	FMcroFunctionTraits_Spec,
	TEXT_"Mcro.FunctionTraits",
	EAutomationTestFlags_ApplicationContextMask
	| EAutomationTestFlags::CriticalPriority
	| EAutomationTestFlags::ProductFilter
);

void FMcroFunctionTraits_Spec::Define()
{
	Describe(TEXT_"InvokeWithTuple", [this]
	{
		It(TEXT_"should produce the same result as calling the function natively", [this]
		{
			FFunctionTestType instance;
			auto expected = instance.MemberMethodReturns(true, TCHAR('C'));
			auto actual = InvokeWithTuple(
				&instance, &FFunctionTestType::MemberMethodReturns,
				TTuple<bool, TCHAR>{ true, TCHAR('C') }
			);
			TestEqualSensitive(
				TEXT_"Native function call result is equal to InvokeWithTuple result.",
				actual, expected
			);
		});
	});
}

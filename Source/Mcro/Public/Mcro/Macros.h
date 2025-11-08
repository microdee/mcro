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
#include "HAL/PreprocessorHelpers.h"
#include "boost/preprocessor.hpp"
#include "Misc/EngineVersionComparison.h"

// These here are back-porting missing macros from UE 5.4 to at least UE 5.3
#if UE_VERSION_OLDER_THAN(5,4,0)

// Expands to a number which is the count of variadic arguments passed to it.
#define UE_VA_ARG_COUNT(...) UE_APPEND_VA_ARG_COUNT(, ##__VA_ARGS__)

// Expands to nothing when used as a function - used as a placeholder
#define UE_EMPTY_FUNCTION(...)

// Expands to a token of Prefix##<count>, where <count> is the number of variadic arguments.
//
// Example:
//   UE_APPEND_VA_ARG_COUNT(SOME_MACRO_)          => SOME_MACRO_0
//   UE_APPEND_VA_ARG_COUNT(SOME_MACRO_, a, b, c) => SOME_MACRO_3
#if !defined(_MSVC_TRADITIONAL) || !_MSVC_TRADITIONAL
 #define UE_APPEND_VA_ARG_COUNT(Prefix, ...) UE_PRIVATE_APPEND_VA_ARG_COUNT(Prefix, ##__VA_ARGS__, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#else
 #define UE_APPEND_VA_ARG_COUNT(Prefix, ...) UE_PRIVATE_APPEND_VA_ARG_COUNT_INVOKE(UE_PRIVATE_APPEND_VA_ARG_COUNT, (Prefix, ##__VA_ARGS__, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

 // MSVC's traditional preprocessor doesn't handle the zero-argument case correctly, so we use a workaround.
 // The workaround uses token pasting of Macro##ArgsInParens, which the conformant preprocessor doesn't like and emits C5103.
 #define UE_PRIVATE_APPEND_VA_ARG_COUNT_INVOKE(Macro, ArgsInParens) UE_PRIVATE_APPEND_VA_ARG_COUNT_EXPAND(Macro##ArgsInParens)
 #define UE_PRIVATE_APPEND_VA_ARG_COUNT_EXPAND(Arg) Arg
#endif
#define UE_PRIVATE_APPEND_VA_ARG_COUNT(Prefix,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,Count,...) Prefix##Count

#define PREPROCESSOR_VA_ARG_COUNT(...)                 UE_VA_ARG_COUNT(__VA_ARGS__)
#define PREPROCESSOR_APPEND_VA_ARG_COUNT(Prefix, ...)  UE_APPEND_VA_ARG_COUNT(Prefix, ##__VA_ARGS__)
#define PREPROCESSOR_NOTHING_FUNCTION(...)             UE_EMPTY_FUNCTION(__VA_ARGS__)

#endif

#define PREPROCESSOR_TO_TEXT(x) TEXT(PREPROCESSOR_TO_STRING(x))

/**
 * @brief
 * Implement preprocessor function overloading based on argument count for a set of macros following a distinct
 * convention. Individual overloads must have a trailing number corresponding to the number of arguments they accept
 *
 * For example:
 * @code
 * #define FOO_3(a, b, c) a##b##c
 * #define FOO_2(a, b)    a##b
 * #define FOO_1(a)       a
 *
 * #define FOO(...) MACRO_OVERLOAD(FOO_, __VA_ARGS__)
 *
 * FOO(1) // -> 1
 * FOO(1, 2) // -> 12
 * FOO(1, 2, 3) // -> 123
 * @endcode
 */
#define MACRO_OVERLOAD(prefix, ...) PREPROCESSOR_APPEND_VA_ARG_COUNT(prefix, __VA_ARGS__)(__VA_ARGS__)

/** @brief Returns given default value when input value is empty */
#define DEFAULT_ON_EMPTY(value, default) BOOST_PP_IF(BOOST_PP_CHECK_EMPTY(value), default, value)

// Forward function of Unreal before UE 5.5 is not constexpr
#if UE_VERSION_OLDER_THAN(5,5,0)

template <typename T>
FORCEINLINE constexpr T&& ForwardConstExpr(typename TRemoveReference<T>::Type& Obj)
{
	return (T&&)Obj;
}

template <typename T>
FORCEINLINE constexpr T&& ForwardConstExpr(typename TRemoveReference<T>::Type&& Obj)
{
	return (T&&)Obj;
}

#define MCRO_FORWARD_FUNCTION ForwardConstExpr

#else

#define MCRO_FORWARD_FUNCTION Forward

#endif

/** @brief Shorten forwarding expression with this macro so one may not need to specify explicit type */
#define FWD(...) MCRO_FORWARD_FUNCTION<decltype(__VA_ARGS__)>(__VA_ARGS__)
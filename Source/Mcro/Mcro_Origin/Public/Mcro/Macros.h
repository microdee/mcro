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

#define MCRO_HAS_PARENTHESIS_0(...) 1
#define MCRO_HAS_PARENTHESIS_1(...) 0
#define MCRO_HAS_PARENTHESIS_DECIDE(...) MACRO_OVERLOAD(MCRO_HAS_PARENTHESIS_, __VA_ARGS__)

#define MCRO_HAS_PARENTHESIS_PASS(...)

/** @brief returns 1 if given argument is wrapped in parenthesis, returns 0 otherwise */
#define HAS_PARENTHESIS(a) MCRO_HAS_PARENTHESIS_DECIDE(PREPROCESSOR_JOIN(MCRO_HAS_PARENTHESIS_PASS, a))

/** @brief returns 1 if the first argument is wrapped in parenthesis, returns 0 otherwise */
#define FIRST_HAS_PARENTHESIS(first, ...) MCRO_HAS_PARENTHESIS_DECIDE(PREPROCESSOR_JOIN(MCRO_HAS_PARENTHESIS_PASS, first))

#define MCRO_FOR_EACH_VA_ARGS_1(transform, first)       transform(first)
#define MCRO_FOR_EACH_VA_ARGS_2(transform, first, ...)  transform(first), MCRO_FOR_EACH_VA_ARGS_1(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_3(transform, first, ...)  transform(first), MCRO_FOR_EACH_VA_ARGS_2(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_4(transform, first, ...)  transform(first), MCRO_FOR_EACH_VA_ARGS_3(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_5(transform, first, ...)  transform(first), MCRO_FOR_EACH_VA_ARGS_4(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_6(transform, first, ...)  transform(first), MCRO_FOR_EACH_VA_ARGS_5(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_7(transform, first, ...)  transform(first), MCRO_FOR_EACH_VA_ARGS_6(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_8(transform, first, ...)  transform(first), MCRO_FOR_EACH_VA_ARGS_7(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_9(transform, first, ...)  transform(first), MCRO_FOR_EACH_VA_ARGS_8(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_10(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_9(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_11(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_10(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_12(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_11(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_13(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_12(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_14(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_13(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_15(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_14(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_16(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_15(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_17(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_16(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_18(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_17(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_19(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_18(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_20(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_19(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_21(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_20(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_22(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_21(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_23(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_22(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_24(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_23(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_25(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_24(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_26(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_25(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_27(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_26(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_28(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_27(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_29(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_28(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_30(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_29(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_31(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_30(transform, __VA_ARGS__)
#define MCRO_FOR_EACH_VA_ARGS_32(transform, first, ...) transform(first), MCRO_FOR_EACH_VA_ARGS_31(transform, __VA_ARGS__)

/**
 * @brief  iterate over `__VA_ARGS__` with a transformation.
 * 
 * Only 32 iterations allowed because this is a pretty dull method to do such things. On the other hand, please don't
 * have more than 32 arguments for a macro function call. 
 */
#define FOR_EACH_VA_ARGS(transform, ...) \
	PREPROCESSOR_JOIN(MCRO_FOR_EACH_VA_ARGS_, PREPROCESSOR_VA_ARG_COUNT(__VA_ARGS__))(transform, __VA_ARGS__)
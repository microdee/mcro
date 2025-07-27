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

/** @brief Shorten forwarding expression with this macro so one may not need to specify explicit type */
#define FWD(...) Forward<decltype(__VA_ARGS__)>(__VA_ARGS__)
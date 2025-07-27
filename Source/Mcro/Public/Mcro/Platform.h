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
#include "Mcro/Macros.h"

#if PLATFORM_WINDOWS
#define MCRO_CURRENT_PLATFORM Win64

#elif PLATFORM_LINUXARM64
#define MCRO_CURRENT_PLATFORM LinuxArm64

#elif PLATFORM_LINUX
#define MCRO_CURRENT_PLATFORM Linux

#elif PLATFORM_MAC
#define MCRO_CURRENT_PLATFORM Mac

#elif PLATFORM_ANDROID
#define MCRO_CURRENT_PLATFORM Android

#elif PLATFORM_IOS
#define MCRO_CURRENT_PLATFORM IOS

#endif

#define MCRO_CURRENT_PLATFORM_STRING PREPROCESSOR_TO_STRING(MCRO_CURRENT_PLATFORM)
#define MCRO_CURRENT_PLATFORM_TEXT PREPROCESSOR_TO_TEXT(MCRO_CURRENT_PLATFORM)

#if defined(_MSC_VER)
#define MCRO_COMPILER_MSVC 1
#define MCRO_COMPILER_GCC 0
#define MCRO_COMPILER_CLANG 0
#define MCRO_COMPILER MSVC
#elif defined(__GNUC__)
#define MCRO_COMPILER_MSVC 0
#define MCRO_COMPILER_GCC 1
#define MCRO_COMPILER_CLANG 0
#define MCRO_COMPILER GCC
#elif defined(__clang__)
#define MCRO_COMPILER_MSVC 0
#define MCRO_COMPILER_GCC 0
#define MCRO_COMPILER_CLANG 1
#define MCRO_COMPILER CLang
#else
#define MCRO_COMPILER_MSVC 0
#define MCRO_COMPILER_GCC 0
#define MCRO_COMPILER_CLANG 0
#define MCRO_COMPILER Unknown
#endif
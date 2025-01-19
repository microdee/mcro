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

/**
 *	@file
 *	These two are the most useful types in the arsenal of the C++ developer. Use these for dummy types or when it is
 *	easier to have explicit return types than dealing with void specializations in templates.
 */

#pragma once

/** @brief This struct may be only used in decltype or templating situations when definition is not required */
struct FDeclareOnly;

/** @brief This struct may be used for situations where something needs to be returned but it's not meaningful to do so. */
struct MCRO_API FVoid
{
	/** @brief A constructor which accepts any arguments and does nothing with them */
	template <typename... Args>
	FVoid(Args&&...) {}
};

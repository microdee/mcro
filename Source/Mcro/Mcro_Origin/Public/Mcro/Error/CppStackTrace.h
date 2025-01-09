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
#include "Mcro/Error/PlainTextComponent.h"

namespace Mcro::Error
{
	/**
	 * An Error component which stores a C++ stack trace in its message upon construction
	 */
	class MCRO_API FCppStackTrace : public IPlainTextComponent
	{
	public:
		/**
		 *	@param numAdditionalStackFramesToIgnore
		 *	Ignore stack frames which might be irrelevant for the error report
		 *	
		 *	@param fastWalk
		 *	Use fast stack trace walking instead of accurate
		 *	
		 *	@param stackFramesIgnoreDefaultOffset
		 *	A default offset applied to ignore-stack-frames which accounts for the facilities of IError. Only set this
		 *	parameter when you need introspection into IError functions for some very unlikely reasons. Following frames
		 *	are ignored:
		 *	- FCppStackTrace (this constructor)
		 */
		FCppStackTrace(
			int32 numAdditionalStackFramesToIgnore = 0,
			bool fastWalk = !UE_BUILD_DEBUG,
			int32 stackFramesIgnoreDefaultOffset = 1
		);
	};
}

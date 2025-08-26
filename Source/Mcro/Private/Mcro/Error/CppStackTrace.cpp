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

#include "Mcro/Error/CppStackTrace.h"
#include "HAL/PlatformStackWalk.h"
#include "Stats/StatsMisc.h"

namespace Mcro::Error
{
	FCppStackTrace::FCppStackTrace(int32 numAdditionalStackFramesToIgnore, bool fastWalk, int32 stackFramesIgnoreDefaultOffset)
	{
		// Walk the stack and dump it to the allocated memory.
		const SIZE_T stackTraceSize = 65535;
		ANSICHAR* stackTrace = (ANSICHAR*)FMemory::SystemMalloc(stackTraceSize);
		
#if STATS
		SCOPE_LOG_TIME_IN_SECONDS(TEXT_"FPlatformStackWalk::StackWalkAndDump", nullptr)
#endif
		stackTrace[0] = 0;
		FPlatformStackWalk::StackWalkAndDumpEx(
			stackTrace, stackTraceSize, numAdditionalStackFramesToIgnore + stackFramesIgnoreDefaultOffset,
			fastWalk
				? FPlatformStackWalk::EStackWalkFlags::FastStackWalk
				: FPlatformStackWalk::EStackWalkFlags::AccurateStackWalk
		);
		Message = ANSI_TO_TCHAR(stackTrace);
	}
}

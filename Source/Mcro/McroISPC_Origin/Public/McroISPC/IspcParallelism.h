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

extern "C" {
	MCROISPC_API void ISPCLaunch(void** handlePtr, void* f, void* data, int countx, int county, int countz);
	MCROISPC_API void* ISPCAlloc(void** handlePtr, int64_t size, int32_t alignment);
	MCROISPC_API void ISPCSync(void* handle);
}

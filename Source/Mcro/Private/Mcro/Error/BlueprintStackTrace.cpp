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

#include "Mcro/Error/BlueprintStackTrace.h"
#include "UObject/Script.h"
#include "UObject/Stack.h"

namespace Mcro::Error
{
	FBlueprintStackTrace::FBlueprintStackTrace()
	{
#if DO_BLUEPRINT_GUARD
		if (const FBlueprintContextTracker* bpCtxTracker = FBlueprintContextTracker::TryGet())
		{
			TArrayView<const FFrame* const> rawStack = bpCtxTracker->GetCurrentScriptStack();
			TStringBuilder<4096> scriptStack;
			scriptStack << TEXT_"\n\nScript Stack (" << rawStack.Num() << TEXT_" frames)\n";

			for (int32 frame = rawStack.Num() - 1; frame >= 0; --frame)
			{
				rawStack[frame]->GetStackDescription(scriptStack);
				scriptStack << TEXT_"\n";
			}
			Message = *scriptStack;
		}
		else
		{
			Message = TEXT_"Blueprint stack trace was not available in this context";
		}
#else
		Message = TEXT_"Blueprint stack trace is not available in Shipping or Test configurations";
#endif
	}
}

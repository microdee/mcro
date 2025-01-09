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

#include "Mcro/AssertMacros.h"

#include "Mcro/Error.h"
#include "Mcro/Error/BlueprintStackTrace.h"
#include "Mcro/Error/CppStackTrace.h"
#include "Mcro/Error/ErrorManager.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

namespace Mcro::AssertMacros::Detail
{
	void SubmitError(
		EErrorSeverity severity,
		FString const& codeContext,
		bool async, bool important,
		TUniqueFunction<void(IErrorRef const&)>&& extraSetup
	) {
		auto error = IError::Make(new FAssertion())
			->WithSeverity(severity)
			->WithMessage(TEXT("Program has hit an assertion"))
			->WithCodeContext(codeContext)
			->WithCppStackTrace({}, true, 1)
			->WithBlueprintStackTrace({}, IsInGameThread());
		extraSetup(error);
		auto future = FErrorManager::Get().DisplayError(
			error,
			{ .bAsync = async, .bImportantToRead = important }
		);
		if (!async && !IsInGameThread())
			future.Wait();
	}
	
#if WITH_EDITOR

	bool IsRunningPIE()
	{
		return GEditor && GEditor->PlayWorld != nullptr;
	}

	void StopPie()
	{
		if (IsRunningPIE())
		{
			GEditor->RequestEndPlayMap();
		}
	}

#endif
}

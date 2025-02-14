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
#include "Mcro/Error/ErrorManager.h"
#include "Mcro/TextMacros.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

namespace Mcro::AssertMacros::Detail
{
	void SubmitError(
		EErrorSeverity severity,
		FString const& codeContext,
		bool async, bool important,
		TUniqueFunction<void(IErrorRef const&)>&& extraSetup,
		std::source_location const& location
	) {
		auto error = IError::Make(new FAssertion())
			->WithSeverity(severity)
			->WithMessage(TEXT_"Program has hit an assertion")
			->WithCodeContext(codeContext)
			->WithCppStackTrace({}, true, 1)
			->WithLocation(location)
			->WithBlueprintStackTrace({}, IsInGameThread());
		
		extraSetup(error);
		auto future = FErrorManager::Get().DisplayError(error,
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
		if (IsRunningPIE()) GEditor->RequestEndPlayMap();
	}
	
#else
	
	bool IsRunningPIE() { return false; }
	void StopPie() {}

#endif
}

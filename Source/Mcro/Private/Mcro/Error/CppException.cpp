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

#include "Mcro/Error/CppException.h"
#include "Mcro/Error/SErrorDisplay.h"
#include "Mcro/Yaml.h"

namespace Mcro::Error
{
	using namespace Mcro::Yaml;
	
	FCppException::FCppException(std::exception const& input)
		: BaseException(input)
	{
		Message = input.what();
	}

	TSharedRef<SErrorDisplay> FCppException::CreateErrorWidget()
	{
		using namespace Mcro::Slate;
		
		return SNew(SErrorDisplay)
			. Error(SharedThis(this))
			. PostSeverity()
			[
				SNew(SEditableTextBox) / SErrorDisplay::Text(GetExceptionType())
			];
	}

	FStringView FCppException::GetExceptionType() const
	{
		return TEXT_"std::exception";
	}

	void FCppException::SerializeMembers(YAML::Emitter& emitter, bool isRoot) const
	{
		emitter << YAML::Key << "ExceptionType" << YAML::Value << GetExceptionType();
		IError::SerializeMembers(emitter, isRoot);
	}
}

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

#include "Mcro/Error.h"
#include "Mcro/Observable.h"
#include "Mcro/Error/SErrorDisplay.h"
#include "Mcro/Error/PlainTextComponent.h"
#include "Mcro/Error/CppStackTrace.h"
#include "Mcro/Error/BlueprintStackTrace.h"
#include "Mcro/Text.h"
#include <sstream>

namespace Mcro::Error
{
	using namespace Mcro::Text;
	using namespace Mcro::Yaml;
	
	void IError::SerializeInnerErrors(YAML::Emitter& emitter) const
	{
		for (auto const& inner : InnerErrors)
		{
			emitter << YAML::Key << inner.Key << YAML::Value;
			inner.Value->SerializeYaml(emitter, false);
		}
	}

	void IError::SerializeErrorPropagation(YAML::Emitter& emitter) const
	{
		if (!ErrorPropagation.IsEmpty())
		{
			emitter << YAML::Key << "ErrorPropagation" << YAML::Value;
			FSeq seq(emitter);
			for (auto const& at : ErrorPropagation)
			{
				std::stringstream format;
				format << at.function_name() << " @ " << at.file_name() << " : " << at.line();
				emitter << format.str();
			}
		}
	}

	void IError::AddError(const FString& name, const TSharedRef<IError>& error, const FString& typeOverride)
	{
		FString type = typeOverride.IsEmpty() ? error->GetType().ToString() : typeOverride;
		FString key = Join(TEXT(" "), type.Replace(TEXT("::"), TEXT(".")), name);
		FString keyUnique = key;
		for (int i = 1; i <= 100 && InnerErrors.Contains(keyUnique); ++i)
		{
			check(i < 100);
			keyUnique = FString::Printf(TEXT("%s %d"), *key, i);
		}
		InnerErrors[keyUnique] = error;
	}

	void IError::AddAppendix(const FString& name, const FString& text, const FString& type)
	{
		AddError(name, Make(new IPlainTextComponent())->WithMessage(text), type);
	}

	void IError::AddCppStackTrace(const FString& name, int32 numAdditionalStackFramesToIgnore, bool fastWalk)
	{
		AddError(name, Make(new FCppStackTrace(numAdditionalStackFramesToIgnore + 1, fastWalk)));
	}

	void IError::AddBlueprintStackTrace(const FString& name)
	{
		AddError(name, Make(new FBlueprintStackTrace()));
	}

	void IError::SerializeMembers(YAML::Emitter& emitter, bool isRoot) const
	{
		if (isRoot)
			emitter << YAML::Key << "Type" << YAML::Value << TypeName;
			
		if (Severity > EErrorSeverity::ErrorComponent)
			emitter << YAML::Key << "Severity" << YAML::Value << Severity;
		
		if (!Message.IsEmpty())
			emitter << YAML::Key << "Message" << YAML::Value << YAML::Literal << Message;
		
		if (!Details.IsEmpty())
			emitter << YAML::Key << "Details" << YAML::Value << YAML::Literal << Details;
		
		if (!CodeContext.IsEmpty())
			emitter << YAML::Key << "CodeContext" << YAML::Value << YAML::Literal << CodeContext;
	}

	void IError::NotifyState(Observable::IState<IErrorPtr>& state)
	{
		state.Set(SharedThis(this));
	}

	void IError::SerializeYaml(YAML::Emitter& emitter, bool isRoot) const
	{
		FMap errorMap(emitter);
		SerializeMembers(emitter, isRoot);
		SerializeErrorPropagation(emitter);

		if (InnerErrors.Num() > 0)
		{
			emitter << YAML::Key << "InnerErrors" << YAML::Value;
			FMap innerErrors(emitter);
			SerializeInnerErrors(emitter);
		}
	}

	FString IError::ToString() const
	{
		return UTF8_TO_TCHAR(ToStringUtf8().c_str());
	}

	std::string IError::ToStringUtf8() const
	{
		YAML::Emitter output;
		SerializeYaml(output, true);
		return output.c_str();
	}

	TArray<FString> IError::GetErrorPropagation() const
	{
		TArray<FString> result;
		Algo::Transform(ErrorPropagation, result, [&](std::source_location const& at)
		{
			return FString::Printf(TEXT("%s @ %s : %d"),
				ANSI_TO_TCHAR(at.function_name()),
				ANSI_TO_TCHAR(at.file_name()),
				at.line()
			);
		});
		return result;
	}

	FString IError::GetErrorPropagationJoined() const
	{
		return FString::Join(GetErrorPropagation(), TEXT("\n"));
	}

	FStringView IError::GetSeverityString() const
	{
		FStdStringView severityStd(magic_enum::enum_name(Severity));
		return UnrealView(severityStd);
	}

	TSharedRef<SErrorDisplay> IError::CreateErrorWidget()
	{
		return SNew(SErrorDisplay).Error(SharedThis(this));
	}

	FUnavailable::FUnavailable()
	{
		Message = TEXT("Attempted to access a resource which doesn't exist.");
	}
}

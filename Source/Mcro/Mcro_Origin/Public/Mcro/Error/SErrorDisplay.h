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
#include "Widgets/SCompoundWidget.h"
#include "Mcro/Error.h"
#include "Mcro/Slate.h"

namespace Mcro::Error
{
	/** @brief Base class for displaying Mcro::Error::IError objects to the user */
	class MCRO_API SErrorDisplay : public SCompoundWidget
	{
	public:
		
		SLATE_BEGIN_ARGS(SErrorDisplay)
		{}
			SLATE_ARGUMENT(IErrorPtr, Error);
			SLATE_NAMED_SLOT(FArguments, PostSeverity);
			SLATE_NAMED_SLOT(FArguments, PostMessage);
			SLATE_NAMED_SLOT(FArguments, PostDetails);
			SLATE_NAMED_SLOT(FArguments, PostCodeContext);
			SLATE_NAMED_SLOT(FArguments, PostErrorPropagation);
			SLATE_NAMED_SLOT(FArguments, PostInnerErrors);
		SLATE_END_ARGS()

		/** Constructs this widget with InArgs */
		void Construct(const FArguments& inArgs);

		static auto Text(const FString& text)               -> Slate::TAttributeBlock<SEditableTextBox>;
		static auto Text(const FStringView& text)           -> Slate::TAttributeBlock<SEditableTextBox>;
		static auto OptionalText(const FString& text)       -> Slate::TAttributeBlock<SEditableTextBox>;
		static auto OptionalTextWidget(const FString& text) -> TSharedRef<SEditableTextBox>;
		 
		static auto ExpandableText(const FText& title, const FString& text) -> Slate::TAttributeBlock<SExpandableArea>;
		static auto ExpandableTextWidget(const FText& title, const FString& text) -> TSharedRef<SExpandableArea>;
		
		static auto Severity(const IErrorRef& error)        -> Slate::TAttributeBlock<STextBlock>;
		static auto SeverityWidget(const IErrorRef& error)  -> TSharedRef<STextBlock>;

		static auto Row() -> SVerticalBox::FSlot::FSlotArguments;
	};
}

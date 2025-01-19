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

#include "Mcro/Error/ErrorManager.h"
#include "Mcro/Threading.h"
#include "Interfaces/IMainFrameModule.h"
#include "Mcro/Error/SErrorDisplay.h"
#include "Styling/StarshipCoreStyle.h"
#include "HAL/PlatformApplicationMisc.h"

DECLARE_LOG_CATEGORY_CLASS(LogErrorManager, Log, Log);

namespace Mcro::Error
{
	FErrorManager& FErrorManager::Get()
	{
		static FErrorManager Singleton {};
		return Singleton;
	}

	struct FErrorHeaderStyle
	{
		FLinearColor BackgroundColor {};
		FLinearColor FontColor {};
		int32 FontSize = 14;
	};

	auto FErrorManager::DisplayError(IErrorRef const& error, FDisplayErrorArgs const& args) -> TFuture<EDisplayErrorResult>
	{
		UE_LOG(
			LogErrorManager, Error,
			TEXT("Displaying error %s:"),
			*error->GetType().ToString()
		);
		ERROR_LOG(LogErrorManager, Error, error);
		UE_DEBUG_BREAK();

		if (bIsDisplayingError)
		{
			UE_LOG(LogErrorManager, Warning, TEXT(
				"Another error is already being displayed. Suppressing this one."
				" If multiple things can go wrong in quick succession please organize them into one aggregate error,"
				" and display that."
			));

			// TODO: deal with this situation more automatically
			return MakeFulfilledPromise<EDisplayErrorResult>(Suppressed_AnotherErrorOpen).GetFuture();
		}
		bIsDisplayingError = true;
		auto result = Threading::PromiseInGameThread([=, this]
		{
			return DisplayError_MainThread(error, args);
		});
		return result;
	}

	auto FErrorManager::DisplayError_MainThread(IErrorRef const& error, FDisplayErrorArgs const& args) -> EDisplayErrorResult
	{
		using namespace Mcro::Slate;
		
		decltype(auto) slate = FSlateApplication::Get();
		auto canInferParentWidget = [&] { return args.Parent.IsValid() || InferParentWidget().IsValid(); };
		
		if (!slate.CanAddModalWindow() || IsEngineExitRequested() || !canInferParentWidget())
		{
			return Suppressed_CannotDisplayModalWindow;
		}

		TSharedPtr<const SWidget> parent = args.Parent ? args.Parent : InferParentWidget();

		decltype(auto) style = FStarshipCoreStyle::GetCoreStyle();

		auto severity = error->GetSeverityString();
		auto title = FString::Printf(
			TEXT("%s error %s"), severity.GetData(), *error->GetType().ToString()
		);
		
		FErrorHeaderStyle headerStyle;
		switch (error->GetSeverity())
		{
		case EErrorSeverity::ErrorComponent:
			headerStyle = {
				FLinearColor(0.20f, 0.20f, 0.20f, 1.00f),
				FLinearColor(0.66f, 0.66f, 0.66f, 1.00f),
				14
			};
			break;
		case EErrorSeverity::Recoverable:
			headerStyle = {
				FLinearColor(0.13f, 0.37f, 0.14f, 1.00f),
				FLinearColor::White,
				21
			};
			break;
		case EErrorSeverity::Fatal:
			headerStyle = {
				FLinearColor(0.61f, 0.23f, 0.00f, 1.00f),
				FLinearColor::White,
				21
			};
			break;
		case EErrorSeverity::Crashing:
			headerStyle = {
				FLinearColor(0.69f, 0.00f, 0.00f, 1.00f),
				FLinearColor::White,
				21
			};
			break;
		}

		TSharedPtr<SWindow> modalWindow;
		TSharedPtr<SCheckBox> pleaseRead;
		SAssignNew(modalWindow, SWindow)
			. Style(&style.GetWidgetStyle<FWindowStyle>(TEXT("Window")))
			. Title(FText::FromString(title))
			. Type(EWindowType::Normal)
			. AutoCenter(EAutoCenter::PreferredWorkArea)
			. SizingRule(ESizingRule::UserSized)
			. IsTopmostWindow(true)
			. ClientSize({700.f, 700.f})
			[
				SNew(SBox)
				. Padding(FMargin(5.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					. HAlign(HAlign_Center)
					. AutoHeight()
					[
						SNew(STextBlock)
						. SimpleTextMode(true)
						. Text(INVTEXT("(you can still interact with the program while this dialog is open)"))
						. Font(FCoreStyle::GetDefaultFontStyle("Italic", 12))
						. ColorAndOpacity(FLinearColor(0.45f, 0.45f, 0.45f, 1.00f))
						. Visibility(IsVisible(args.bAsync))
					]
					+ SVerticalBox::Slot()
					. HAlign(HAlign_Fill)
					. Padding(FMargin(0.f, 5.f))
					. AutoHeight()
					[
						SNew(SBorder)
						. Padding(FMargin(10.f, 14.f))
						. BorderImage(new FSlateColorBrush(headerStyle.BackgroundColor))
						[
							SNew(STextBlock)
							. Font(FCoreStyle::GetDefaultFontStyle("BoldItalic", headerStyle.FontSize))
							. ColorAndOpacity(headerStyle.FontColor)
							. SimpleTextMode(true)
							. Text(FText::FromString(title))
						]
					]
					+ SVerticalBox::Slot()
					. HAlign(HAlign_Fill)
					. Padding(FMargin(0.f, 5.f))
					. AutoHeight()
					[
						SNew(STextBlock)
						. Text(INVTEXT(
							"Unfortunately this application has ran into a problem it could not handle automatically."
							" There can be a wide spectrum of reasons which this error summary aims to narrow down."
							" Please examine ít carefully and patiently. While reporting this error DO NOT send (only)"
							" the screenshot of this dialog box, but use the \"Copy Error to Clipboard\" button!"
							"\nThank you for your patience, understanding and cooperation!"
						))
					]
					+ SVerticalBox::Slot()
					. HAlign(HAlign_Fill)
					. VAlign(VAlign_Fill)
					[
						SNew(SScrollBox)
						. ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
						+ SScrollBox::Slot()
						. HAlign(HAlign_Fill)
						. AutoSize()
						[
							error->CreateErrorWidget()
						]
					]
					+ SVerticalBox::Slot()
					. HAlign(HAlign_Fill)
					. AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						. HAlign(HAlign_Left)
						. AutoWidth()
						[
							SAssignNew(pleaseRead, SCheckBox)
							. Type(ESlateCheckBoxType::Type::CheckBox)
							. Visibility(IsVisible(args.bImportantToRead))
							. Content()
							[
								SNew(STextBlock)
								. SimpleTextMode(true)
								. Text(INVTEXT("I have read the error summary."))
							]
						]
						+ SHorizontalBox::Slot()
						. HAlign(HAlign_Left)
						. AutoWidth()
						[
							SNew(SButton)
							. Text(INVTEXT("Dismiss"))
							. ToolTipText_Lambda([weakPleaseRead = pleaseRead.ToWeakPtr(), important = args.bImportantToRead]
							{
								if (auto pleaseRead = weakPleaseRead.Pin())
								if (important && !pleaseRead->IsChecked())
									return INVTEXT(
										"Please confirm that you have read this error summary by ticking the checkbox"
										" to the left."
									);
								return INVTEXT("Once done reading dismiss this error summary.");
							})
							. IsEnabled_Lambda([weakPleaseRead = pleaseRead.ToWeakPtr(), important = args.bImportantToRead]
							{
								if (auto pleaseRead = weakPleaseRead.Pin())
									return pleaseRead->IsChecked() || !important;
								return true;
							})
							. OnClicked_Lambda([weakWindow = modalWindow.ToWeakPtr()]
							{
								if (auto window = weakWindow.Pin())
									window->RequestDestroyWindow();
								return FReply::Handled();
							})
						]
						+ SHorizontalBox::Slot()
						. HAlign(HAlign_Right)
						. AutoWidth()
						[
							SNew(SButton)
							. Text(INVTEXT("Copy Error to Clipboard"))
							. ToolTipText(INVTEXT("The error is copied in its entirety formatted as YAML plain text."))
							. OnClicked_Lambda([error]
							{
								FPlatformApplicationMisc::ClipboardCopy(*error->ToString());
								return FReply::Handled();
							})
						]
					]
				]
			]
		;

		slate.AddModalWindow(modalWindow.ToSharedRef(), parent, args.bAsync);
		
		bIsDisplayingError = false;
		return Displayed;
	}

	auto FErrorManager::InferParentWidget() -> TSharedPtr<const SWidget>
	{
		TSharedPtr<SWindow> parentWindow {};
#if WITH_EDITOR
		if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
		{
			IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
			parentWindow = MainFrame.GetParentWindow();
		}
#else
		auto gameViewport = GEngine ? GEngine->GameViewport : nullptr;
		parentWindow = gameViewport ? gameViewport->GetWindow() : nullptr;
#endif
		return parentWindow;
	}
}

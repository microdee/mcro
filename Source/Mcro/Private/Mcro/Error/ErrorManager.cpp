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
#include "Mcro/Error/SErrorDisplay.h"
#include "Mcro/Threading.h"
#include "Mcro/FmtMacros.h"
#include "Mcro/Range.h"
#include "Mcro/Range/Conversion.h"
#include "Mcro/Range/Views.h"

#include "HAL/PlatformApplicationMisc.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/StarshipCoreStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Brushes/SlateColorBrush.h"

#if WITH_EDITOR
#include "Interfaces/IMainFrameModule.h"
#endif

DECLARE_LOG_CATEGORY_CLASS(LogErrorManager, Log, Log);

namespace Mcro::Error
{
	using namespace Mcro::Delegates::InferDelegate;
	using namespace Mcro::Range;
	
	FErrorManager& FErrorManager::Get()
	{
		static FErrorManager Singleton {};
		return Singleton;
	}

	struct FErrorHeaderStyle
	{
		FColor BackgroundColor {};
		FColor FontColor {};
		int32 FontSize = 14;
	};

	auto FErrorManager::DisplayError(IErrorRef const& error, FDisplayErrorArgs const& args) -> TFuture<EDisplayErrorResult>
	{
		UE_CLOG(
			args.bLogError,
			LogErrorManager, Error,
			TEXT_"Displaying error %s:",
			*error->GetType().ToStringCopy()
		);
		ERROR_CLOG(args.bLogError && !bIsDisplayingError, LogErrorManager, Error, error);
		if (args.bBreakDebugger)
		{
			UE_DEBUG_BREAK();
		}

		if (bIsDisplayingError)
		{
			UE_LOG(LogErrorManager, Warning, TEXT_
				"Another error is already being displayed. Suppressing this one."
				" If multiple things can go wrong in quick succession please organize them into one aggregate error,"
				" and display that."
			);

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
		namespace rv = ranges::views;
		
		decltype(auto) slate = FSlateApplication::Get();
		auto canInferParentWidget = [&] { return args.Parent.IsValid() || InferParentWidget().IsValid(); };
		
		if (!slate.CanAddModalWindow() || IsEngineExitRequested() || !canInferParentWidget())
		{
			return Suppressed_CannotDisplayModalWindow;
		}

		TSharedPtr<const SWidget> parent = args.Parent ? args.Parent : InferParentWidget();
		
		auto allExtensions = IErrorWindowExtension::GetAll();
		auto extensions = allExtensions
			| rv::filter([error, &args](IErrorWindowExtension* i) { return i->SupportsError(error, args); })
			| RenderAs<TArray>()
		;

		decltype(auto) style = FStarshipCoreStyle::GetCoreStyle();

		auto title = TEXT_"{0} error {1}" _FMT(error->GetSeverity(), error->GetType());
		
		FErrorHeaderStyle headerStyle;
		switch (error->GetSeverity())
		{
		case EErrorSeverity::ErrorComponent:
			headerStyle = {
				FColor(51, 51, 51, 255),
				FColor(169, 169, 169, 255),
				14
			};
			break;
		case EErrorSeverity::Recoverable:
			headerStyle = {
				FColor(32, 94, 36, 255),
				FColor::White,
				21
			};
			break;
		case EErrorSeverity::Fatal:
			headerStyle = {
				FColor(203, 72, 0, 255),
				FColor::White,
				21
			};
			break;
		case EErrorSeverity::Crashing:
			headerStyle = {
				FColor(177, 0, 0, 255),
				FColor::White,
				21
			};
			break;
		}

		TSharedPtr<SCheckBox> pleaseRead;
		SAssignNew(ModalWindow, SWindow)
			. Style(&style.GetWidgetStyle<FWindowStyle>(TEXT_"Window"))
			. Title(FText::FromString(title))
			. Type(EWindowType::Normal)
			. AutoCenter(EAutoCenter::PreferredWorkArea)
			. SizingRule(ESizingRule::UserSized)
			. IsTopmostWindow(true)
			. HasCloseButton(false)
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
						. Text(INVTEXT_"(you can still interact with the program while this dialog is open)")
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
						. BorderImage(new FSlateColorBrush(FLinearColor::FromSRGBColor(headerStyle.BackgroundColor)))
						[
							SNew(STextBlock)
							. Font(FCoreStyle::GetDefaultFontStyle("BoldItalic", headerStyle.FontSize))
							. ColorAndOpacity(FLinearColor::FromSRGBColor(headerStyle.FontColor))
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
						. AutoWrapText(true)
						. Text(INVTEXT_
							"Unfortunately this application has ran into a problem it could not handle automatically."
							" There can be a wide spectrum of reasons which this error summary aims to narrow down."
							" Please examine ít carefully and patiently. While reporting this error DO NOT send (only)"
							" the screenshot of this dialog box, but use the \"Copy Error to Clipboard\" button!"
							"\nThank you for your patience, understanding and cooperation!"
						)
					]
					+ TSlots(
						extensions
							| rv::transform([error, &args](IErrorWindowExtension* i)
							{
								return i->PreErrorDisplay(error, args);
							})
							| FilterValid()
						,
						[](TSharedPtr<SWidget> const& i)
						{
							return MoveTemp(
								SVerticalBox::Slot()
								. VAlign(VAlign_Fill)
								. Padding(FMargin(0.f, 5.f))
								. AutoHeight()
								[ i.ToSharedRef() ]
							);
						}
					)
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
					+ TSlots(
						extensions
							| rv::transform([error, &args](IErrorWindowExtension* i)
							{
								return i->PostErrorDisplay(error, args);
							})
							| FilterValid()
						,
						[](TSharedPtr<SWidget> const& i)
						{
							return MoveTemp(
								SVerticalBox::Slot()
								. VAlign(VAlign_Fill)
								. Padding(FMargin(0.f, 5.f))
								. AutoHeight()
								[ i.ToSharedRef() ]
							);
						}
					)
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
								. Text(INVTEXT_"I have read the error summary.")
							]
						]
						+ SHorizontalBox::Slot()
						. HAlign(HAlign_Fill)
						[ SNew(SSpacer) ]
						+ SHorizontalBox::Slot()
						. HAlign(HAlign_Right)
						. AutoWidth()
						[
							SNew(SButton)
							. Text(INVTEXT_"Copy Error to Clipboard")
							. ToolTipText(INVTEXT_"The error is copied in its entirety formatted as YAML plain text.")
							. OnClicked_Lambda([error]
							{
								FPlatformApplicationMisc::ClipboardCopy(*error->ToString());
								return FReply::Handled();
							})
						]
						+ SHorizontalBox::Slot()
						. HAlign(HAlign_Left)
						. AutoWidth()
						[
							SNew(SButton)
							. Text(INVTEXT_"Dismiss")
							. ToolTipText_Lambda([weakPleaseRead = pleaseRead.ToWeakPtr(), important = args.bImportantToRead]
							{
								if (auto pleaseRead = weakPleaseRead.Pin())
									if (important && !pleaseRead->IsChecked())
										return INVTEXT_
											"Please confirm that you have read this error summary by ticking the checkbox"
											" to the left."
										;
									return INVTEXT_"Once done reading, dismiss this error summary.";
							})
							. IsEnabled_Lambda([weakPleaseRead = pleaseRead.ToWeakPtr(), important = args.bImportantToRead]
							{
								if (auto pleaseRead = weakPleaseRead.Pin())
									return pleaseRead->IsChecked() || !important;
								return true;
							})
							. OnClicked_Lambda([this]
							{
								ModalWindow->RequestDestroyWindow();
								return FReply::Handled();
							})
						]
					]
				]
			]
		;
		
		ModalWindow->SetOnWindowClosed(From([this](TSharedRef<SWindow> const& window)
		{
			ModalWindow.Reset();
			bIsDisplayingError = false;
			OnErrorDialogDismissed.Broadcast();
		}));
		
		if (args.bAsync)
			slate.AddWindow(ModalWindow.ToSharedRef(), true);
		else
			slate.AddModalWindow(ModalWindow.ToSharedRef(), parent, false);

		return Displayed;
	}

	auto FErrorManager::InferParentWidget() -> TSharedPtr<const SWidget>
	{
		TSharedPtr<SWindow> parentWindow = FSlateApplication::Get().GetActiveTopLevelRegularWindow();
		if (parentWindow) return parentWindow;
		
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

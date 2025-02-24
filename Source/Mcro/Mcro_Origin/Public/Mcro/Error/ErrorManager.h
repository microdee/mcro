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
#include "Mcro/Error.h"
#include "Mcro/Delegates/EventDelegate.h"
#include "Mcro/AutoModularFeature.h"

namespace Mcro::Error
{
	using namespace Mcro::AutoModularFeature;

	/** @brief Control how an error is being displayed. Use C++ 20 designated initializers for convenience */
	struct FDisplayErrorArgs
	{
		/**
		 *	@brief
		 *	The error message will not block the engine tick. This is useful for errors happening in the editor
		 *	so even if PIE session is aborted due to an error, the developer can cross-check their assets with the
		 *	error still open.
		 */
		bool bAsync = false;

		/** @brief Enables an extra checkbox which reminds the user to please do not immediately dismiss the error */
		bool bImportantToRead = false;

		/**
		 *	@brief
		 *	Set this to false if for any reason you don't need the debugger to break before displaying the error
		 *	to the user.
		 */
		bool bBreakDebugger = true;

		/**
		 *	@brief
		 *	Set this to false if for any reason you don't need the error to be logged before displaying it to the user.
		 */
		bool bLogError = true;

		/**
		 *	@brief
		 *	Optionally set a parent widget for the modal window of the error. By default if not specified here the
		 *	main editor window is used, or the main gameplay viewport.
		 */
		TSharedPtr<const SWidget> Parent = {};
	};

	/**
	 *	@brief
	 *	A modular feature which allows other modules to inject their own UI into error windows displayed to the user.
	 */
	class MCRO_API IErrorWindowExtension : public TAutoModularFeature<IErrorWindowExtension>
	{
	public:
		virtual bool SupportsError(IErrorRef const& error, FDisplayErrorArgs const& displayArgs) { return true; };
		virtual TSharedPtr<SWidget> PreErrorDisplay(IErrorRef const& error, FDisplayErrorArgs const& displayArgs) { return {}; };
		virtual TSharedPtr<SWidget> PostErrorDisplay(IErrorRef const& error, FDisplayErrorArgs const& displayArgs) { return {}; };
	};
	
	/** @brief Global facilities for IError handling, including displaying them to the user, trigger error events, etc */
	class MCRO_API FErrorManager
	{
	public:
	
		/** @brief Get the global singleton */
		static FErrorManager& Get();

		/** @brief The results of displaying an error. In all cases the error is logged. */
		enum EDisplayErrorResult
		{
			/** @brief The error has been displayed for the user. */
			Displayed,

			/** @brief The error has not been shown to the user because another error is already being shown. */
			Suppressed_AnotherErrorOpen,

			/** @brief Modal windows couldn't be created at the time, so we couldn't show it to the user either. */
			Suppressed_CannotDisplayModalWindow,
		};

		/**
		 *	@brief
		 *	Display the error summary for the user. Only use this when your program arrives to an unrecoverable state
		 *	which either needs explanation for the user or requires action from the user (like configuration changes).
		 *
		 *	@important
		 *	The modal window and the widgets representing the error will be created on the main thread, keep that in
		 *	mind while making the widgets for the errors.
		 *	
		 *	@param error  The input error
		 *	@param args   Simple arguments object for this function, use initializer list or C++ 20 designated initializer.
		 *	
		 *	@return
		 *	A future telling that either the dialog has been displayed or how it has been suppressed. The future also
		 *	gives an opportunity to block the calling thread until the user acknowledges the error.
		 *	
		 *	@remarks
		 *	Unless `bAsync` is set in the arguments, calling this function from any thread will also block the main
		 *	thread while the modal window containing the error is open. If the calling thread also needs to be blocked
		 *	then simply wait on the returned future.
		 *
		 *	@todo
		 *	Add ability to let the user "ignore" errors, and continue execution, because they know better.
		 */
		auto DisplayError(IErrorRef const& error, FDisplayErrorArgs const& args) -> TFuture<EDisplayErrorResult>;

		TEventDelegate<void()> OnErrorDialogDismissed;

	private:
		
		auto DisplayError_MainThread(IErrorRef const& error, FDisplayErrorArgs const& args) -> EDisplayErrorResult;
		auto InferParentWidget() -> TSharedPtr<const SWidget>;

		TSharedPtr<SWindow> ModalWindow;
		FThreadSafeBool bIsDisplayingError;
	};
}

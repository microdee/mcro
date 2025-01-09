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

namespace Mcro::Error
{
	/** Global facilities for IError handling, including displaying them to the user, trigger error events, etc */
	class MCRO_API FErrorManager
	{
	public:
	
		/** Get the global singleton */
		static FErrorManager& Get();

		/** The results of displaying an error. In all cases the error is logged. */
		enum EDisplayErrorResult
		{
			/** The error has been displayed for the user. */
			Displayed,

			/** The error has not been shown to the user because another error is already being shown. */
			Suppressed_AnotherErrorOpen,

			/** Modal windows couldn't be created at the time, so we couldn't show it to the user either. */
			Suppressed_CannotDisplayModalWindow,
		};

		/** Control how an error is being displayed. Use C++ 20 designated initializers for convenience */
		struct FDisplayErrorArgs
		{
			/**
			 *	The error message will not block the engine tick. This is useful for errors happening in the editor
			 *	so even if PIE session is aborted due to an error, the developer can cross-check their assets with the
			 *	error still open.
			 */
			bool bAsync = false;

			/** Enables an extra checkbox which reminds the user to please do not immediately dismiss the error */
			bool bImportantToRead = false;

			/**
			 *	Optionally set a parent widget for the modal window of the error. By default if not specified here the
			 *	main editor window is used, or the main gameplay viewport.
			 */
			TSharedPtr<const SWidget> Parent = {};
		};

		/**
		 *	Display the error summary for the user. Only use this when your program arrives to an unrecoverable state
		 *	which either needs explanation for the user or requires action from the user (like configuration changes).
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
		 *	Add ability to let the user "ignore" errors, and continue execution.
		 */
		auto DisplayError(IErrorRef const& error, FDisplayErrorArgs const& args) -> TFuture<EDisplayErrorResult>;

	private:
		
		auto DisplayError_MainThread(IErrorRef const& error, FDisplayErrorArgs const& args) -> EDisplayErrorResult;
		auto InferParentWidget() -> TSharedPtr<const SWidget>;
		
		FThreadSafeBool bIsDisplayingError;
	};
}

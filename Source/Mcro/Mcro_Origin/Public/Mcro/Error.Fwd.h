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
#include "Mcro/Concepts.h"

/** @brief Contains utilities for structured error handling */
namespace Mcro::Error
{
	using namespace Mcro::Concepts;
	
	class IError;
	class SErrorDisplay;
	
	using IErrorRef = TSharedRef<IError>;   /**< @brief Convenience alias for an instance of an error */
	using IErrorPtr = TSharedPtr<IError>;   /**< @brief Convenience alias for an instance of an error */
	using IErrorWeakPtr = TWeakPtr<IError>; /**< @brief Convenience alias for an instance of an error */

	using FNamedError = TPair<FString, IErrorRef>;

	/** @brief Concept constraining input type argument T to an IError */
	template <typename T>
	concept CError = CDerivedFrom<T, IError>;

	/** @brief Concept constraining input type argument T to be an IErrorRef */
	template <typename T>
	concept CErrorRef = CSharedRef<T> && CError<typename T::ElementType>;

	/** @brief Concept constraining input type argument T to be an IErrorPtr */
	template <typename T>
	concept CErrorPtr = CSharedPtr<T> && CError<typename T::ElementType>;

	/** @brief Concept constraining input type argument T to be an IErrorPtr */
	template <typename T>
	concept CErrorRefOrPtr = CErrorRef<T> || CErrorPtr<T>;

	/** @brief Concept constraining input type argument T to be an IErrorPtr */
	template <typename T>
	concept CSharedError = CErrorRefOrPtr<T> || (CWeakPtr<T> && CError<typename T::ElementType>);

	/** @brief Indicate the severity of an error and at what discretion the caller may treat it. */
	enum class EErrorSeverity
	{
		/** @brief Indicates that an inner error just contains extra context for a real error */
		ErrorComponent = -1,
		
		/** @brief The caller can handle the error and may continue execution, for example errors with telemetry. */
		Recoverable,
		
		/**
		 *	@brief
		 *	A sub-program (like PIE) or a thread should abort its entire purpose but it should not crash the entire
		 *	encompassing application, for example early runtime checks about the correctness of some required configuration.
		 */
		Fatal,

		/**
		 *	@brief
		 *	The application has arrived to an invalid state from which recovery is impossible, for example access
		 *	violation errors.
		 */
		Crashing
	};
}
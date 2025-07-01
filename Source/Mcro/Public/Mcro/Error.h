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
#include "Mcro/Error.Fwd.h"
#include "Void.h"
#include "Mcro/Types.h"
#include "Mcro/Concepts.h"
#include "Mcro/SharedObjects.h"
#include "Mcro/Observable.Fwd.h"
#include "Mcro/TextMacros.h"
#include "Mcro/Text.h"
#include "Mcro/Delegates/EventDelegate.h"

#include "Mcro/LibraryIncludes/Start.h"
#include "yaml-cpp/yaml.h"
#include "Mcro/LibraryIncludes/End.h"

#include <source_location>

/** Contains utilities for structured error handling */
namespace Mcro::Error
{
	using namespace Mcro::Text;
	using namespace Mcro::Types;
	using namespace Mcro::FunctionTraits;
	using namespace Mcro::SharedObjects;
	using namespace Mcro::Delegates;

	/**
	 *	@brief  A base class for a structured error handling and reporting with modular architecture and fluent API.
	 *	
	 *	@important
	 *	Instantiate errors only with `IError::Make(new FMyError())` this ensures the minimal runtime reflection features.
	 *	
	 *	Many times runtime errors are unavoidable and if an API only gives indication of success or failure (let's say in
	 *	the form of a boolean) that will be quite frustrating for the user to report, as it gives no direction of course
	 *	what went wrong, how it went wrong, and when it went wrong. Slightly better when the API gives a list of things
	 *	what can go wrong and return an item from that list when things go wrong. This of course still doesn't allow to
	 *	provide much of a context for the user.
	 *	
	 *	An 'improvement' to that is using C++ exceptions, however it is not unanimously well received in the community
	 *	because it can hide the fact that the API can bail on its caller. So when exceptions are enabled one may call
	 *	every function of an API like if they were walking on a minefield. For this (and a couple more) reasons C++
	 *	exceptions are disabled by default in Unreal projects and viewed as terrible practice to introduce it ourselves.
	 *	
	 *	Unreal does provide though the `TValueOrError` template which allows API's to indicate that they can fail in some
	 *	ways without the need to consult an external documentation. It gives the developer total freedom however of what
	 *	the error can be, so on its own it does not solve the questions of what/when/how.
	 *	
	 *	Using `TMaybe` with IError can be a powerful tool in the developer's arsenal when creating a library.
	 *	IError can standardize a detailed and structured way of communicating errors without hindering call-site
	 *	usage. It can also automate the method and the format of logging the (many times excessive amount of)
	 *	information surrounding an error, or decide how it may be presented for the user.
	 */
	class MCRO_API IError : public IHaveTypeShareable
	{
	protected:
		TMap<FString, IErrorRef> InnerErrors;
		TArray<std::source_location> ErrorPropagation;
		EErrorSeverity Severity = EErrorSeverity::ErrorComponent;
		FString Message;
		FString Details;
		FString CodeContext;
		mutable bool bIsRoot = false;

		/** @brief Override this method if inner errors needs custom way of serialization */
		virtual void SerializeInnerErrors(YAML::Emitter& emitter) const;
		
		/** @brief Override this method if error propagation history needs custom way of serialization */
		virtual void SerializeErrorPropagation(YAML::Emitter& emitter) const;

		/** @brief Override this method if inner errors added to current one needs special attention */
		virtual void AddError(const FString& name, const TSharedRef<IError>& error, const FString& typeOverride = {});

		/** @brief Add extra separate blocks of text in an ad-hoc fashion */
		virtual void AddAppendix(const FString& name, const FString& text, const FString& type = TEXT_"Appendix");

		void AddCppStackTrace(const FString& name, int32 numAdditionalStackFramesToIgnore, bool fastWalk);
		void AddBlueprintStackTrace(const FString& name);

		/**
		 * 	@brief
		 *	Override this method if direct members should be serialized differently or extra members are added by
		 *	derived errors.
		 */
		virtual void SerializeMembers(YAML::Emitter& emitter) const;

		virtual void NotifyState(Observable::IState<IErrorPtr>& state);
		
	public:
		
		FORCEINLINE decltype(InnerErrors)::TRangedForIterator      begin()       { return InnerErrors.begin(); }
		FORCEINLINE decltype(InnerErrors)::TRangedForConstIterator begin() const { return InnerErrors.begin(); }
		FORCEINLINE decltype(InnerErrors)::TRangedForIterator      end()         { return InnerErrors.end(); }
		FORCEINLINE decltype(InnerErrors)::TRangedForConstIterator end()   const { return InnerErrors.end(); }
		
		/**
		 * 	@brief
		 *	This is an empty function so any IError can fulfill CSharedInitializeable without needing extra
		 *	attention in derived classes. Simply hide this function with overloads in derived classes if they need
		 *	to use TSharedFromThis for initialization
		 */
		void Initialize() {};

		/**
		 * 	@brief
		 *	Override this function to change the method how this error is entirely serialized into a YAML format
		 *	
		 *	@param emitter  the YAML node into which the data of this error needs to be appended to
		 */
		virtual void SerializeYaml(YAML::Emitter& emitter) const;

		/** @brief Overload append operator for YAML::Emitter */
		friend auto operator << (YAML::Emitter& emitter, IErrorRef const& error) -> YAML::Emitter&;

		/** @brief Render this error as a string using the YAML representation */
		FString ToString() const;

		/** @brief Render this error as a std::string using the YAML representation */
		std::string ToStringUtf8() const;
		
		/**
		 *	@brief  To ensure automatic type reflection use IError::Make instead of manually constructing error objects
		 *	
		 *	@code
		 *	IError::Make(new FMyError(myConstructorArgs), myInitializerArgs);
		 *	@endcode
		 *	
		 *	@tparam        T  Type of new error
		 *	@tparam     Args  Arguments for the new error initializer.
		 *	@param  newError  Pass the new object in as `new FMyError(...)`
		 *	@param      args  Arguments for the new error initializer.
		 */
		template <CError T, typename... Args>
		requires CSharedInitializeable<T, Args...>
		static TSharedRef<T> Make(T* newError, Args&&... args)
		{
			return MakeShareableInit(newError, FWD(args)...)->WithType();
		}

		/**
		 *	@brief
		 *	When `Report` is called on an error this event is triggered, allowing issue tracking systems to react on
		 *	IError's which are deemed "ready".
		 *
		 *	`ERROR_LOG`, `ERROR_CLOG` and `FErrorManager::DisplayError` automatically report their input error.
		 */
		static auto OnErrorReported() -> TEventDelegate<void(IErrorRef)>&;

		FORCEINLINE EErrorSeverity                  GetSeverity() const        { return Severity; }
		FORCEINLINE int32                           GetSeverityInt() const     { return static_cast<int32>(Severity); }
		FORCEINLINE FString const&                  GetMessage() const         { return Message; }
		FORCEINLINE FString const&                  GetDetails() const         { return Details; }
		FORCEINLINE FString const&                  GetCodeContext() const     { return CodeContext; }
		FORCEINLINE TMap<FString, IErrorRef> const& GetInnerErrors() const     { return InnerErrors; }
		FORCEINLINE int32                           GetInnerErrorCount() const { return InnerErrors.Num(); }

		/**
		 *	@brief
		 *	Get a list of source locations where this error has been handled. This is not equivalent of stack-traces but
		 *	rather a historical record of where this error was considered throughout the source code. Each item in this
		 *	list is explicitly recorded via WithLocation. The first item is the earliest consideration of this error.
		 */
		TArray<FString> GetErrorPropagation() const;

		/** @brief Same as `GetErrorPropagation` but items are separated by new line. */
		FString GetErrorPropagationJoined() const;

		/** @brief Get the error severity as an unreal string. */
		FStringView GetSeverityString() const;

		/** @brief Override this function to customize how an error is displaxed for the end-user */
		virtual TSharedRef<SErrorDisplay> CreateErrorWidget();

		/**
		 *	@brief   Specify error message with a fluent API
		 *	@tparam       Self  Deducing this
		 *	@param        self  Deduced this (not present in calling arguments)
		 *	@param       input  the message
		 *	@param   condition  Only add message when this condition is satisfied
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self>
		SelfRef<Self> WithMessage(this Self&& self, const FString& input, bool condition = true)
		{
			if (condition) self.Message = input;
			return self.SharedThis(&self);
		}

		/**
		 *	@brief   Specify formatted error message with a fluent API
		 *	@tparam     Self  Deducing this
		 *	@param      self  Deduced this (not present in calling arguments)
		 *	@param     input  the message format
		 *	@param   fmtArgs  ordered format arguments
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self, CStringFormatArgument... FormatArgs>
		SelfRef<Self> WithMessageF(this Self&& self, const TCHAR* input, FormatArgs&&... fmtArgs)
		{
			self.Message = FString::Format(input, OrderedArguments(FWD(fmtArgs)...));
			return self.SharedThis(&self);
		}

		/**
		 *	@brief   Specify formatted error message with a fluent API
		 *	@tparam       Self  Deducing this
		 *	@param        self  Deduced this (not present in calling arguments)
		 *	@param   condition  Only add message when this condition is satisfied
		 *	@param       input  the message format
		 *	@param     fmtArgs  format arguments
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self, typename... FormatArgs>
		SelfRef<Self> WithMessageFC(this Self&& self, bool condition, const TCHAR* input, FormatArgs&&... fmtArgs)
		{
			if (condition) self.Message = FString::Format(input, OrderedArguments(FWD(fmtArgs)...));
			return self.SharedThis(&self);
		}
		
		/**
		 *	@brief   Specify severity with a fluent API
		 *	@tparam   Self  Deducing this
		 *	@param    self  Deduced this (not present in calling arguments)
		 *	@param   input  the severity
		 *	@return  Self for further fluent API setup
		 *	@see     EErrorSeverity
		 */
		template <typename Self>
		SelfRef<Self> WithSeverity(this Self&& self, EErrorSeverity input)
		{
			self.Severity = input;
			return self.SharedThis(&self);
		}

		/** @brief Recoverable shorthand */
		template <typename Self>
		SelfRef<Self> AsRecoverable(this Self&& self)
		{
			self.Severity = EErrorSeverity::Recoverable;
			return self.SharedThis(&self);
		}

		/** @brief Fatal shorthand */
		template <typename Self>
		SelfRef<Self> AsFatal(this Self&& self)
		{
			self.Severity = EErrorSeverity::Fatal;
			return self.SharedThis(&self);
		}

		/** @brief Crashing shorthand */
		template <typename Self>
		SelfRef<Self> AsCrashing(this Self&& self)
		{
			self.Severity = EErrorSeverity::Crashing;
			return self.SharedThis(&self);
		}

		/**
		 *	@brief 
		 *	Specify details for the error which may provide further context for the user or provide them
		 *	reminders/suggestions
		 *	
		 *	@tparam       Self  Deducing this
		 *	@param        self  Deduced this (not present in calling arguments)
		 *	@param       input  the details text
		 *	@param   condition  Only add details when this condition is satisfied
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self>
		SelfRef<Self> WithDetails(this Self&& self, const FString& input, bool condition = true)
		{
			if (condition) self.Details = input;
			return self.SharedThis(&self);
		}

		/**
		 *	@brief
		 *	Specify formatted details for the error which may provide further context for the user or provide them
		 *	reminders/suggestions
		 *	
		 *	@tparam     Self  Deducing this
		 *	@param      self  Deduced this (not present in calling arguments)
		 *	@param     input  the details text
		 *	@param   fmtArgs  ordered format arguments
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self, CStringFormatArgument... FormatArgs>
		SelfRef<Self> WithDetailsF(this Self&& self, const TCHAR* input, FormatArgs&&... fmtArgs)
		{
			self.Details = FString::Format(input, OrderedArguments(FWD(fmtArgs)...));
			return self.SharedThis(&self);
		}

		/**
		 *	@brief
		 *	Specify formatted details for the error which may provide further context for the user or provide them
		 *	reminders/suggestions
		 *	
		 *	@tparam       Self  Deducing this
		 *	@param        self  Deduced this (not present in calling arguments)
		 *	@param       input  the details text
		 *	@param   condition  Only add details when this condition is satisfied
		 *	@param     fmtArgs  ordered format arguments
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self, CStringFormatArgument... FormatArgs>
		SelfRef<Self> WithDetailsFC(this Self&& self, bool condition, const TCHAR* input, FormatArgs&&... fmtArgs)
		{
			if (condition) self.Details = FString::Format(input, OrderedArguments(FWD(fmtArgs)...));
			return self.SharedThis(&self);
		}

		/**
		 *	@brief   If available write a source code context into the error directly displaying where this error has occured
		 *	@tparam       Self  Deducing this
		 *	@param        self  Deduced this (not present in calling arguments)
		 *	@param       input  the source code context
		 *	@param   condition  Only add code context when this condition is satisfied
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self>
		SelfRef<Self> WithCodeContext(this Self&& self, const FString& input, bool condition = true)
		{
			if (condition) self.CodeContext = input;
			return self.SharedThis(&self);
		}

		/**
		 *	@brief   Add a uniquely typed inner error.
		 *	@tparam       Self  Deducing this
		 *	@tparam      Error  Deduced type of the error
		 *	@param        self  Deduced this (not present in calling arguments)
		 *	@param       input  Inner error
		 *	@param   condition  Only add inner error when this condition is satisfied
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self, CError Error>
		SelfRef<Self> WithError(this Self&& self, const TSharedRef<Error>& input, bool condition = true)
		{
			if (condition) self.AddError({}, input);
			return self.SharedThis(&self);
		}

		/**
		 *	@brief   Add one inner error with specific name.
		 *	@tparam       Self  Deducing this
		 *	@tparam      Error  Deduced type of the error
		 *	@param        self  Deduced this (not present in calling arguments)
		 *	@param        name  Optional name of the error. If it's empty only the type of the error will be used for ID
		 *	@param       input  Inner error
		 *	@param   condition  Only add inner error when this condition is satisfied
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self, CError Error>
		SelfRef<Self> WithError(this Self&& self, const FString& name, const TSharedRef<Error>& input, bool condition = true)
		{
			if (condition) self.AddError(name, input);
			return self.SharedThis(&self);
		}

		/**
		 *	@brief   Add multiple errors at once with optional names
		 *	@tparam       Self  Deducing this
		 *	@param       input  An array of tuples with otional error name and the error itself
		 *	@param   condition  Only add errors when this condition is satisfied
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self>
		SelfRef<Self> WithErrors(this Self&& self, const TArray<TTuple<FString, IErrorRef>>& input, bool condition = true)
		{
			if (condition)
			{
				for (const auto& error : input)
					self.AddError(error.Key, error.Value);
			}
			return self.SharedThis(&self);
		}

		/**
		 *	@brief   Add multiple errors at once
		 *	@tparam    Self  Deducing this
		 *	@tparam  Errors  Deduced type of the errors
		 *	@param   errors  Errors to be added
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self, CError... Errors>
		SelfRef<Self> WithErrors(this Self&& self, const TSharedRef<Errors>&... errors)
		{
			(self.AddError({}, errors), ...);
			return self.SharedThis(&self);
		}

		/**
		 *	@brief   Add multiple errors at once
		 *	@tparam       Self  Deducing this
		 *	@tparam     Errors  Deduced type of the errors
		 *	@param      errors  Errors to be added
		 *	@param   condition  Only add errors when this condition is satisfied
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self, CError... Errors>
		SelfRef<Self> WithErrors(this Self&& self, bool condition, const TSharedRef<Errors>&... errors)
		{
			if (condition) self.WithErrors(errors...);
			return self.SharedThis(&self);
		}

		/**
		 *	@brief   Add an extra plain text block inside inner errors
		 *	@tparam       Self  Deducing this
		 *	@param        name  Name of the extra text block
		 *	@param        text  Value of the extra text block
		 *	@param   condition  Only add inner error when this condition is satisfied
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self>
		SelfRef<Self> WithAppendix(this Self&& self, const FString& name, const FString& text, bool condition = true)
		{
			if (condition) self.AddAppendix(name, text);
			return self.SharedThis(&self);
		}

		/**
		 *	@brief   Add an extra plain text block inside inner errors
		 *	@tparam     Self  Deducing this
		 *	@param      name  Name of the extra text block
		 *	@param      text  Value of the extra text block
		 *	@param   fmtArgs  ordered format arguments
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self, CStringFormatArgument... FormatArgs>
		SelfRef<Self> WithAppendixF(this Self&& self, const FString& name, const TCHAR* text, FormatArgs&&... fmtArgs)
		{
			self.AddAppendix(name, FString::Format(text, OrderedArguments(FWD(fmtArgs)...)));
			return self.SharedThis(&self);
		}

		/**
		 *	@brief   Add an extra plain text block inside inner errors
		 *	@tparam       Self  Deducing this
		 *	@param        name  Name of the extra text block
		 *	@param        text  Value of the extra text block
		 *	@param     fmtArgs  ordered format arguments
		 *	@param   condition  Only add inner error when this condition is satisfied
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self, CStringFormatArgument... FormatArgs>
		SelfRef<Self> WithAppendixFC(this Self&& self, bool condition, const FString& name, const TCHAR* text, FormatArgs&&... fmtArgs)
		{
			if (condition)
				self.AddAppendix(name, FString::Format(text, OrderedArguments(FWD(fmtArgs)...)));
			return self.SharedThis(&self);
		}

		/** @brief Notify an observable state about this error */
		template <typename Self>
		SelfRef<Self> Notify(this Self&& self, Observable::IState<IErrorPtr>& state)
		{
			self.NotifyState(state);
			return self.SharedThis(&self);
		}

		/** @brief Break if a debugger is attached when this error is created */
		template <typename Self>
		SelfRef<Self> BreakDebugger(this Self&& self)
		{
			UE_DEBUG_BREAK();
			return self.SharedThis(&self);
		}

		/** @brief Shorthand for adding the current C++ stacktrace to this error */
		template <typename Self>
		SelfRef<Self> WithCppStackTrace(this Self&& self, const FString& name = {}, bool condition = true, int32 numAdditionalStackFramesToIgnore = 0, bool fastWalk = !UE_BUILD_DEBUG)
		{
#if !UE_BUILD_SHIPPING
			if (condition)
				self.AddCppStackTrace(name, numAdditionalStackFramesToIgnore + 1, fastWalk);
#endif
			return self.SharedThis(&self);
		}

		/** @brief Shorthand for adding the current Blueprint stacktrace to this error */
		template <typename Self>
		SelfRef<Self> WithBlueprintStackTrace(this Self&& self, const FString& name = {}, bool condition = true)
		{
#if !UE_BUILD_SHIPPING
			if (condition)
				self.AddBlueprintStackTrace(name);
#endif
			return self.SharedThis(&self);
		}

		/**
		 *	@brief
		 *	Allow the error to record the source locations it has been handled at compile time. For example this gives
		 *	more information than stack-traces because it can also record where errors were handled between parallel 
		 *	threads.
		 *	
		 *	@tparam      Self  Deducing this
		 *	@param   location  The location this error is handled at. In 99% of cases this should be left at the default
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self>
		SelfRef<Self> WithLocation(this Self&& self, std::source_location const& location = std::source_location::current())
		{
			self.ErrorPropagation.Add(location);
			return self.SharedThis(&self);
		}

		/**
		 *	@brief
		 *	Report this error to the global `IError::OnErrorReported` event once it is deemed "ready", so an issue
		 *	tracking system can handle important error unintrusively. Make sure this is the last modification on the
		 *	error. Only call it once no further information can be added and on the top-most main error in case of
		 *	aggregate errors.
		 *	
		 *	@tparam       Self  Deducing this
		 *	@param   condition  Only report errors when this condition is satisfied
		 *	@return  Self for further fluent API setup
		 */
		template <typename Self>
		SelfRef<Self> Report(this Self&& self, bool condition = true)
		{
			if (condition)
				OnErrorReported().Broadcast(self.SharedThis(&self));
			
			return self.SharedThis(&self);
		}

		/**
		 *	@brief
		 *	Call an arbitrary function with this error. Other than cases like invoking macros inline operating on
		 *	errors (like the inline ERROR_LOG overload) this may have limited use.
		 *	
		 *	@tparam     Self  Deducing this. 
		 *	@tparam Function  Function to call with this error. 
		 *	@param      self  Deducing this. 
		 *	@param  function  Function to call with this error. 
		 *	@return  Self for further fluent API setup
		 */
		template <
			typename Self,
			CFunctionCompatible_ArgumentsDecay<void(SelfRef<Self>)> Function
		>
		SelfRef<Self> AsOperandWith(this Self&& self, Function&& function)
		{
			auto sharedSelf = self.SharedThis(&self);
			function(sharedSelf);
			return sharedSelf;
		}
	};

	/** @brief A simple error type for checking booleans. It adds no extra features to IError */
	class MCRO_API FAssertion : public IError {};

	/**
	 *	@brief
	 *	A simple error type denoting that whatever is being accessed is not available like attempting to access nullptr.
	 *	It adds no extra features to IError
	 */
	class MCRO_API FUnavailable : public IError
	{
	public:
		FUnavailable();
	};

	/**
	 *	@brief
	 *	A `TValueOrError` alternative for IError which allows implicit conversion from values and errors (no need for
	 *	`MakeError` or `MakeValue`) and is boolean testable. It also doesn't have ambiguous state such as
	 *	`TValueOrError` has, so a TMaybe will always have either an error or a value, it will never have neither of
	 *	them or both of them.
	 */
	template <CNonVoid T>
	struct TMaybe
	{
		using ValueType = T;

		/**
		 *	@brief
		 *	Default initializing a TMaybe while its value is not default initializable, initializes the resulting
		 *	TMaybe to an erroneous state.
		 */
		template <typename = T>
		requires (!CDefaultInitializable<T>)
		TMaybe() : Error(IError::Make(new FUnavailable())
			->WithMessageF(
				TEXT_"TMaybe has been default initialized, but a Value of {0} cannot be default initialized",
				TTypeName<T>
			)
		) {}

		/** @brief If T is default initializable then the default state of TMaybe will be the default value of T, and not an error */
		template <CDefaultInitializable = T>
		TMaybe() : Value(T{}) {}
		
		/** @brief Enable copy constructor for T only when T is copy constructable */
		template <CConvertibleToDecayed<T> From, CCopyConstructible = T>
		TMaybe(From const& value) : Value(value) {}
		
		/** @brief Enable move constructor for T only when T is move constructable */
		template <CConvertibleToDecayed<T> From, CMoveConstructible = T>
		TMaybe(From&& value) : Value(FWD(value)) {}
		
		/** @brief Enable copy constructor for TMaybe only when T is copy constructable */
		template <CConvertibleToDecayed<T> From, CCopyConstructible = T>
		TMaybe(TMaybe<From> const& other) : Value(other.Value) {}
		
		/** @brief Enable move constructor for TMaybe only when T is move constructable */
		template <CConvertibleToDecayed<T> From, CMoveConstructible = T>
		TMaybe(TMaybe<From>&& other) : Value(MoveTemp(other.Value)) {}

		/** @brief Set this TMaybe to an erroneous state */
		template <CError ErrorType>
		TMaybe(TSharedRef<ErrorType> const& error) : Error(error) {}

		bool HasValue() const { return Value.IsSet(); }
		bool HasError() const { return Error.IsValid(); }

		auto TryGetValue()       -> TOptional<T>&       { return Value; }
		auto TryGetValue() const -> TOptional<T> const& { return Value; }
		
		auto GetValue()       -> T&       { return Value.GetValue(); }
		auto GetValue() const -> T const& { return Value.GetValue(); }

		T&& StealValue() && { return MoveTemp(Value.GetValue()); }

		auto GetError() const -> IErrorPtr { return Error; }
		auto GetErrorRef() const -> IErrorRef { return Error.ToSharedRef(); }

		operator bool() const { return HasValue(); }

		/**
		 *	@brief  Modify a potential error stored in this monad
		 *	@tparam     Self  Deducing this
		 *	@tparam Function  Modifying function type
		 *	@param      self  Deducing this
		 *	@param       mod  Input function modifying a potential error
		 *	@return  Self, preserving qualifiers.
		 */
		template <typename Self, CFunctionCompatible_ArgumentsDecay<void(IErrorRef)> Function>
		Self&& ModifyError(this Self&& self, Function&& mod)
		{
			if (self.HasError()) mod(self.GetErrorRef());
			return FWD(self);
		}
		
		operator TValueOrError<T, IErrorPtr>() const
		{
			if (HasValue())
				return MakeValue(Value.GetValue());
			return MakeError(Error);
		}

	private:
		TOptional<T> Value;
		IErrorPtr Error;
	};

	/** @brief Indicate that an otherwise void function that it may fail with an IError. */
	using FCanFail = TMaybe<FVoid>;

	/**
	 *	@brief
	 *	Syntactically same as `FCanFail` but for functions which is explicitly used to query some boolean decidable
	 *	thing, and which can also provide a reason why the queried thing is false. 
	 */
	using FTrueOrReason = TMaybe<FVoid>;

	/** @brief Return an FCanFail or FTrueOrReason indicating a success or truthy output */
	FORCEINLINE FCanFail Success() { return FVoid(); }
}

#define MCRO_ERROR_LOG_3(categoryName, verbosity, error) \
	UE_LOG(categoryName, verbosity, TEXT_"%s", *((error) \
		->WithLocation()                                 \
		->Report()                                       \
		->ToString()                                     \
	))                                                  //

#define MCRO_ERROR_LOG_2(categoryName, verbosity)                         \
	Report()                                                              \
	->AsOperandWith([](IErrorRef const& error)                            \
	{                                                                     \
		UE_LOG(categoryName, verbosity, TEXT_"%s", *(error->ToString())); \
	})                                                                   //

/**
 *	@brief  Convenience macro for logging an error with UE_LOG
 *
 *	The following overloads are available:
 *	- `(categoryName, verbosity, error)` Simply use UE_LOG to log the error
 *	- `(categoryName, verbosity)` Log the error preceding this macro (use ERROR_LOG inline).
 *	  `WithLocation` is omitted from this overload
 */
#define ERROR_LOG(...) MACRO_OVERLOAD(MCRO_ERROR_LOG_, __VA_ARGS__)

#define ERROR_CLOG(condition, categoryName, verbosity, error)        \
	UE_CLOG(condition, categoryName, verbosity, TEXT_"%s", *((error) \
		->WithLocation()                                             \
		->Report()                                                   \
		->ToString()                                                 \
	))                                                              //

#define MCRO_ASSERT_RETURN_2(condition, error)                  \
	if (UNLIKELY(!(condition)))                                 \
		return Mcro::Error::IError::Make(new error)             \
			->WithLocation()                                    \
			->AsRecoverable()                                   \
			->WithCodeContext(PREPROCESSOR_TO_TEXT(condition)) //

#define MCRO_ASSERT_RETURN_1(condition) MCRO_ASSERT_RETURN_2(condition, Mcro::Error::FAssertion())

/**
 *	@brief  Similar to check() macro, but return an error instead of crashing
 *	
 *	The following overloads are available:
 *	- `(condition, error)` Specify error type to return
 *	- `(condition)` Return `FAssertion` error
 */
#define ASSERT_RETURN(...) MACRO_OVERLOAD(MCRO_ASSERT_RETURN_, __VA_ARGS__)

#define MCRO_UNAVAILABLE_1(error)                                                              \
	return Mcro::Error::IError::Make(new DEFAULT_ON_EMPTY(error, Mcro::Error::FUnavailable())) \
		->WithLocation()                                                                       \
		->AsRecoverable()                                                                     //

/**
 *	@brief  Denote that a resource which is asked for doesn't exist
 *	
 *	The following overloads are available:
 *	- `(error)` Specify error type to return
 *	- `()` Return `FUnavailable` error
 */
#define UNAVAILABLE(error) MCRO_UNAVAILABLE_1(error)

#define MCRO_PROPAGATE_FAIL_3(type, var, expression)    \
	type var = (expression);                            \
	if (UNLIKELY(var.HasError())) return var.GetError() \
		->WithLocation()                               //

#define MCRO_PROPAGATE_FAIL_2(var, expression) MCRO_PROPAGATE_FAIL_3(auto, var, expression)
#define MCRO_PROPAGATE_FAIL_1(expression) MCRO_PROPAGATE_FAIL_2(PREPROCESSOR_JOIN(tempResult, __LINE__), expression)

/**
 *	@brief
 *	If a function returns a TMaybe or an FCanFail inside another function which may also return another error use this
 *	convenience macro to propagate the failure.
 *
 *	The following overloads are available:
 *	- `(expression)` Use this when the value of success is not needed further below. This will create a local variable
 *	  with an automatically determined name and type
 *	- `(var, expression)` Use this when the value of success will be used further below. The local variable will be
 *	  created with `auto`
 *	- `(type, var, expression)` Use this when the value of success will be used further below and its local variable
 *	  should have an explicit type
 */
#define PROPAGATE_FAIL(...) MACRO_OVERLOAD(MCRO_PROPAGATE_FAIL_, __VA_ARGS__)
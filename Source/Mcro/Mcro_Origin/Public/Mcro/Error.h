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
#include "Mcro/Yaml.h"

#include <source_location>

/** Contains utilities for structured error handling */
namespace Mcro::Error
{
	using namespace Mcro::Text;
	using namespace Mcro::Types;
	using namespace Mcro::FunctionTraits;
	using namespace Mcro::SharedObjects;

	/**
	 *	A base class for a structured error handling and reporting with modular architecture and fluent API.
	 *	
	 *	@important
	 *	Instantiate errors only with `IError::Make(new FMyError())` this ensures the minimal runtime reflection features.
	 *	
	 *	@remarks
	 *	Many times runtime errors are unavoidable and if an API only gives indication of success or failure (let's say in
	 *	the form of a boolean) that will be quite frustrating for the user to report, as it gives no direction of course
	 *	what went wrong, how it went wrong, and when it went wrong. Slightly better when the API gives a list of things
	 *	what can go wrong and return an item from that list when things go wrong. This of course still doesn't allow to
	 *	provide much of a context for the user.
	 *	@remarks
	 *	An 'improvement' to that is using C++ exceptions, however it is not unanimously well received in the community
	 *	because it can hide the fact that the API can bail on its caller. So when exceptions are enabled one may call
	 *	every function of an API like if they were walking on a minefield. For this (and a couple more) reasons C++
	 *	exceptions are disabled by default in Unreal projects and viewed as terrible practice to introduce it ourselves.
	 *	@remarks
	 *	Unreal does provide though the `TValueOrError` template which allows API's to indicate that they can fail in some
	 *	ways without the need to consult an external documentation. It gives the developer total freedom however of what
	 *	the error can be, so on its own it does not solve the questions of what/when/how.
	 *	@remarks
	 *	Using `TMaybe` with `IError` can be a powerful tool in the developer's arsenal when creating a library.
	 *	`IError` can standardize a detailed and structured way of communicating errors without hindering call-site
	 *	usage. It can also automate the method and the format of logging the (many times excessive amount of)
	 *	information surrounding an error, or decide how it may be presented for the user.
	 */
	class MCRO_API IError : public IHaveType
	{
	protected:
		TMap<FString, IErrorRef> InnerErrors;
		TArray<std::source_location> ErrorPropagation;
		EErrorSeverity Severity = EErrorSeverity::ErrorComponent;
		FString Message;
		FString Details;
		FString CodeContext;

		/** Override this method if inner errors needs custom way of serialization */
		virtual void SerializeInnerErrors(YAML::Emitter& emitter) const;
		
		/** Override this method if error propagation history needs custom way of serialization */
		virtual void SerializeErrorPropagation(YAML::Emitter& emitter) const;

		/** Override this method if inner errors added to current one needs special attention */
		virtual void AddError(const FString& name, const TSharedRef<IError>& error, const FString& typeOverride = {});

		/** Add extra separate blocks of text in an ad-hoc fashion */
		virtual void AddAppendix(const FString& name, const FString& text, const FString& type = TEXT("Appendix"));

		void AddCppStackTrace(const FString& name, int32 numAdditionalStackFramesToIgnore, bool fastWalk);
		void AddBlueprintStackTrace(const FString& name);

		/**
		 *	Override this method if direct members should be serialized differently or extra members are added by
		 *	derived errors.
		 */
		virtual void SerializeMembers(YAML::Emitter& emitter, bool isRoot) const;

		virtual void NotifyState(Observable::IState<IErrorPtr>& state);
		
	public:
		
		FORCEINLINE decltype(InnerErrors)::TRangedForIterator      begin()       { return InnerErrors.begin(); }
		FORCEINLINE decltype(InnerErrors)::TRangedForConstIterator begin() const { return InnerErrors.begin(); }
		FORCEINLINE decltype(InnerErrors)::TRangedForIterator      end()         { return InnerErrors.end(); }
		FORCEINLINE decltype(InnerErrors)::TRangedForConstIterator end()   const { return InnerErrors.end(); }
		
		/**
		 *	This is an empty function so any `IError` can fulfill `CSharedInitializeable` without needing extra
		 *	attention in derived classes. Simply hide this function with overloads in derived classes if they need
		 *	to use TSharedFromThis for initioalization
		 */
		void Initialize() {};

		/**
		 *	Override this function to change the method how this error is entirely serialized into a YAML format
		 *	@param emitter  the YAML node into which the data of this error needs to be appended to
		 *	@param isRoot   true when the top level error is being serialized
		 */
		virtual void SerializeYaml(YAML::Emitter& emitter, bool isRoot) const;

		/** Render this error as a string using the YAML representation */
		FString ToString() const;

		/** Render this error as a std::string using the YAML representation */
		std::string ToStringUtf8() const;
		
		/**
		 *	To ensure automatic type reflection use IError::Make instead of manually constructing error objects
		 *	@code
		 *	IError::Make(new FMyError(myConstructorArgs), myInitializerArgs);
		 *	@endcode
		 *	@tparam T        Type of new error
		 *	@tparam Args     Arguments for the new error initializer.
		 *	@param newError  Pass the new object in as `new FMyError(...)`
		 *	@param args      Arguments for the new error initializer.
		 *	@return 
		 */
		template <CError T, typename... Args>
		requires CSharedInitializeable<T, Args...>
		static TSharedRef<T> Make(T* newError, Args&&... args)
		{
			return MakeShareableInit(newError, Forward<Args>(args)...)->WithType();
		}

		FORCEINLINE EErrorSeverity                  GetSeverity() const        { return Severity; }
		FORCEINLINE FString const&                  GetMessage() const         { return Message; }
		FORCEINLINE FString const&                  GetDetails() const         { return Details; }
		FORCEINLINE FString const&                  GetCodeContext() const     { return CodeContext; }
		FORCEINLINE TMap<FString, IErrorRef> const& GetInnerErrors() const     { return InnerErrors; }
		FORCEINLINE int32                           GetInnerErrorCount() const { return InnerErrors.Num(); }

		/**
		 *	Get a list of source locations where this error has been handled. This is not equivalent of stack-traces but
		 *	rather a historical record of where this error was considered throughout the source code. Each item in this
		 *	list is explicitly recorded via `WithLocation`. The first item is the earliest consideration of this error.
		 */
		TArray<FString> GetErrorPropagation() const;

		/** Same as `GetErrorPropagation` but items are separated by new line. */
		FString GetErrorPropagationJoined() const;

		/** Get the error severity as an unreal string. */
		FStringView GetSeverityString() const;

		/** Override this function to customize how an error is displaxed for the end-user */
		virtual TSharedRef<SErrorDisplay> CreateErrorWidget();

		/**
		 *	Specify error     message with a fluent API
		 *	@tparam Self      Deducing this
		 *	@param self       Deduced this (not present in calling arguments)
		 *	@param input      the message
		 *	@param condition  Only add message when this condition is satisfied
		 *	@return       Self for further fluent API setup
		 */
		template <typename Self>
		SelfRef<Self> WithMessage(this Self&& self, const FString& input, bool condition = true)
		{
			if (condition) self.Message = input;
			return self.SharedThis(&self);
		}

		/**
		 *	Specify formatted error message with a fluent API
		 *	@tparam Self    Deducing this
		 *	@param self     Deduced this (not present in calling arguments)
		 *	@param input    the message format
		 *	@param fmtArgs  format arguments
		 *	@return       Self for further fluent API setup
		 */
		template <typename Self, typename... FormatArgs>
		SelfRef<Self> WithMessageF(this Self&& self, const TCHAR* input, FormatArgs&&... fmtArgs)
		{
			self.Message = DynamicPrintf(input, Forward<FormatArgs>(fmtArgs)...);
			return self.SharedThis(&self);
		}

		/**
		 *	Specify formatted error message with a fluent API
		 *	@tparam Self      Deducing this
		 *	@param self       Deduced this (not present in calling arguments)
		 *	@param condition  Only add message when this condition is satisfied
		 *	@param input      the message format
		 *	@param fmtArgs    format arguments
		 *	@return       Self for further fluent API setup
		 */
		template <typename Self, typename... FormatArgs>
		SelfRef<Self> WithMessageFC(this Self&& self, bool condition, const TCHAR* input, FormatArgs&&... fmtArgs)
		{
			if (condition) self.Message = DynamicPrintf(input, Forward<FormatArgs>(fmtArgs)...);
			return self.SharedThis(&self);
		}
		
		/**
		 *	Specify severity with a fluent API
		 *	@tparam Self  Deducing this
		 *	@param self   Deduced this (not present in calling arguments)
		 *	@param input  the severity
		 *	@return       Self for further fluent API setup
		 *	@see  Mcro::Error::EErrorSeverity
		 */
		template <typename Self>
		SelfRef<Self> WithSeverity(this Self&& self, EErrorSeverity input)
		{
			self.Severity = input;
			return self.SharedThis(&self);
		}

		/** Recoverable shorthand */
		template <typename Self>
		SelfRef<Self> AsRecoverable(this Self&& self)
		{
			self.Severity = EErrorSeverity::Recoverable;
			return self.SharedThis(&self);
		}

		/** Fatal shorthand */
		template <typename Self>
		SelfRef<Self> AsFatal(this Self&& self)
		{
			self.Severity = EErrorSeverity::Fatal;
			return self.SharedThis(&self);
		}

		/** Crashing shorthand */
		template <typename Self>
		SelfRef<Self> AsCrashing(this Self&& self)
		{
			self.Severity = EErrorSeverity::Crashing;
			return self.SharedThis(&self);
		}

		/**
		 *	Specify details for the error which may provide further context for the user or provide them
		 *	reminders/suggestions
		 *	@tparam Self      Deducing this
		 *	@param self       Deduced this (not present in calling arguments)
		 *	@param input      the details text
		 *	@param condition  Only add details when this condition is satisfied
		 *	@return           Self for further fluent API setup
		 */
		template <typename Self>
		SelfRef<Self> WithDetails(this Self&& self, const FString& input, bool condition = true)
		{
			if (condition) self.Details = input;
			return self.SharedThis(&self);
		}

		/**
		 *	Specify formatted details for the error which may provide further context for the user or provide them
		 *	reminders/suggestions
		 *	@tparam Self    Deducing this
		 *	@param self     Deduced this (not present in calling arguments)
		 *	@param input    the details text
		 *	@param fmtArgs  format arguments
		 *	@return         Self for further fluent API setup
		 */
		template <typename Self, typename... FormatArgs>
		SelfRef<Self> WithDetailsF(this Self&& self, const TCHAR* input, FormatArgs&&... fmtArgs)
		{
			self.Details = DynamicPrintf(input, Forward<FormatArgs>(fmtArgs)...);
			return self.SharedThis(&self);
		}

		/**
		 *	Specify formatted details for the error which may provide further context for the user or provide them
		 *	reminders/suggestions
		 *	@tparam Self      Deducing this
		 *	@param self       Deduced this (not present in calling arguments)
		 *	@param input      the details text
		 *	@param condition  Only add details when this condition is satisfied
		 *	@param fmtArgs    format arguments
		 *	@return           Self for further fluent API setup
		 */
		template <typename Self, typename... FormatArgs>
		SelfRef<Self> WithDetailsFC(this Self&& self, bool condition, const TCHAR* input, FormatArgs&&... fmtArgs)
		{
			if (condition) self.Details = DynamicPrintf(input, Forward<FormatArgs>(fmtArgs)...);
			return self.SharedThis(&self);
		}

		/**
		 *	If available write a source code context into the error directly displaying where this error has occured
		 *	@tparam Self      Deducing this
		 *	@param self       Deduced this (not present in calling arguments)
		 *	@param input      the source code context
		 *	@param condition  Only add code context when this condition is satisfied
		 *	@return           Self for further fluent API setup
		 */
		template <typename Self>
		SelfRef<Self> WithCodeContext(this Self&& self, const FString& input, bool condition = true)
		{
			if (condition) self.CodeContext = input;
			return self.SharedThis(&self);
		}

		/**
		 *	Add a uniquely typed inner error.
		 *	@tparam Self      Deducing this
		 *	@tparam Error     Deduced type of the error
		 *	@param self       Deduced this (not present in calling arguments)
		 *	@param input      Inner error
		 *	@param condition  Only add inner error when this condition is satisfied
		 *	@return           Self for further fluent API setup
		 */
		template <typename Self, CError Error>
		SelfRef<Self> WithError(this Self&& self, const TSharedRef<Error>& input, bool condition = true)
		{
			if (condition) self.AddError({}, input);
			return self.SharedThis(&self);
		}

		/**
		 *	Add one inner error with specific name.
		 *	@tparam Self      Deducing this
		 *	@tparam Error     Deduced type of the error
		 *	@param self       Deduced this (not present in calling arguments)
		 *	@param name       Optional name of the error. If it's empty only the type of the error will be used for ID
		 *	@param input      Inner error
		 *	@param condition  Only add inner error when this condition is satisfied
		 *	@return           Self for further fluent API setup
		 */
		template <typename Self, CError Error>
		SelfRef<Self> WithError(this Self&& self, const FString& name, const TSharedRef<Error>& input, bool condition = true)
		{
			if (condition) self.AddError(name, input);
			return self.SharedThis(&self);
		}

		/**
		 *	Add multiple errors at once with optional names
		 *	@tparam Self      Deducing this
		 *	@param input      An array of tuples with otional error name and the error itself
		 *	@param condition  Only add errors when this condition is satisfied
		 *	@return           Self for further fluent API setup
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
		 *	Add multiple errors at once
		 *	@tparam Self    Deducing this
		 *	@tparam Errors  Deduced type of the errors
		 *	@param errors   Errors to be added
		 *	@return         Self for further fluent API setup
		 */
		template <typename Self, CError... Errors>
		SelfRef<Self> WithErrors(this Self&& self, const TSharedRef<Errors>&... errors)
		{
			(self.AddError({}, errors), ...);
			return self.SharedThis(&self);
		}

		/**
		 *	Add multiple errors at once
		 *	@tparam Self      Deducing this
		 *	@tparam Errors    Deduced type of the errors
		 *	@param errors     Errors to be added
		 *	@param condition  Only add errors when this condition is satisfied
		 *	@return           Self for further fluent API setup
		 */
		template <typename Self, CError... Errors>
		SelfRef<Self> WithErrors(this Self&& self, bool condition, const TSharedRef<Errors>&... errors)
		{
			if (condition) self.WithErrors(errors...);
			return self.SharedThis(&self);
		}

		/**
		 *	Add an extra plain text block inside inner errors
		 *	@tparam Self      Deducing this
		 *	@param name       Name of the extra text block
		 *	@param text       Value of the extra text block
		 *	@param condition  Only add inner error when this condition is satisfied
		 *	@return           Self for further fluent API setup
		 */
		template <typename Self>
		SelfRef<Self> WithAppendix(this Self&& self, const FString& name, const FString& text, bool condition = true)
		{
			if (condition) self.AddAppendix(name, text);
			return self.SharedThis(&self);
		}

		/**
		 *	Add an extra plain text block inside inner errors
		 *	@tparam Self    Deducing this
		 *	@param name     Name of the extra text block
		 *	@param text     Value of the extra text block
		 *	@param fmtArgs  format arguments
		 *	@return         Self for further fluent API setup
		 */
		template <typename Self, typename... FormatArgs>
		SelfRef<Self> WithAppendixF(this Self&& self, const FString& name, const TCHAR* text, FormatArgs&&... fmtArgs)
		{
			self.AddAppendix(name, DynamicPrintf(text, Forward<FormatArgs>(fmtArgs)...));
			return self.SharedThis(&self);
		}

		/**
		 *	Add an extra plain text block inside inner errors
		 *	@tparam Self      Deducing this
		 *	@param name       Name of the extra text block
		 *	@param text       Value of the extra text block
		 *	@param fmtArgs    format arguments
		 *	@param condition  Only add inner error when this condition is satisfied
		 *	@return           Self for further fluent API setup
		 */
		template <typename Self, typename... FormatArgs>
		SelfRef<Self> WithAppendixFC(this Self&& self, bool condition, const FString& name, const TCHAR* text, FormatArgs&&... fmtArgs)
		{
			if (condition)
				self.AddAppendix(name, DynamicPrintf(text, Forward<FormatArgs>(fmtArgs)...));
			return self.SharedThis(&self);
		}

		/** Notify an observable state about this error */
		template <typename Self>
		SelfRef<Self> Notify(this Self&& self, Observable::IState<IErrorPtr>& state)
		{
			self.NotifyState(state);
			return self.SharedThis(&self);
		}

		/** Break if a debugger is attached when this error is created */
		template <typename Self>
		SelfRef<Self> BreakDebugger(this Self&& self)
		{
			MCRO_DEBUG_BREAK();
			return self.SharedThis(&self);
		}

		/** Shorthand for adding the current C++ stacktrace to this error */
		template <typename Self>
		SelfRef<Self> WithCppStackTrace(this Self&& self, const FString& name = {}, bool condition = true, int32 numAdditionalStackFramesToIgnore = 0, bool fastWalk = !UE_BUILD_DEBUG)
		{
#if !UE_BUILD_SHIPPING
			if (condition)
				self.AddCppStackTrace(name, numAdditionalStackFramesToIgnore + 1, fastWalk);
#endif
			return self.SharedThis(&self);
		}

		/** Shorthand for adding the current Blueprint stacktrace to this error */
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
		 *	Allow the error to record the source locations it has been handled at compile time. For example this gives
		 *	more information than stack-traces because it can also record where errors were handled between parallel 
		 *	threads.
		 *	@tparam Self     Deducing this
		 *	@param location  The location this error is handled at. In 99% of cases this should be left at the default
		 *	@return          Self for further fluent API setup
		 */
		template <typename Self>
		SelfRef<Self> WithLocation(this Self&& self, std::source_location location = std::source_location::current())
		{
			self.ErrorPropagation.Add(location);
			return self.SharedThis(&self);
		}
	};

	/** A simple error type for checking booleans. It adds no extra features to IError */
	class MCRO_API FAssertion : public IError {};

	/**
	 *	A simple error type denoting that whatever is being accessed is not available like attempting to access nullptr.
	 *	It adds no extra features to IError
	 */
	class MCRO_API FUnavailable : public IError
	{
	public:
		FUnavailable();
	};

	/**
	 *	A TValueOrError alternative for IError which allows implicit conversion from values and errors (no need for
	 *	`MakeError` or `MakeValue`) and is boolean testable. It also doesn't have ambiguous state such as TValueOrError
	 *	has, so a TMaybe will always have either an error or a value, it will never have neither of them or both of them.
	 */
	template <CNonVoid T>
	struct TMaybe
	{
		using ValueType = T;

		/**
		 *	Default initializing a TMaybe while its value is not default initializable, initializes the resulting
		 *	TMaybe to an erroneous state.
		 */
		template <typename = T>
		requires (!CDefaultInitializable<T>)
		TMaybe() : Error(IError::Make(new FUnavailable())
			->WithMessageF(
				TEXT("TMaybe has been default initialized, but a Value of %s cannot be default initialized"),
				*TTypeString<T>
			)
		) {}

		/** If T is default initializable then the default state of TMaybe will be the default value of T, and not an error */
		template <CDefaultInitializable = T>
		TMaybe() : Value(T{}) {}
		
		/** Enable copy constructor for T only when T is copy constructable */
		template <CConvertibleToDecayed<T> From, CCopyConstructible = T>
		TMaybe(From const& value) : Value(value) {}
		
		/** Enable move constructor for T only when T is move constructable */
		template <CConvertibleToDecayed<T> From, CMoveConstructible = T>
		TMaybe(From&& value) : Value(Forward<From>(value)) {}
		
		/** Enable copy constructor for TMaybe only when T is copy constructable */
		template <CConvertibleToDecayed<T> From, CCopyConstructible = T>
		TMaybe(TMaybe<From> const& other) : Value(other.Value) {}
		
		/** Enable move constructor for TMaybe only when T is move constructable */
		template <CConvertibleToDecayed<T> From, CMoveConstructible = T>
		TMaybe(TMaybe<From>&& other) : Value(MoveTemp(other.Value)) {}

		/** Set this TMaybe to an erroneous state */
		template <CError ErrorType>
		TMaybe(TSharedRef<ErrorType> const& error) : Error(error) {}

		bool HasValue() const { return Value.IsSet(); }
		bool HasError() const { return Error.IsValid(); }

		auto TryGetValue()       -> TOptional<T>&       { return Value; }
		auto TryGetValue() const -> TOptional<T> const& { return Value; }
		
		auto GetValue()       -> T&       { return Value.GetValue(); }
		auto GetValue() const -> T const& { return Value.GetValue(); }

		auto GetError() const -> IErrorPtr { return Error; }

		operator bool() const { return HasValue(); }
		
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

	/** Indicate that an otherwise void function that it may fail with an `IError`. */
	using FCanFail = TMaybe<FVoid>;

	/**
	 *	Syntactically same as `FCanFail` but for functions which is explicitly used to query some boolean decidable
	 *	thing, and which can also provide a reason why the queried thing is false. 
	 */
	using FTrueOrReason = TMaybe<FVoid>;

	/** Return an FCanFail or FTrueOrReason indicating a success or truthy output */
	FORCEINLINE FCanFail Success() { return FVoid(); }
}

#define ERROR_LOG(categoryName, verbosity, error)         \
	UE_LOG(categoryName, verbosity, TEXT("%s"), *((error) \
		->WithLocation()                                  \
		->ToString()                                      \
	))                                                   //

#define ERROR_CLOG(condition, categoryName, verbosity, error)         \
	UE_CLOG(condition, categoryName, verbosity, TEXT("%s"), *((error) \
		->WithLocation()                                              \
		->ToString()                                                  \
	))                                                               //

/** Similar to check() macro, but return an error instead of crashing */
#define ASSERT_RETURN(condition)                                        \
	if (UNLIKELY(!(condition)))                                         \
		return Mcro::Error::IError::Make(new Mcro::Error::FAssertion()) \
			->WithLocation()                                            \
			->AsRecoverable()                                           \
			->WithCodeContext(PREPROCESSOR_TO_TEXT(condition))         //

/** Denote that a resource which is asked for doesn't exist */
#define UNAVAILABLE()                                                 \
	return Mcro::Error::IError::Make(new Mcro::Error::FUnavailable()) \
		->WithLocation()                                              \
		->AsRecoverable()                                            //

/**
 *	If a function returns a TMaybe<V> inside another function which may also return another error use this convenience
 *	macro to propagate the failure. Set a target variable name to store a returned value upon success. Leave type
 *	argument empty for existing variables
 */
#define PROPAGATE_FAIL_TV(type, var, expression)        \
	type var = (expression);                            \
	if (UNLIKELY(var.HasError())) return var.GetError() \
		->WithLocation()                               //

/**
 *	If a function returns a TMaybe<V> inside another function which may also return another error use this convenience
 *	macro to propagate the failure. Set a local variable to store a returned value upon success.
 */
#define PROPAGATE_FAIL_V(var, expression) PROPAGATE_FAIL_TV(auto, var, expression)

/**
 *	If a function returns an FCanFail inside another function which may also return another error use this convenience
 *	macro to propagate the failure. This is only useful with expressions which doesn't return a value upon success.
 */
#define PROPAGATE_FAIL(expression) PROPAGATE_FAIL_V(PREPROCESSOR_JOIN(tempResult, __LINE__), expression)
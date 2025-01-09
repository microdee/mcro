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

namespace Mcro::Error
{
	using namespace Mcro::Concepts;
	
	/**
	 *	TValueOrError cannot have a default state, and cannot be copied, this is done to be able to handle payload types
	 *	which doesn't have either or any of them. Obviously this wrapper will not work with such types and will cause
	 *	substitution error if they're used. However some scenarios like futures need to provide a default state,
	 *	or need to make copies of this wrapper. If both the value and the error types are copyable this should be
	 *	fine to move them around with the potential error info attached.
	 *	ErrorType must be default constructable as that's used as the error value in the default state of this wrapper.
	 *	Also this wrapper mirrors the API of TValueOrError
	 */
	template<CCopyable ValueType, CCopyable ErrorType>
	struct TValueOrErrorCopyable
	{
	private:
		static TValueOrError<ValueType, ErrorType> CopyValueOrError(const TValueOrError<ValueType, ErrorType>& from)
		{
			if (from.HasValue())
			{
				return MakeValue(from.GetValue());
			}
			return MakeError(from.GetError());
		}
		TOptional<TValueOrError<ValueType, ErrorType>> Storage {};
		
	public:
		TValueOrErrorCopyable() {}
		
		template <typename... ArgTypes>
		TValueOrErrorCopyable(TValueOrError_ValueProxy<ArgTypes...>&& proxy)
		{
			TValueOrError<ValueType, ErrorType> payload(MoveTemp(proxy));
			Storage = MoveTemp(payload);
		}
		
		template <typename... ArgTypes>
		TValueOrErrorCopyable(TValueOrError_ErrorProxy<ArgTypes...>&& proxy)
		{
			TValueOrError<ValueType, ErrorType> payload(MoveTemp(proxy));
			Storage = MoveTemp(payload);
		}

		TValueOrErrorCopyable(TValueOrError<ValueType, ErrorType>&& valueOrError)
		{
			Storage = MoveTemp(valueOrError);
		}

		TValueOrErrorCopyable(const TValueOrError<ValueType, ErrorType>& valueOrError)
		{
			Storage = CopyValueOrError(valueOrError);
		}

		TValueOrErrorCopyable(TValueOrErrorCopyable&& from) noexcept
		{
			if (from.Storage.IsSet())
			{
				Storage = MoveTemp(from.Storage);
				from.Storage = {};
			}
		}

		TValueOrErrorCopyable(const TValueOrErrorCopyable& from)
		{
			if (from.Storage.IsSet())
			{
				Storage = CopyValueOrError(from.Storage.GetValue());
			}
		}

		operator TValueOrError<ValueType, ErrorType>() const
		{
			static ErrorType defaultError {};
			if (Storage.IsSet())
			{
				return CopyValueOrError(Storage.GetValue());
			}
			return MakeError(defaultError);
		}

		TValueOrErrorCopyable& operator =(const TValueOrErrorCopyable &) = default;

		bool HasError() const
		{
			return Storage.IsSet() ? Storage.GetValue().HasError() : true;
		}

		bool HasValue() const
		{
			return Storage.IsSet() ? Storage.GetValue().HasValue() : false;
		}

		bool IsValid() const
		{
			return Storage.IsSet() && Storage.GetValue().IsValid();
		}

		ErrorType& GetError() &
		{
			static ErrorType defaultError {};
			if (Storage.IsSet())
			{
				return Storage.GetValue().GetError();
			}
			return defaultError;
		}

		const ErrorType& GetError() const &
		{
			static ErrorType defaultError {};
			if (Storage.IsSet())
			{
				return Storage.GetValue().GetError();
			}
			return defaultError;
		}

		ErrorType* TryGetError()
		{
			static ErrorType defaultError {};
			if (Storage.IsSet())
			{
				return Storage.GetValue().TryGetError();
			}
			// We're not returning nullptr here as the default state of this wrapper is having an error 
			return &defaultError;
		}

		const ValueType* TryGetError() const
		{
			static ErrorType defaultError {};
			if (Storage.IsSet())
			{
				return Storage.GetValue().TryGetError();
			}
			// We're not returning nullptr here as the default state of this wrapper is having an error
			return &defaultError;
		}

		ValueType& GetValue() &
		{
			check(Storage.IsSet())
			return Storage.GetValue().GetValue();
		}

		const ValueType& GetValue() const &
		{
			check(Storage.IsSet())
			return Storage.GetValue().GetValue();
		}

		ValueType* TryGetValue()
		{
			if (Storage.IsSet())
			{
				return Storage.GetValue().TryGetValue();
			} 
			return nullptr;
		}

		const ValueType* TryGetValue() const
		{
			if (Storage.IsSet())
			{
				return Storage.GetValue().TryGetValue();
			}
			return nullptr;
		}
	};
}

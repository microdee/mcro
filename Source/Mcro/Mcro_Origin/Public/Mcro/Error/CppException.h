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

#include <exception>

namespace Mcro::Error
{
	/**
	 *	Unreal itself discourages the usage of C++ exceptions and there are also couple of thoughts above IError why
	 *	that's the case. However there are third-party libraries which may require their users to catch errors their functions
	 *	may yield via exceptions. For this reason there's an IError wrapper around std::exception base class which however
	 *	will not preserve the actual type of the cought exception. For that use TCppException template.
	 */
	class MCRO_API FCppException : public IError
	{
	public:
		FCppException(std::exception const& input);
		virtual ~FCppException() = default;
		
		virtual TSharedRef<SErrorDisplay> CreateErrorWidget() override;

		/** Storage for the exception but as the base STL type. It's copied to this member */
		std::exception BaseException;

	protected:
		virtual FStringView GetExceptionType() const;
		virtual void SerializeMembers(YAML::Emitter& emitter, bool isRoot) const override;
	};

	/**
	 *	Use this template for catching STL exceptions to preserve the exception type name. 
	 *	@tparam Exception  Type of the encapsulated exception
	 */
	template <CDerivedFrom<std::exception> Exception>
	class TCppException : public FCppException
	{
	public:
		TCppException(Exception const& input)
			: FCppException(input)
			, TypedException(input)
		{}
		
		/** Storage for the exception but preserving its actual type. It's copied to this member */
		Exception TypedException;
		
	protected:
		virtual FStringView GetExceptionType() const override
		{
			return TTypeName<Exception>;
		}
	};
}

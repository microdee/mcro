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
#include "Mcro/Text.h"

#include "Mcro/LibraryIncludes/Start.h"
#include "PreMagicEnum.h"
#include "magic_enum.hpp"
#include "yaml-cpp/yaml.h"
#include "Mcro/LibraryIncludes/End.h"

namespace Mcro::Yaml
{
	using namespace Mcro::Text;

	/**
	 *	@brief  RAII friendly region annotation for YAML::Emitter streams
	 *	@tparam Begin  The YAML region begin tag
	 *	@tparam   End  The YAML region end tag
	 */
	template <YAML::EMITTER_MANIP Begin, YAML::EMITTER_MANIP End>
	class TScopedRegion
	{
		YAML::Emitter& Out;

	public:
		TScopedRegion(YAML::Emitter& out) : Out(out) { Out << Begin; }
		~TScopedRegion() { Out << End; }

		template <typename T>
		TScopedRegion& operator << (T&& rhs)
		{
			Out << Forward<T>(rhs);
			return *this;
		}
	};

	/** @brief Annotate a mapping region in a YAML::Emitter stream, which ends when this object goes out of scope */
	using FMap = TScopedRegion<YAML::BeginMap, YAML::EndMap>;
	
	/** @brief Annotate a sequence region in a YAML::Emitter stream, which ends when this object goes out of scope */
	using FSeq = TScopedRegion<YAML::BeginSeq, YAML::EndSeq>;

	/** @brief Convenience operator to append Unreal or potentially wide strings to YAML::Emitter streams */
	template <typename String>
	requires (CStringOrViewOrName<String> || CStdStringOrView<String>)
	YAML::Emitter& operator << (YAML::Emitter& out, String&& v)
	{
		out << StdConvert<ANSICHAR>(Forward<String>(v));
		return out;
	}
	
	/** @brief Convenience operator to append enums as strings to a YAML::Emitter streams */
	template <CEnum Enum>
	YAML::Emitter& operator << (YAML::Emitter& out, Enum v)
	{
		out << StdConvert<ANSICHAR>(magic_enum::enum_name(v));
		return out;
	}
}
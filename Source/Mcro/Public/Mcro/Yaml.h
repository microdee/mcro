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
#include "Mcro/Tuples.h"
#include "Mcro/Templates.h"

#include "Mcro/LibraryIncludes/Start.h"
#include "PreMagicEnum.h"
#include "magic_enum.hpp"
#include "yaml-cpp/yaml.h"
#include "Mcro/LibraryIncludes/End.h"

namespace Mcro::Yaml
{
	using namespace Mcro::Text;
	using namespace Mcro::Tuples;
	using namespace Mcro::Templates;

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

	namespace Detail
	{
		template <typename T>
		struct TYamlAppend
		{
			static constexpr bool bHandled = false;
		};

		template <typename T>
		concept CYamlAppendHandled = TYamlAppend<T>::bHandled;

		template <CYamlAppendHandled T>
		YAML::Emitter& YamlAppend(YAML::Emitter& out, T const& v)
		{
			return TYamlAppend<T>::Append(out, v);
		}
		
		template <typename T>
		requires (!CYamlAppendHandled<T>)
		YAML::Emitter& YamlAppend(YAML::Emitter& out, T const& v)
		{
			return out << v;
		}
		
		template <CStringFormatArgument T>
		requires (!CStdStringOrViewTyped<T, ANSICHAR>)
		struct TYamlAppend<T>
		{
			static constexpr bool bHandled = true;
			static YAML::Emitter& Append(YAML::Emitter& out, T const& v)
			{
				out << StdConvert<ANSICHAR>(AsString(v));
				return out;
			}
		};
		
		template <CTuple T, size_t... Indices>
		YAML::Emitter& YamlAppendTuple(YAML::Emitter& out, T const& v, std::index_sequence<Indices...>&&)
		{
			out << YAML::Flow;
			FSeq seq(out);
			(YamlAppend(out, GetItem<Indices>(v)), ...);
			return out;
		}

		template <CTuple T>
		struct TYamlAppend<T>
		{
			static constexpr bool bHandled = true;
			static YAML::Emitter& Append(YAML::Emitter& out, T const& v)
			{
				return YamlAppendTuple(out, v, std::make_index_sequence<GetSize<T>()>());
			}
		};

		template <CRangeMember T>
		requires (
			!CIsTemplate<T, TMap>
			&& !CIsTemplate<T, std::map>
			&& !CIsTemplate<T, std::unordered_map>
			&& !CIsTemplate<T, std::vector>
			&& !CIsTemplate<T, std::list>
			&& !CStringOrViewOrName<T>
			&& !CStdStringOrView<T>
		)
		struct TYamlAppend<T>
		{
			static constexpr bool bHandled = true;
			static YAML::Emitter& Append(YAML::Emitter& out, T const& v)
			{
				FSeq seq(out);
				for (auto const& item : v)
					YamlAppend(out, item);
				return out;
			}
		};

		template <typename Key, typename Value, typename... Rest>
		struct TYamlAppend<TMap<Key, Value, Rest...>>
		{
			static constexpr bool bHandled = true;
			static YAML::Emitter& Append(YAML::Emitter& out, TMap<Key, Value, Rest...> const& v)
			{
				FMap map(out);
				for (TPair<Key, Value> const& item : v)
				{
					out << YAML::Key << YAML::Flow;
					YamlAppend(out, item.Key);
					out << YAML::Value;
					YamlAppend(out, item.Value);
				}
				return out;
			}
		};
	}

	/**
	 *	@brief
	 *	A generic append operator for YAML::Emitter, which attempts to handle containers, tuples and can accept types
	 *	which are also compatible as a string format argument. In other words this templated append operator overload
	 *	is a fallback when an explicit one for the given type is not available.
	 */
	template <Detail::CYamlAppendHandled T>
	YAML::Emitter& operator << (YAML::Emitter& out, T const& v)
	{
		return Detail::YamlAppend(out, v);
	}
}

#define MCRO_GENERATE_YAML_MAP_MEMBER_FIELD(r, data, field) \
	BOOST_PP_SEQ_ELEM(0, data) << YAML::Key << BOOST_PP_STRINGIZE(field) << YAML::Value << BOOST_PP_SEQ_ELEM(1, data) field;

#define MCRO_GENERATE_YAML_MAP_THIS_FIELD(r, data, field) \
	data << YAML::Key << BOOST_PP_STRINGIZE(field) << YAML::Value << field;

#define GENERATE_YAML_MAP(emitter, object, ...) \
	BOOST_PP_SEQ_FOR_EACH(MCRO_GENERATE_YAML_MAP_MEMBER_FIELD, (emitter)(object), BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
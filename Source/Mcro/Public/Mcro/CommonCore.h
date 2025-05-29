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

/**
 *	@file
 *	@brief
 *	This header is a central include for simpler utilities MCRO offers. Higher level features like errors and delegates
 *	are culminated in `Mcro/Common.h`. You may use the Mcro::Common namespace or use the individual namespaces declared
 *	in the individual headers if you notice name conflicts with your own project.
 */

#pragma once

#include "CoreMinimal.h"

#include "Mcro/ArrayViews.h"
#include "Mcro/Ansi/Allocator.h"
#include "Mcro/Ansi/New.h"
#include "Mcro/Badge.h"
#include "Mcro/Composition.h"
#include "Mcro/Concepts.h"
#include "Mcro/Construct.h"
#include "Mcro/Enums.h"
#include "Mcro/Finally.h"
#include "Mcro/FmtMacros.h"
#include "Mcro/FunctionTraits.h"
#include "Mcro/InitializeOnCopy.h"
#include "Mcro/Once.h"
#include "Mcro/SharedObjects.h"
#include "Mcro/Text.h"
#include "Mcro/Text/TupleAsString.h"
#include "Mcro/Threading.h"
#include "Mcro/Types.h"
#include "Mcro/Void.h"
#include "Mcro/TextMacros.h"
#include "Mcro/Range.h"
#include "Mcro/Range/Conversion.h"
#include "Mcro/Range/Views.h"
#include "Mcro/ValueThunk.h"
#include "Mcro/Zero.h"

/** @brief Use this namespace for the minimal utilities MCRO has to offer */
namespace Mcro::CommonCore
{
	namespace Ansi = Mcro::Ansi;
	using namespace Mcro::ArrayViews;
	using namespace Mcro::Badge;
	using namespace Mcro::Composition;
	using namespace Mcro::Concepts;
	using namespace Mcro::Construct;
	using namespace Mcro::Enums;
	using namespace Mcro::Finally;
	using namespace Mcro::FunctionTraits;
	using namespace Mcro::InitializeOnCopy;
	using namespace Mcro::Once;
	using namespace Mcro::SharedObjects;
	using namespace Mcro::Text;
	using namespace Mcro::Threading;
	using namespace Mcro::Types;
	using namespace Mcro::Range;
	using namespace Mcro::ValueThunk;
}

/** @brief Use this namespace for all the common features MCRO has to offer */
namespace Mcro::Common
{
	using namespace Mcro::CommonCore;
}
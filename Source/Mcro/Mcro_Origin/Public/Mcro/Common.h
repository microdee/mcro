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
 *	This header is a central include for all common utilities MCRO offers including higher level features like errors
 *	and delegates. You may use the Mcro::Common namespace or use the individual namespaces declared
 *	in the individual headers if you notice name conflicts with your own project.
 */

#pragma once

#include "Mcro/CommonCore.h"
#include "Mcro/AutoModularFeature.h"
#include "Mcro/AssertMacros.h"
#include "Mcro/Dll.h"
#include "Mcro/Delegates/EventDelegate.h"
#include "Mcro/Delegates/DelegateFrom.h"
#include "Mcro/Delegates/AsNative.h"
#include "Mcro/Delegates/EventMultiplex.h"
#include "Mcro/Error/BlueprintStackTrace.h"
#include "Mcro/Error/CppException.h"
#include "Mcro/Error/CppStackTrace.h"
#include "Mcro/Error/ErrorManager.h"
#include "Mcro/Error/PlainTextComponent.h"
#include "Mcro/Error/SErrorDisplay.h"
#include "Mcro/Error/SPlainTextDisplay.h"
#include "Mcro/Modules.h"
#include "Mcro/Observable.h"
#include "Mcro/Rendering/Textures.h"
#include "Mcro/Slate.h"
#include "Mcro/Subsystems.h"
#include "Mcro/TimespanLiterals.h"
#include "Mcro/UObjects/Init.h"
#include "Mcro/UObjects/ScopeObject.h"
#include "Mcro/Yaml.h"

/** @brief Use this namespace for all the common features MCRO has to offer */
namespace Mcro::Common
{
	using namespace Mcro::AutoModularFeature;
	using namespace Mcro::Dll;
	using namespace Mcro::Delegates;
	using namespace Mcro::Error;
	using namespace Mcro::Modules;
	using namespace Mcro::Observable;
	using namespace Mcro::Rendering::Textures;
	using namespace Mcro::Slate;
	using namespace Mcro::Subsystems;
	using namespace Mcro::Timespan;
	using namespace Mcro::UObjects::Init;
	using namespace Mcro::Yaml;

	/** @brief Use Mcro::Common with namespaces included which may guard common vocabulary symbols like "From" or "Get" */
	namespace With
	{
		/** @copydoc Mcro::Delegates::InferDelegate */
		namespace InferDelegate   { using namespace Mcro::Common; using namespace Mcro::Common::InferDelegate; }
		
		/** @copydoc Mcro::Slate::AttributeAppend */
		namespace AttributeAppend { using namespace Mcro::Common; using namespace Mcro::Slate::AttributeAppend; }
		
		/** @copydoc Mcro::Timespan::Literals */
		namespace Literals        { using namespace Mcro::Common; using namespace Mcro::Timespan::Literals; }
	}
}
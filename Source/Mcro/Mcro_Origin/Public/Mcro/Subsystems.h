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
#include "Mcro/AssertMacros.h"
#include "Kismet/GameplayStatics.h"

namespace Mcro::Subsystems
{
	using namespace Mcro::Concepts;
	using namespace Mcro::TypeName;

	template<typename T>
	concept CSubsystem = CDerivedFrom<T, USubsystem>;

	template<typename T>
	concept CEngineSubsystem = CDerivedFrom<T, UEngineSubsystem>;

	template<typename T>
	concept CGameInstanceSubsystem = CDerivedFrom<T, UGameInstanceSubsystem>;

	template<typename T>
	concept CLocalPlayerSubsystem = CDerivedFrom<T, ULocalPlayerSubsystem>;

	template<typename T>
	concept CWorldSubsystem = CDerivedFrom<T, UWorldSubsystem>;

	enum class EGameInstanceFallback
	{
		UseGameViewport,
		UseFirstWorldContext,
	};

	/** @brief Extra namespace encapsulates common vocabulary */
	namespace Subsystems
	{
		/**
		 *	@brief
		 *	Helper for getting a subsystem, the internal boilerplate will be chosen based on the type of subsystem T
		 *	inherits from. Required arguments may vary depending on the type of subsystem.
		 *	
		 *	@tparam  T  UEngineSubsystem derivative
		 *	@return  Subsystem if exists or nullptr
		 */
		template<CEngineSubsystem T>
		T* Get()
		{
			return GEngine ? GEngine->GetEngineSubsystem<T>() : nullptr;
		}

		/**
		 *	@brief
		 *	Helper for getting a subsystem, the internal boilerplate will be chosen based on the type of subsystem T
		 *	inherits from. Required arguments may vary depending on the type of subsystem.
		 *	
		 *	@tparam  T  UGameInstanceSubsystem derivative
		 *	@return  Subsystem if exists or nullptr
		 */
		template<CGameInstanceSubsystem T>
		T* Get(
			const UObject* worldContextObject = nullptr,
			EGameInstanceFallback fallback = EGameInstanceFallback::UseGameViewport,
			EGetWorldErrorMode errorMode = EGetWorldErrorMode::LogAndReturnNull
		)
		{
			if (!GEngine)
			{
				return nullptr;
			}
			if (worldContextObject == nullptr)
			{
				if (IsRunningDedicatedServer() || GEngine->GameViewport == nullptr)
				{
					fallback = EGameInstanceFallback::UseFirstWorldContext;
				}
				switch (fallback)
				{
				default:
				case EGameInstanceFallback::UseGameViewport:
					{
						const FWorldContext* worldContext = GEngine->GameViewport
							? GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport)
							: nullptr;
						const UGameInstance* owningGameInstance = worldContext ? worldContext->OwningGameInstance : nullptr;
						return owningGameInstance ? owningGameInstance->GetSubsystem<T>() : nullptr;
					}
				case EGameInstanceFallback::UseFirstWorldContext:
					{
						for (auto it = GEngine->GetWorldContexts().CreateConstIterator(); it; ++it)
						{
							if (T* result = it->OwningGameInstance ? it->OwningGameInstance->GetSubsystem<T>() : nullptr)
							{
								return result;
							}
						}
						return nullptr;
					}
				}
			}
			if (const UWorld* world = GEngine->GetWorldFromContextObject(worldContextObject, errorMode))
			{
				return UGameInstance::GetSubsystem<T>(world->GetGameInstance());
			}
			return nullptr;
		}

		/**
		 *	@brief
		 *	Helper for getting a subsystem, the internal boilerplate will be chosen based on the type of subsystem T
		 *	inherits from. Required arguments may vary depending on the type of subsystem.
		 *	
		 *	@tparam  T  ULocalPlayerSubsystem derivative
		 *	@return  Subsystem if exists or nullptr
		 */
		template<CLocalPlayerSubsystem T>
		T* Get(const UObject* worldContext)
		{
			const UWorld* world = GEngine->GetWorldFromContextObject(worldContext, EGetWorldErrorMode::LogAndReturnNull);
			APlayerController* controller = world && !world->IsNetMode(NM_DedicatedServer)
				? UGameplayStatics::GetPlayerController(worldContext, 0)
				: nullptr;
			return controller ? ULocalPlayer::GetSubsystem<T>(controller->GetLocalPlayer()) : nullptr;
		}

		/**
		 *	@brief
		 *	Helper for getting a subsystem, the internal boilerplate will be chosen based on the type of subsystem T
		 *	inherits from. Required arguments may vary depending on the type of subsystem.
		 *	
		 *	@tparam  T  UWorldSubsystem derivative
		 *	@return  Subsystem if exists or nullptr
		 */
		template<CWorldSubsystem T>
		T* Get(const UObject* worldContext, EGetWorldErrorMode errorMode = EGetWorldErrorMode::LogAndReturnNull)
		{
			if (const UWorld* world = GEngine ? GEngine->GetWorldFromContextObject(worldContext, errorMode) : nullptr)
			{
				return world->GetSubsystem<T>();
			}
			return nullptr;
		}

		/**
		 *	@brief
		 *	Helper for getting a subsystem, the internal boilerplate will be chosen based on the type of subsystem T
		 *	inherits from. Required arguments may vary depending on the type of subsystem. This is a checked version so
		 *	if any steps fail to produce the target subsystem the program may crash.
		 *	
		 *	@tparam  T  type of the subsystem
		 *	@return  Guaranteed valid reference to a subsystem
		 */
		template<CSubsystem T, typename... Args>
		T& GetChecked(Args... args)
		{
			T* result = Get<T>(args...);
			
			ASSERT_CRASH(result,
				->WithMessageF(TEXT_"Couldn't find required subsystem {0}", TTypeName<T>)
			)
			
			return *result;
		}

		/**
		 *	@brief
		 *	Helper for getting a subsystem, the internal boilerplate will be chosen based on the type of subsystem T
		 *	inherits from. Required arguments may vary depending on the type of subsystem. This is an ensured version so
		 *	if any steps fail to produce the target subsystem an ensure may be hit.
		 *	
		 *	@tparam  T  type of the subsystem
		 *	@return  Subsystem if exists or nullptr
		 */
		template<CSubsystem T, typename... Args>
		T* GetEnsured(Args... args)
		{
			T* result = Get<T>(args...);
			ensure(result);
			return result;
		}

		/**
		 *	@brief
		 *	Helper for checking if some other subsystem should be created. Useful when you want to make a subsystem
		 *	which should only be created if some other subsystem should be also created.
		 *	
		 *	@tparam T  type of the subsystem
		 *	
		 *	@param outer
		 *	be sure to pass outer argument which type aligns with checked subsystem outer. This may fail for example
		 *	if you pass game instance as outer to check for world subsystem existence as ShouldCreateSubsystem of that
		 *	subsystem may expect the outer to be World.
		 *	
		 *	@return  True if the target subsystem should be created
		 */
		template<CSubsystem T>
		bool ShouldCreate(UObject* outer)
		{
			const USubsystem* cdo = T::StaticClass()->template GetDefaultObject<USubsystem>();
			return cdo->ShouldCreateSubsystem(outer);
		}
	}
}

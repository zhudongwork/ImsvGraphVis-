// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "BodyStatePrivatePCH.h"
#include "FBodyState.h"
#include "BodyStateBPLibrary.h"

UBodyStateBPLibrary::UBodyStateBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

int32 UBodyStateBPLibrary::DeviceAttached(const FBodyStateDeviceConfig& Configuration, TScriptInterface<IBodyStateInputInterface> InputCallbackDelegate)
{
	UE_LOG(BodyStateLog, Warning, TEXT("Not implemented yet, use raw for now."));
	return -1;
}


int32 UBodyStateBPLibrary::DeviceAttachedRaw(const FBodyStateDeviceConfig& Configuration, IBodyStateInputRawInterface* InputCallbackDelegate)
{
	return IBodyState::Get().DeviceAttached(Configuration, InputCallbackDelegate);
}

bool UBodyStateBPLibrary::DeviceDetached(int32 DeviceID)
{
	if (IBodyState::IsAvailable())
	{
		return IBodyState::Get().DeviceDetached(DeviceID);
	}
	else
	{
		return false;
	}
}

UBodyStateSkeleton* UBodyStateBPLibrary::SkeletonForDevice(UObject* WorldContextObject, int32 DeviceID /*= 0*/)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
	{
		UE_LOG(BodyStateLog, Warning, TEXT("SkeletonForDevice:: Wrong world context"))
		return nullptr;
	}

	if (IBodyState::IsAvailable() && 
		(World->IsGameWorld() || World->IsPreviewWorld()))
	{
		return IBodyState::Get().SkeletonForDevice(DeviceID);
	}
	else
	{
		return nullptr;
	}
}

bool UBodyStateBPLibrary::AttachMergeAlgorithm(TFunction< void()> InFunction)
{
	UE_LOG(BodyStateLog, Warning, TEXT("Not implemented yet."));
	return false;
}


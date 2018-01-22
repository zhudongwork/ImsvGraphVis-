#include "BodyStatePrivatePCH.h"
#include "BodyStateSkeletonStorage.h"

FBodyStateSkeletonStorage::FBodyStateSkeletonStorage()
{
	PrivateMergedSkeleton = nullptr;
	DeviceIndex = 0;
}

FBodyStateSkeletonStorage::~FBodyStateSkeletonStorage()
{
	//Allow merge skeleton to be removed
	if (PrivateMergedSkeleton && PrivateMergedSkeleton->IsValidLowLevel())
	{
		PrivateMergedSkeleton->RemoveFromRoot();
		PrivateMergedSkeleton = nullptr;
	}
}

int32 FBodyStateSkeletonStorage::AddDevice(const FBodyStateDevice& InDevice)
{
	FBodyStateDevice Device = InDevice;
	DeviceIndex++;
	Device.DeviceId = DeviceIndex;

	Device.Skeleton = NewObject<UBodyStateSkeleton>();
	Device.Skeleton->AddToRoot();

	Devices.Add(Device.InputCallbackDelegate, Device);
	DeviceKeyMap.Add(Device.DeviceId, Device.InputCallbackDelegate);

	UE_LOG(BodyStateLog, Log, TEXT("BodyState::DeviceAttached: %s (%d)"), *Device.Config.DeviceName, Devices.Num());
	return Device.DeviceId;
}

bool FBodyStateSkeletonStorage::RemoveDevice(int32 DeviceId)
{
	UE_LOG(BodyStateLog, Log, TEXT("Id: %d"), DeviceId);
	if (DeviceId < 0)
	{
		UE_LOG(BodyStateLog, Log, TEXT("BodyState::RemoveDevice attempted to remove invalid index: (%d)"), DeviceId);
		return false;	//couldn't find
	}

	bool HasKey = DeviceKeyMap.Contains(DeviceId);
	
	if (!HasKey)
	{
		UE_LOG(BodyStateLog, Log, TEXT("BodyState::RemoveDevice already removed (%d)"), DeviceId);
		return false;	//already removed
	}

	IBodyStateInputRawInterface* DelegatePtr = DeviceKeyMap[DeviceId];
	const FBodyStateDevice& Device = Devices[DelegatePtr];
	const FString DeviceName = Device.Config.DeviceName;

	if (Device.Skeleton->IsValidLowLevel())
	{
		Device.Skeleton->RemoveFromRoot();
	}
	DeviceKeyMap.Remove(DeviceId);
	Devices.Remove(DelegatePtr);

	UE_LOG(BodyStateLog, Log, TEXT("BodyState::Device Detached: %s (%d)"), *DeviceName, Devices.Num());

	return true;
}

void FBodyStateSkeletonStorage::RemoveAllDevices()
{
	CallFunctionOnDevices([this](const FBodyStateDevice& Device)
	{
		RemoveDevice(Device.DeviceId);
	});
}

UBodyStateSkeleton* FBodyStateSkeletonStorage::SkeletonForDevice(int32 DeviceId)
{
	//Return merged/only skeleton
	if (DeviceId == 0)
	{
		return PrivateMergedSkeleton;

		//Optimization: If there is exactly 1 skeleton, return that one
		/*if (Devices.Num() == 1)
		{
			for (auto& Elem : Devices)
			{
				return Elem.Value.Skeleton;
			}
			return nullptr;	//should never happen
		}
		else
		{
			//Return the merge skeleton, will be null if devices = 0
			return PrivateMergedSkeleton;
		}*/
	}
	//Return specific skeleton
	else if (!DeviceKeyMap.Contains(DeviceId))
	{
		UE_LOG(BodyStateLog, Warning, TEXT("DeviceID: %d is invalid, returning nullptr skeleton."), DeviceId);
		return nullptr;
	}
	else
	{
		return Devices[DeviceKeyMap[DeviceId]].Skeleton;
	}
}

UBodyStateSkeleton* FBodyStateSkeletonStorage::MergedSkeleton()
{
	//create as needed
	if (!PrivateMergedSkeleton || !PrivateMergedSkeleton->IsValidLowLevel())
	{
		PrivateMergedSkeleton = NewObject<UBodyStateSkeleton>();
		PrivateMergedSkeleton->AddToRoot();
	}
	return PrivateMergedSkeleton;
}

void FBodyStateSkeletonStorage::UpdateMergeSkeletonData()
{
	//Basic merge of skeleton data
	MergedSkeleton();

	//For now just merge everything to latest
	//Todo: update merge algorithm to merge only tracked bones at highest confidence
	for (auto& Elem : Devices)
	{
		UBodyStateSkeleton* Skeleton = Elem.Value.Skeleton;
		PrivateMergedSkeleton->SetFromOtherSkeleton(Skeleton);
	}
}

void FBodyStateSkeletonStorage::CallFunctionOnDevices(TFunction< void(const FBodyStateDevice&)> InFunction)
{
	for (auto& Elem : Devices)
	{
		auto& Device = Elem.Value;
		if (Device.InputCallbackDelegate != nullptr)
		{
			InFunction(Device);
		}
	}
}
#pragma once

#include "BodyStateDevice.h"

/**
* Holds all device/skeleton storage data
*/
class FBodyStateSkeletonStorage
{
public:
	FBodyStateSkeletonStorage();
	~FBodyStateSkeletonStorage();

	/** 
	* Add device, return unique id
	*/
	int32 AddDevice(const FBodyStateDevice& Device);
	
	/**
	* Remove device by id. Return if operation removed the device, false if already removed
	*/
	bool RemoveDevice(int32 DeviceId);

	/** 
	* Usually used when cleaning up
	*/
	void RemoveAllDevices();

	/** 
	* Get the skeleton associated with the device
	*/
	UBodyStateSkeleton* SkeletonForDevice(int32 DeviceId);	

	/** 
	* Return the merged skeleton
	*/
	UBodyStateSkeleton* MergedSkeleton();

	void UpdateMergeSkeletonData();

	/**
	* Return a list of tracked Devices that are currently tracked/active
	* Todo: make that true
	*/
	//TArray<FBodyStateDevice&> TrackedDevices();

	/**
	* Call this function on all the attached devices. 
	*/
	void CallFunctionOnDevices(TFunction< void(const FBodyStateDevice&)> InFunction);

private:
	UBodyStateSkeleton* PrivateMergedSkeleton;

	TMap<IBodyStateInputRawInterface*, FBodyStateDevice> Devices;
	TMap<int32, IBodyStateInputRawInterface*> DeviceKeyMap;
	int32 DeviceIndex;
};
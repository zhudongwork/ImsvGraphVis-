#pragma once

#include "BodyStatePrivatePCH.h"
#include "IBodyState.h"
class FBodyStateSkeletonStorage;

class FBodyState : public IBodyState
{
public:
	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler);

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	bool IsActive();

	virtual bool IsInputReady() override;

	TSharedPtr< class FBodyStateInputDevice > BodyStateInputDevice;
	virtual TSharedPtr< class FBodyStateInputDevice > GetInputDevice() { return BodyStateInputDevice; }

	//Attach/Detach devices
	virtual int32 DeviceAttached(const FBodyStateDeviceConfig& Configuration, IBodyStateInputRawInterface* InputCallbackDelegate) override;
	virtual bool DeviceDetached(int32 DeviceID) override;

	virtual UBodyStateSkeleton* SkeletonForDevice(int32 DeviceID) override;

private:
	bool bActive = false;
	TSharedPtr<FBodyStateSkeletonStorage> SkeletonStorage;
};
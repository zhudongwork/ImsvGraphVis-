// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"
#include "BodyStateDeviceConfig.h"
#include "BodyStateInputInterface.h"
#include "IInputDeviceModule.h"

class IBodyStateInputRawInterface;

class BODYSTATE_API  IBodyState : public IInputDeviceModule
{
public:
	/**
	* Singleton-like access to this module's interface.  This is just for convenience!
	* Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	*
	* @return Returns singleton instance, loading the module on demand if needed
	*/
	static inline IBodyState& Get()
	{
		return FModuleManager::LoadModuleChecked< IBodyState >("BodyState");
	}

	/**
	* Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	*
	* @return True if the module is loaded and ready to use
	*/
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("BodyState");
	}

	/** Public Module methods*/
	virtual bool IsInputReady() { return false; }

	//Add/remove device
	virtual int32 DeviceAttached(const FBodyStateDeviceConfig& Configuration, IBodyStateInputRawInterface* InputCallbackDelegate) { return -1;}
	virtual bool DeviceDetached(int32 DeviceID) { return false; }

	//Fetch
	virtual class UBodyStateSkeleton* SkeletonForDevice(int32 DeviceID) { return nullptr; }
};
#include "BodyStatePrivatePCH.h"
#include "IHeadMountedDisplay.h"
#include "SlateBasics.h"
#include "BodyStateInputInterface.h"
#include "FBodyStateInputDevice.h"
#include "BodyStateHMDSnapshot.h"
#include "BodyStateDevice.h"

//UE v4.6 IM event wrappers
bool FBodyStateInputDevice::EmitKeyUpEventForKey(FKey Key, int32 User, bool Repeat)
{
	FKeyEvent KeyEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0);
	return FSlateApplication::Get().ProcessKeyUpEvent(KeyEvent);
}

bool FBodyStateInputDevice::EmitKeyDownEventForKey(FKey Key, int32 User, bool Repeat)
{
	FKeyEvent KeyEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0);
	return FSlateApplication::Get().ProcessKeyDownEvent(KeyEvent);
}

bool FBodyStateInputDevice::EmitAnalogInputEventForKey(FKey Key, float Value, int32 User, bool Repeat)
{
	FAnalogInputEvent AnalogInputEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0, Value);
	return FSlateApplication::Get().ProcessAnalogInputEvent(AnalogInputEvent);
}

FBodyStateInputDevice::FBodyStateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) : MessageHandler(InMessageHandler) 
{
	//HMDSamples = new HMDSnapshotSamples(ControllerData.LeapController); 	//store a reference for our timing circuits
	DeviceIndex = 0;	//Always restart the index at 0
}

FBodyStateInputDevice::~FBodyStateInputDevice()
{
}

void FBodyStateInputDevice::SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}

bool FBodyStateInputDevice::AttachMergeAlgorithm(TFunction< void()> InFunction)
{
	return false;
}



bool FBodyStateInputDevice::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	// Nothing necessary to do (boilerplate code to complete the interface)
	return false;
}


void FBodyStateInputDevice::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value)
{
	// Nothing necessary to do (boilerplate code to complete the interface)
}


void FBodyStateInputDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values)
{
	// Nothing necessary to do (boilerplate code to complete the interface)
}

/************************************************************************/
/* Key Tick flow functions                                              */
/************************************************************************/
void FBodyStateInputDevice::Tick(float DeltaTime)
{
	//Tick - store delta time for BS refence

}

//Main loop event emitter
void FBodyStateInputDevice::SendControllerEvents()
{
	//HMDSamples->AddCurrentHMDSample();
	
	DispatchInput();
	DispatchEstimators();
	
	//DispatchRecognizers();
}

//End key tick flow functions

/************************************************************************/
/* Key Input functions                                                  */
/************************************************************************/
void FBodyStateInputDevice::DispatchInput()
{
	//TODO expand this

	//Fetch input from all attached devices
	SkeletonStorage->CallFunctionOnDevices([this](const FBodyStateDevice& Device)
	{
		Device.InputCallbackDelegate->UpdateInput(Device.DeviceId, Device.Skeleton);
	});
}

void FBodyStateInputDevice::DispatchEstimators()
{
	//Copy results to merged skeleton
	SkeletonStorage->UpdateMergeSkeletonData();

	//Dispatch estimator function lambdas which give merge skeleton and expect further updated values
}

void FBodyStateInputDevice::DispatchRecognizers()
{

}

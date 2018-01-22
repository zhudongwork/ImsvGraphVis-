#include "LeapMotionPrivatePCH.h"
#include "FLeapMotionInputDevice.h"
#include "BodyStateBPLibrary.h"
#include "Projects.h"
#include "BodyStateDeviceConfig.h"

#define LOCTEXT_NAMESPACE "LeapPlugin"

void FLeapMotionPlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	// Custom module-specific init can go here.

	// IMPORTANT: This line registers our input device module with the engine.
	//	      If we do not register the input device module with the engine,
	//	      the engine won't know about our existence. Which means 
	//	      CreateInputDevice never gets called, which means the engine
	//	      will never try to poll for events from our custom input device.
	IModularFeatures::Get().RegisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);

	//Get and display our plugin version in the log for debugging
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FString("LeapMotion"));
	const FPluginDescriptor& PluginDescriptor = Plugin->GetDescriptor();
	UE_LOG(LeapMotionLog, Log, TEXT("Leap Plugin started v%s"), *PluginDescriptor.VersionName);
}

void FLeapMotionPlugin::ShutdownModule()
{
	UE_LOG(LeapMotionLog, Log, TEXT("Leap Plugin shutdown."));

	// Unregister our input device module
	IModularFeatures::Get().UnregisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);
}


void FLeapMotionPlugin::AddEventDelegate(const ULeapComponent* EventDelegate)
{
	if (bActive)
	{
		LeapInputDevice->AddEventDelegate(EventDelegate);
	}
	else 
	{
		//Delay load it until it is ready
		DeferredComponentList.Add((ULeapComponent*)EventDelegate);
	}
}


void FLeapMotionPlugin::RemoveEventDelegate(const ULeapComponent* EventDelegate)
{
	if (bActive)
	{
		LeapInputDevice->RemoveEventDelegate(EventDelegate);
	}
}

FLeapStats FLeapMotionPlugin::GetLeapStats()
{
	if (bActive)
	{
		return LeapInputDevice->GetStats();
	}
	else
	{
		return ILeapMotionPlugin::GetLeapStats();
	}
}

void FLeapMotionPlugin::SetOptions(const FLeapOptions& Options)
{
	if (bActive)
	{
		LeapInputDevice->SetOptions(Options);
	}
}

FLeapOptions FLeapMotionPlugin::GetOptions()
{
	if (bActive)
	{
		return LeapInputDevice->GetOptions();
	}
	else
	{
		return ILeapMotionPlugin::GetOptions();
	}
}

void FLeapMotionPlugin::AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible)
{
	LeapInputDevice->AreHandsVisible(LeftHandIsVisible, RightHandIsVisible);
}

void FLeapMotionPlugin::GetLatestFrameData(FLeapFrameData& OutData)
{
	//Copies data
	LeapInputDevice->LatestFrame(OutData);
}

void FLeapMotionPlugin::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable)
{
	LeapInputDevice->SetLeapPolicy(Flag, Enable);
}

bool FLeapMotionPlugin::EnableImageStreaming(bool Enable)
{
	return LeapInputDevice->EnableImageStreaming(Enable);
}

void FLeapMotionPlugin::ShutdownLeap()
{
	UE_LOG(LeapMotionLog, Log, TEXT("Shutting down leap from command."));
	LeapInputDevice->ShutdownLeap();
}

TSharedPtr< class IInputDevice > FLeapMotionPlugin::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	FLeapMotionPlugin::LeapInputDevice = MakeShareable(new FLeapMotionInputDevice(InMessageHandler));

	bActive = true;

	//Add all the deferred components and empty it
	for (int i = 0; i < DeferredComponentList.Num(); i++)
	{
		AddEventDelegate(DeferredComponentList[i]);
	}
	DeferredComponentList.Empty();

	return TSharedPtr< class IInputDevice >(LeapInputDevice);
}

IMPLEMENT_MODULE(FLeapMotionPlugin, LeapMotion)

#undef LOCTEXT_NAMESPACE
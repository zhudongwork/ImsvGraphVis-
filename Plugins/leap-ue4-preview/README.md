leap-ue4-preview
====================

Private Preview build for modern rewrite of leap-ue4 using leapc exclusively

## 

The official [Leap Motion](http://www.leapmotion.com) plugin for the Unreal Engine 4. 

You can use convenience blueprints to just select and play or use a custom approach via blueprints or C++. See relevant sections for details

See [unreal thread](https://forums.unrealengine.com/showthread.php?49107-Plugin-Leap-Motion-Event-Driven) for version downloads and development updates.


# Setup

1. Create new or open a project. 
2. Download [latest release](https://github.com/getnamo/leap-ue4-preview/releases)
3. Drag and drop Plugins into your project root folder
4. The plugin should be enabled and ready to use, if not enable it.
5. (preview only) you may want to disable the engine plugin while testing the preview plugin

#### Quick setup video, get up and running in 41 seconds!
[![Install and Go](https://img.youtube.com/vi/z4HIqr-5kWQ/0.jpg)](https://youtu.be/z4HIqr-5kWQ)

# How to use it - Convenience Rigged Pawn

### VR - Example Pawn
After the plugin is enabled you can change your default pawn to *LeapHandsPawn* or place it in the level with auto-posses set to player 0. If you're using the leap with a Vive, it expects the pawn camera to be at the floor which is it's tracking origin. 

[![VR Quickstart, Click me to see higher resolution](http://i.imgur.com/EATrySQ.gif)](https://youtu.be/qRo-2-Qe4bs)

*NB: click the gif for a higher resolution video*

If you're using Oculus, you may wish to adjust the LeapHandsPawn offset or the MoveUpVector to match your expected origin if different than default.

If you're using the mobile platform exclusively, place the pawn where you head is and disable *Move Up for Mobile VR*.

If you're using multiple platforms, place the pawn at the floor and leave *Move Up for Mobile VR* on.

### Desktop - Example Actor
After the plugin is enabled, find *Leap Desktop Actor* in your content browser (plugin content enabled) and place it anywhere in your scene. When you hit play your hands will be tracked from the actor center with the sensor expected to be facing upwards.

## Gestures

### Grab and Pinch

These should be globally available via Input Mapping. Look for keys *Leap (L)/(R) Pinch* and *Leap (L)/(R) Grab*

![direct input](http://i.imgur.com/2oDQllv.png)

If you add input mapping to a non-input chain actor, ensure you override the default to receive the input 
![ensure input is received](http://i.imgur.com/zWMrHxn.png)

# Common Examples - Ok great but how do I interact with... stuff?

Common use cases and examples will be found here:

https://github.com/getnamo/leap-ue4-examples

e.g. basic interaction with physics actors

[![VR pickup and drop](https://img.youtube.com/vi/dkZD1JuSSnM/0.jpg)](https://youtu.be/dkZD1JuSSnM)

# Custom Blueprint & C++, the Leap Component

Add a *Leap Component* to any actor of choice to access events relating to the leap motion. 

![add component](http://i.imgur.com/UOAexrc.png")

The main API is the same in both languages, consult the following sections if you're not familiar with *Actor Components*.

For more information about adding an actor component in blueprint please see [Components](https://docs.unrealengine.com/latest/INT/Engine/Blueprints/UserGuide/Components/index.html).

To add an actor component in C++ please see [Creating and Attaching actor components](https://docs.unrealengine.com/latest/INT/Programming/Tutorials/Components/1/).

## Blueprint - Example implementations

Please see blueprint *LeapLowPolyHand* for an example of how all these functions and events are used in practice.

## Leap Component Events

From the Leap Component the following events are available, with *On Leap Tracking Data* event being the default way of getting latest frame data.

For blueprint you add delegate events by selecting your Leap Component and hitting +

<img src="http://i.imgur.com/sBldvwR.png">

For C++ consult how to bind [multicast delegates](https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Delegates/Multicast/)

#### On Connected
Event called when the leap service connects. Will likely be called before game begin play so some components won't receive this call. Signature: *void*
```c++
FLeapEventSignature OnLeapConnected;
```

#### On Leap Tracking Data

Event called when new tracking data is available, typically every game tick. Signature: ```const FLeapFrameData&, Frame```. See See [FLeapFrameData](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L310).
```c++
FLeapFrameSignature OnLeapTrackingData;
```

#### On Hand Grabbed

Event called when a leap hand grab gesture is detected. Signature: ```const FLeapHandData&, Hand```, see [FLeapHandData](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L243)
```c++
FLeapHandSignature OnHandGrabbed;
```

#### On Hand Released

Event called when a leap hand release gesture is detected. Signature: ```const FLeapHandData&, Hand```, see [FLeapHandData](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L243)
```c++
FLeapHandSignature OnHandReleased;
```

#### On Hand Pinched

Event called when a leap hand pinch gesture is detected. Signature: ```const FLeapHandData&, Hand```, see [FLeapHandData](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L243)
```c++
FLeapHandSignature OnHandPinched;
```

#### On Hand Unpinched

Event called when a leap hand unpinch gesture is detected. Signature: ```const FLeapHandData&, Hand```, see [FLeapHandData](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L243)
```c++
FLeapHandSignature OnHandUnpinched;
```

#### On Hand Begin Tracking

Event called when a leap hand enters the field of view and begins tracking. Signature: ```const FLeapHandData&, Hand```, see [FLeapHandData](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L243)
```c++
FLeapHandSignature OnHandBeginTracking;
```

#### On Hand End Tracking

Event called when a leap hand exits the field of view and stops tracking. Signature: ```const FLeapHandData&, Hand```, see [FLeapHandData](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L243)
```c++
FLeapHandSignature OnHandEndTracking;
```  

#### On Left Hand Visibility Changed

Event called when the left hand tracking changes. Signature: ```bool bIsVisible```.
```c++
FLeapVisibilityBoolSignature OnLeftHandVisibilityChanged;
```

#### On Right Hand Visibility Changed

Event called when the right hand begins tracking. Signature: ```bool bIsVisible```.
```c++
FLeapVisibilityBoolSignature OnRightHandVisibilityChanged;
```

#### On Leap Policies Updated

Event called when leap policies have changed. Signature: an array of policy flags defined as ```TArray<TEnumAsByte<ELeapPolicyFlag>>```. See [ELeapPolicyFlag](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L29)

```c++
FLeapPolicySignature OnLeapPoliciesUpdated;
```

### Leap Component Function List

From the component you can also access functions to retrieve the latest frame data via polling and a convenience function to check if Left/Right hands are visible.

#### Are Hands Visible

Utility function to check if a left/right hand is visible and tracked at this moment
```c++
void AreHandsVisible(bool& LeftIsVisible, bool& RightIsVisible);
```  

#### Get Latest Frame Data

Polling function to get latest data. See [FLeapFrameData](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L310).
```c++
void GetLatestFrameData(FLeapFrameData& OutData);
```

# Blueprint - Leap Blueprint Function Library

Some settings and options are global, these will affect all tracking properties and are set via global functions available to any blueprint.

### Global Functions

#### Set Leap Mode

Set basic global leap tracking options. Useful for switching tracking fidelity or desktop/vr tracking mode. See [ELeapMode](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L13) and [ELeapTrackingFidelity](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L20)
```c++
static void SetLeapMode(ELeapMode Mode, ELeapTrackingFidelity Fidelity = ELeapTrackingFidelity::LEAP_NORMAL);
```

#### Set Leap Options

Set global leap options. See [FLeapOptions](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L99).
```c++
static void SetLeapOptions(const FLeapOptions& Options);
```
If tracking fidelity is set to custom, passed in ```TimewarpOffset```, ```TimewarpFactor```, ```HandInterpFactor```, and ```FingerInterpFactor``` settings will apply.

#### Get Leap Options

Gets currently set global options. See [FLeapOptions](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L99).

```c++
static void GetLeapOptions(FLeapOptions& OutOptions);
```

#### Get Leap Stats

Gets Leap read only stats such as api version, frame lookup and device information. See [FLeapStats](https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/Public/LeapMotionData.h#L83)

```c++
static void GetLeapStats(FLeapStats& OutStats);
```

#### Set Leap Policy

Change leap policy. See [Leap Policies](https://developer.leapmotion.com/documentation/cpp/api/Leap.Controller.html#policy-flags)

```c++
static void SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable);
```

# Packaging

### Windows

To package project plugins you will need a C++ project. If you have a blueprint only project, simply add a C++ class to your project and then package away.

Below is a link to an example video for packaging for windows. The user here had a blueprint only project and added the required C++ class to package successfully. The user also added a simple on beginplay command to ensure VR is enabled on beginplay as the default behavior is desktop for UE4.

[![Windows Packaging](https://img.youtube.com/vi/pRzm0M_a8uY/0.jpg)](https://youtu.be/pRzm0M_a8uY)



### Android
Ensure you've completed https://docs.unrealengine.com/latest/INT/Platforms/Android/GettingStarted/1/index.html at least once per engine release first, then package for you build type

#### Cardboard
Pick Android ATC, set SDK version to at least 21. See daydream for good settings.

#### Gear VR
For Gear VR follow https://docs.unrealengine.com/latest/INT/Platforms/GearVR/ instructions.

#### Daydream
For Daydream follow these instructions (https://docs.unrealengine.com/latest/INT/Platforms/GoogleVR/Daydream/)

Currently known good settings:
![Daydream NDK](http://i.imgur.com/SNkgnUp.png)

![Daydream SDK](http://i.imgur.com/1OLST4t.png)

NB: if you pick armv7, ensure you swap to https://github.com/getnamo/leap-ue4-preview/blob/android-leapC/Source/LeapMotionC/LeapMotionC.Build.cs#L152 to use armeabi-v7a (comment 153, uncomment 152)


#### Example Packaging - Cardboard
Below is a link to an example video of packaging for Cardboard after having NDK 1R5 installed. The user first changes build to android-21 then changes the default material to a simple engine material instance with a different color so that it shows up on cardboard due to simpler rendering pipeline. Finally the user selects Package-> Android ATC and installs the resulting apk using the install .bat. Remember connect your device to usb before this final step.

[![Android Packaging](https://img.youtube.com/vi/6Gb767S353o/0.jpg)](https://youtu.be/6Gb767S353o)

## Contact

During preview stages please post issues and feature requests to this [github repository issues section](https://github.com/getnamo/leap-ue4-preview/issues)

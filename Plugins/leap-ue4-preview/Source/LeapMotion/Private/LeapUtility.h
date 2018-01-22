 #pragma once

#include "LeapC.h"
#include "Core.h"

DECLARE_LOG_CATEGORY_EXTERN(LeapMotionLog, Log, All);

class FLeapUtility 
{
public:
	//Rotation
	static FRotator CombineRotators(FRotator A, FRotator B);


	//Conversion
	//To ue
	static FQuat ConvertToFQuat(LEAP_QUATERNION Quaternion);
	static FVector ConvertLeapVectorToUE(LEAP_VECTOR LeapVector);
	static FVector ConvertAndScaleLeapVectorToUE(LEAP_VECTOR LeapVector);
	static FMatrix ConvertLeapBasisMatrix(LEAP_DISTORTION_MATRIX LeapMatrix);
	static FMatrix SwapLeftHandRuleForRight(const FMatrix &UEMatrix);		//needed for all left hand basis which will be incorrect in ue format

																	//To leap
	static LEAP_VECTOR ConvertUEToLeap(FVector UEVector);
	static LEAP_VECTOR ConvertAndScaleUEToLeap(FVector UEVector);

	static float ScaleLeapFloatToUE(float LeapFloat);
	static float ScaleUEToLeap(float UEFloat);

	static void LeapSetMountToHMDOffset(FVector Offset);

	//HMD automation
	static void LeapSetShouldAdjustForFacing(bool ShouldRotate);
	static void LeapSetShouldAdjustForHMD(bool ShouldRotate, bool ShouldOffset);
	static void LeapSetShouldAdjustForMountOffset(bool ShouldAddOffset);

	/*template <typename EnumType>
	static FORCEINLINE EnumType LeapGetEnumValueFromString(const FString& EnumName, const FString& String)
	{
		UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
		if (!Enum)
		{
			return EnumType(0);
		}
		return (EnumType)Enum->FindEnumIndex(FName(*String));
	}

	template<typename TEnum>
	static FORCEINLINE FString LeapGetEnumValueToString(const FString& Name, TEnum Value)
	{
		const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
		if (!enumPtr)
		{
			return FString("Invalid");
		}

		return enumPtr->GetEnumName((int32)Value);
	}*/
};	

class LeapUtilityTimer
{
	int64 TickTime = 0;
	int64 TockTime = 0;
public:
	LeapUtilityTimer()
	{
		tick();
	}

	double unixTimeNow()
	{
		FDateTime timeUtc = FDateTime::UtcNow();
		return timeUtc.ToUnixTimestamp() * 1000 + timeUtc.GetMillisecond();
	}

	void tick()
	{
		TickTime = unixTimeNow();
	}

	int32 tock()
	{
		TockTime = unixTimeNow();
		return TockTime - TickTime;
	}
};
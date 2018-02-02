
#include "LeapMotionPrivatePCH.h"
#include "LeapMotionData.h"
#include "LeapC.h"
#include "LeapUtility.h"

#define MAX_DIGITS 5	//almost all humans have 5?
#define MAX_DIGIT_BONES 4	//some bones don't have all bones, see Leap documentation

FLeapHandData FLeapFrameData::HandForId(int32 HandId)
{
	for (auto& Hand : Hands)
	{
		//if found return hand
		if (Hand.Id == HandId)
		{
			return Hand;
		}
	}
	//not found? return an empty hand
	FLeapHandData EmptyHand;
	return EmptyHand;
}

void FLeapFrameData::SetFromLeapFrame(struct _LEAP_TRACKING_EVENT* frame)
{
	if (frame == nullptr)
	{
		return;
	}

	//Copy basics
	NumberOfHandsVisible = frame->nHands;
	FrameRate = frame->framerate;

	TimeStamp = frame->info.timestamp;

	InteractionBoxCenter = FLeapUtility::ConvertAndScaleLeapVectorToUE(frame->interaction_box_center);
	InteractionBoxSize = FLeapUtility::ConvertAndScaleLeapVectorToUE(frame->interaction_box_size);

	//Copy hand data
	if (Hands.Num() != NumberOfHandsVisible)	//always clear the hand data if number of hands changed
	{
		Hands.Empty();
	}

	LeftHandVisible = false;
	RightHandVisible = false;

	for (int i = 0; i < NumberOfHandsVisible; i++)
	{
		//Expand as necessary to fit
		if (Hands.Num() <= i) 
		{
			FLeapHandData HandData;
			Hands.Add(HandData);
		}

		const LEAP_HAND& LeapHand = frame->pHands[i];
		Hands[i].SetFromLeapHand((_LEAP_HAND*)&LeapHand);

		if (Hands[i].HandType == EHandType::LEAP_HAND_LEFT)
		{
			LeftHandVisible = true;
		}
		else if (Hands[i].HandType == EHandType::LEAP_HAND_RIGHT)
		{
			RightHandVisible= true;
		}
	}

	FrameId = frame->tracking_frame_id;
}

void FLeapFrameData::SetInterpolationPartialFromLeapFrame(struct _LEAP_TRACKING_EVENT* frame)
{
	if (frame == nullptr)
	{
		return;
	}

	if (NumberOfHandsVisible != frame->nHands)
	{
		return;
	}

	for (int i = 0; i < NumberOfHandsVisible; i++)
	{
		const LEAP_HAND& LeapHand = frame->pHands[i];
		Hands[i].SetArmPartialsFromLeapHand((_LEAP_HAND*)&LeapHand);
	}

	TimeStamp = frame->info.timestamp;
}

void FLeapFrameData::ScaleFrame(float InScale)
{
	InteractionBoxCenter *= InScale;
	InteractionBoxSize *= InScale;

	for (auto& Hand : Hands)
	{
		Hand.ScaleHand(InScale);
	}
}

void FLeapFrameData::RotateFrame(const FRotator& InRotation)
{
	InteractionBoxCenter = InRotation.RotateVector(InteractionBoxCenter);

	for (auto& Hand : Hands)
	{
		Hand.RotateHand(InRotation);
	}
}

void FLeapFrameData::TranslateFrame(const FVector& InTranslation)
{
	InteractionBoxCenter += InTranslation;

	for (auto& Hand : Hands)
	{
		Hand.TranslateHand(InTranslation);
	}
}

void FLeapHandData::SetFromLeapHand(struct _LEAP_HAND* hand)
{
	Arm.SetFromLeapBone((_LEAP_BONE*)&hand->arm);
	Confidence = hand->confidence;
	GrabAngle = hand->grab_angle;
	GrabStrength = hand->grab_strength;
	Id = hand->id;

	for (int i = 0; i < MAX_DIGITS; i++)
	{
		if (Digits.Num() <= i) //will only pay the cost of filling once
		{
			FLeapDigitData DigitData;
			Digits.Add(DigitData);
		}
		Digits[i].SetFromLeapDigit((_LEAP_DIGIT*)&hand->digits[i]);
	}

	Flags = hand->flags;

	Index.SetFromLeapDigit((_LEAP_DIGIT*)&hand->index);
	Middle.SetFromLeapDigit((_LEAP_DIGIT*)&hand->middle);
	Pinky.SetFromLeapDigit((_LEAP_DIGIT*)&hand->pinky);
	Ring.SetFromLeapDigit((_LEAP_DIGIT*)&hand->ring);
	Thumb.SetFromLeapDigit((_LEAP_DIGIT*)&hand->thumb);

	PinchDistance = FLeapUtility::ScaleLeapFloatToUE(hand->pinch_distance);
	PinchStrength = hand->pinch_strength;

	HandType = (EHandType)hand->type;

	Palm.SetFromLeapPalm((_LEAP_PALM*)&hand->palm);

	VisibleTime = ((double)hand->visible_time / 1000000.0);	//convert to seconds
}

void FLeapHandData::SetArmPartialsFromLeapHand(struct _LEAP_HAND* hand)
{
	//Arm Partial
	Arm.NextJoint = FLeapUtility::ConvertAndScaleLeapVectorToUE(hand->arm.next_joint);
	Arm.PrevJoint = FLeapUtility::ConvertAndScaleLeapVectorToUE(hand->arm.prev_joint);

	//Palm Partial
	Palm.Position = FLeapUtility::ConvertAndScaleLeapVectorToUE(hand->palm.position);

	//Debug - Set Orientation
	//Palm.Direction = ConvertLeapVectorToUE(hand->palm.direction);
	//Palm.Normal = ConvertLeapVectorToUE(hand->palm.normal);
	//Palm.Orientation = FRotationMatrix::MakeFromXZ(Palm.Direction, -Palm.Normal).Rotator();
}

void FLeapHandData::ScaleHand(float InScale)
{
	Arm.ScaleBone(InScale);

	Index.ScaleDigit(InScale);
	Middle.ScaleDigit(InScale);
	Pinky.ScaleDigit(InScale);
	Ring.ScaleDigit(InScale);
	Thumb.ScaleDigit(InScale);

	Palm.ScalePalm(InScale);
}

void FLeapHandData::RotateHand(const FRotator& InRotation)
{
	Arm.RotateBone(InRotation);

	Index.RotateDigit(InRotation);
	Middle.RotateDigit(InRotation);
	Pinky.RotateDigit(InRotation);
	Ring.RotateDigit(InRotation);
	Thumb.RotateDigit(InRotation);

	Palm.RotatePalm(InRotation);
}

void FLeapHandData::TranslateHand(const FVector& InTranslation)
{
	Arm.TranslateBone(InTranslation);

	Index.TranslateDigit(InTranslation);
	Middle.TranslateDigit(InTranslation);
	Pinky.TranslateDigit(InTranslation);
	Ring.TranslateDigit(InTranslation);
	Thumb.TranslateDigit(InTranslation);

	Palm.TranslatePalm(InTranslation);
}

void FLeapBoneData::SetFromLeapBone(struct _LEAP_BONE* bone)
{
	NextJoint = FLeapUtility::ConvertAndScaleLeapVectorToUE(bone->next_joint);
	PrevJoint = FLeapUtility::ConvertAndScaleLeapVectorToUE(bone->prev_joint);
	Rotation = FLeapUtility::ConvertToFQuat(bone->rotation).Rotator();
	Width = FLeapUtility::ScaleLeapFloatToUE(bone->width);
}

void FLeapBoneData::ScaleBone(float InScale)
{
	NextJoint *= InScale;
	PrevJoint *= InScale;
}

void FLeapBoneData::RotateBone(const FRotator& InRotation)
{
	NextJoint = InRotation.RotateVector(NextJoint);
	PrevJoint = InRotation.RotateVector(PrevJoint);
	Rotation = FLeapUtility::CombineRotators(Rotation, InRotation);
}

void FLeapBoneData::TranslateBone(const FVector& InTranslation)
{
	NextJoint += InTranslation;
	PrevJoint += InTranslation;
}

void FLeapDigitData::SetFromLeapDigit(struct _LEAP_DIGIT* digit)
{
	//set bone data
	for (int i = 0; i < MAX_DIGIT_BONES; i++)
	{
		if (Bones.Num() <= i)  //will only pay the cost of filling once
		{
			FLeapBoneData BoneData;
			Bones.Add(BoneData);
		}
		Bones[i].SetFromLeapBone((_LEAP_BONE*)&digit->bones[i]);
	}

	Distal.SetFromLeapBone((_LEAP_BONE*)&digit->distal);
	Intermediate.SetFromLeapBone((_LEAP_BONE*)&digit->intermediate);
	Metacarpal.SetFromLeapBone((_LEAP_BONE*)&digit->metacarpal);
	Proximal.SetFromLeapBone((_LEAP_BONE*)&digit->proximal);
	
	FingerId = digit->finger_id;
	IsExtended = digit->is_extended == 1;
	StabilizedTipPosition = FLeapUtility::ConvertAndScaleLeapVectorToUE(digit->stabilized_tip_position);
	TipVelocity = FLeapUtility::ConvertAndScaleLeapVectorToUE(digit->tip_velocity);
}

void FLeapDigitData::ScaleDigit(float InScale)
{
	/*for (auto& Bone : Bones)	//should the array also be used?
	{
		Bone.ScaleByInScale(InScale);
	}*/

	Distal.ScaleBone(InScale);
	Intermediate.ScaleBone(InScale);
	Metacarpal.ScaleBone(InScale);
	Proximal.ScaleBone(InScale);

	StabilizedTipPosition *= InScale;
	TipVelocity *= InScale;
}

void FLeapDigitData::RotateDigit(const FRotator& InRotation)
{
	/*for (auto& Bone : Bones)
	{
		Bone.RotateBone(InRotation);
	}*/

	Distal.RotateBone(InRotation);
	Intermediate.RotateBone(InRotation);
	Metacarpal.RotateBone(InRotation);
	Proximal.RotateBone(InRotation);

	StabilizedTipPosition = InRotation.RotateVector(StabilizedTipPosition);
	TipVelocity = InRotation.RotateVector(StabilizedTipPosition);
}

void FLeapDigitData::TranslateDigit(const FVector& InTranslation)
{
	Distal.TranslateBone(InTranslation);
	Intermediate.TranslateBone(InTranslation);
	Metacarpal.TranslateBone(InTranslation);
	Proximal.TranslateBone(InTranslation);

	StabilizedTipPosition += InTranslation;
	//TipVelocity += InTranslation;
}

void FLeapPalmData::SetFromLeapPalm(struct _LEAP_PALM* palm)
{
	Direction = FLeapUtility::ConvertLeapVectorToUE(palm->direction);

	Normal = FLeapUtility::ConvertLeapVectorToUE(palm->normal);

	Orientation = FRotationMatrix::MakeFromXZ(Direction, -Normal).Rotator();	//normal*-1.f

	Position = FLeapUtility::ConvertAndScaleLeapVectorToUE(palm->position);

	StabilizedPosition = FLeapUtility::ConvertAndScaleLeapVectorToUE(palm->stabilized_position);

	Velocity = FLeapUtility::ConvertAndScaleLeapVectorToUE(palm->velocity);

	Width = FLeapUtility::ScaleLeapFloatToUE(palm->width);
}

void FLeapPalmData::ScalePalm(float InScale)
{
	Position *= InScale;
	StabilizedPosition *= InScale;
	Velocity *= InScale;
}

void FLeapPalmData::RotatePalm(const FRotator& InRotation)
{
	Position = InRotation.RotateVector(Position);
	StabilizedPosition = InRotation.RotateVector(StabilizedPosition);
	Velocity = InRotation.RotateVector(Velocity);
	Direction = InRotation.RotateVector(Direction);
	Normal = InRotation.RotateVector(Normal);
	Orientation = FLeapUtility::CombineRotators(Orientation, InRotation);
}

void FLeapPalmData::TranslatePalm(const FVector& InTranslation)
{
	Position += InTranslation;
	StabilizedPosition += InTranslation;
	//Velocity += InTranslation;
}

FLeapOptions::FLeapOptions()
{
	//Good Vive settings used as defaults
	Mode = LEAP_MODE_VR;//LEAP_MODE_DESKTOP
	TrackingFidelity = LEAP_SMOOTH;//LEAP_NORMAL
	bUseTimeWarp = true;
	bUseInterpolation = true;
	bTransformOriginToHMD = true;
	TimewarpOffset = 5500;
	TimewarpFactor = 1.f;
	HandInterpFactor = 0.f;
	FingerInterpFactor = 0.f;
	HMDOffset = FVector(8.0, 0, 0);		//Vive default, for oculus use 8,0,0
	bEnableImageStreaming = false;		//default image streaming to off
}

FLeapStats::FLeapStats()
{
	
}

void FLeapDevice::SetFromLeapDevice(struct _LEAP_DEVICE_INFO* LeapInfo)
{
	Status = LeapInfo->status;
	Caps = LeapInfo->caps;
	PID = FString(LeapDevicePIDToString(LeapInfo->pid));
	Baseline = LeapInfo->baseline;
	Serial = LeapInfo->serial;
	HorizontalFOV = LeapInfo->h_fov;
	VerticalFOV = LeapInfo->v_fov;
	Range = LeapInfo->range;
}

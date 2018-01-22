#include "LeapMotionPrivatePCH.h"
#include "IHeadMountedDisplay.h"

DEFINE_LOG_CATEGORY(LeapMotionLog);

#define LEAP_TO_UE_SCALE 0.1f
#define UE_TO_LEAP_SCALE 10.f

FVector LeapMountOffset = FVector(8.f, 0, 0);
//FQuat FacingAdjustQuat = FQuat(FRotator(90.f, 0.f, 180.f));
FQuat FacingAdjustQuat = FQuat(FRotator(90.f, 0.f, 0.f));
FQuat LeapRotationOffset = FQuat(FRotator(90.f, 0.f, 180.f));

FQuat PreQuat = FQuat(FRotator(0.f, -90.f, 0.f));
FQuat PostQuat = FQuat(FRotator(0.f, 0.f, 90.f));

//NB: localized variable for transforms - defaults
bool LeapShouldAdjustForFacing = false; //true
bool LeapShouldAdjustRotationForHMD = false;
bool LeapShouldAdjustPositionForHMD = false;
bool LeapShouldAdjustForMountOffset = true;	//todo: re-enable

//Todo: use and verify this for all values
float LeapGetWorldScaleFactor()
{
	if (GEngine != nullptr && GEngine->GetWorld() != nullptr)
	{
		return (GEngine->GetWorld()->GetWorldSettings()->WorldToMeters) / 100.f;
	}
	return 1.f;
}


FRotator FLeapUtility::CombineRotators(FRotator A, FRotator B)
{
	FQuat AQuat = FQuat(A);
	FQuat BQuat = FQuat(B);

	return FRotator(BQuat*AQuat);
}

//Single point to handle leap conversion
FVector ConvertToFVector(const LEAP_VECTOR& LeapVector)
{
	//Expects VR Orientation
	return FVector(LeapVector.y, -LeapVector.x, -LeapVector.z);
}

FQuat FLeapUtility::ConvertToFQuat(LEAP_QUATERNION LeapQuat)
{
	FQuat Quat;
	
	//it's -Z, X, Y tilted back by 90 degree which is -y,x,z
	Quat.X = -LeapQuat.y;
	Quat.Y = LeapQuat.x;
	Quat.Z = LeapQuat.z;
	Quat.W = LeapQuat.w;

	return Quat * LeapRotationOffset;
}

FVector AdjustForLeapFacing(FVector In)
{
	return FacingAdjustQuat.RotateVector(In);
}

FVector AdjustForHMD(FVector In)
{
	if (GEngine->HMDDevice.IsValid())
	{
		FQuat OrientationQuat;
		FVector Position;
		GEngine->HMDDevice->GetCurrentOrientationAndPosition(OrientationQuat, Position);
		FVector Out = OrientationQuat.RotateVector(In);
		if (LeapShouldAdjustPositionForHMD)
		{
			if (LeapShouldAdjustForMountOffset)
			{
				Position += OrientationQuat.RotateVector(LeapMountOffset);
			}
			Out += Position;
		}
		return Out;
	}
	else
	{
		return In;
	}
}

FVector AdjustForHMDOrientation(FVector In)
{
	if (GEngine->HMDDevice.IsValid())
	{
		FQuat OrientationQuat;
		FVector Position;
		GEngine->HMDDevice->GetCurrentOrientationAndPosition(OrientationQuat, Position);
		FVector Out = OrientationQuat.RotateVector(In);
		return Out;
	}
	else
		return In;

}

//Conversion - use find and replace to change behavior, no scaling version is typically used for orientations
FVector FLeapUtility::ConvertLeapVectorToUE(LEAP_VECTOR LeapVector)
{
	//Convert Axis
	FVector ConvertedVector = ConvertToFVector(LeapVector);

	//Hmd orientation adjustment
	if (LeapShouldAdjustForFacing)
	{
		ConvertedVector = AdjustForLeapFacing(ConvertedVector);
		if (LeapShouldAdjustRotationForHMD)
		{
			if (GEngine->HMDDevice.IsValid())
			{
				FQuat orientationQuat;
				FVector position;
				GEngine->HMDDevice->GetCurrentOrientationAndPosition(orientationQuat, position);
				ConvertedVector = orientationQuat.RotateVector(ConvertedVector);
			}
		}
	}
	return ConvertedVector;
}

FVector FLeapUtility::ConvertAndScaleLeapVectorToUE(LEAP_VECTOR LeapVector)
{
	//Scale from mm to cm (ue default)
	FVector ConvertedVector = (ConvertToFVector(LeapVector) + LeapMountOffset) * (LEAP_TO_UE_SCALE * LeapGetWorldScaleFactor());

	//Front facing leap adjustments
	if (LeapShouldAdjustForFacing)
	{
		ConvertedVector = AdjustForLeapFacing(ConvertedVector);
		if (LeapShouldAdjustRotationForHMD)
		{
			ConvertedVector = AdjustForHMD(ConvertedVector);
		}
	}
	return ConvertedVector;
}

FMatrix FLeapUtility::ConvertLeapBasisMatrix(LEAP_DISTORTION_MATRIX LeapMatrix)
{
	/*
	Leap Basis depends on hand type with -z, x, y as general format. This then needs to be inverted to point correctly (x = forward), which yields the matrix below.
	*/
	FVector InX, InY, InZ, InW;
	/*InX.Set(LeapMatrix.zBasis.z, -LeapMatrix.zBasis.x, -LeapMatrix.zBasis.y);
	InY.Set(-LeapMatrix.xBasis.z, LeapMatrix.xBasis.x, LeapMatrix.xBasis.y);
	InZ.Set(-LeapMatrix.yBasis.z, LeapMatrix.yBasis.x, LeapMatrix.yBasis.y);
	InW.Set(-LeapMatrix.origin.z, LeapMatrix.origin.x, LeapMatrix.origin.y);

	if (LeapShouldAdjustForFacing)
	{
		InX = AdjustForLeapFacing(InX);
		InY = AdjustForLeapFacing(InY);
		InZ = AdjustForLeapFacing(InZ);
		InW = AdjustForLeapFacing(InW);

		if (LeapShouldAdjustRotationForHMD)
		{
			InX = adjustForHMDOrientation(InX);
			InY = adjustForHMDOrientation(InY);
			InZ = adjustForHMDOrientation(InZ);
			InW = adjustForHMDOrientation(InW);
		}
	}
	
	Disabled for now, not sure what the equivalent is now
	*/

	return (FMatrix(InX, InY, InZ, InW));
}
FMatrix FLeapUtility::SwapLeftHandRuleForRight(const FMatrix& UEMatrix)
{
	FMatrix Matrix = UEMatrix;
	//Convenience method to swap the axis correctly, already in UE format to swap Y instead of leap Z
	FVector InverseVector = -Matrix.GetUnitAxis(EAxis::Y);
	Matrix.SetAxes(NULL, &InverseVector, NULL, NULL);
	return Matrix;
}

LEAP_VECTOR FLeapUtility::ConvertUEToLeap(FVector UEVector)
{
	LEAP_VECTOR vector;
	vector.x = UEVector.Y;
	vector.y = UEVector.Z;
	vector.z = -UEVector.X;
	return vector;
}

LEAP_VECTOR FLeapUtility::ConvertAndScaleUEToLeap(FVector UEVector)
{
	LEAP_VECTOR vector;
	vector.x = UEVector.Y * UE_TO_LEAP_SCALE;
	vector.y = UEVector.Z * UE_TO_LEAP_SCALE;
	vector.z = -UEVector.X * UE_TO_LEAP_SCALE;
	return vector;
}

float FLeapUtility::ScaleLeapFloatToUE(float LeapFloat)
{
	return LeapFloat * LEAP_TO_UE_SCALE;	//mm->cm
}

float FLeapUtility::ScaleUEToLeap(float UEFloat)
{
	return UEFloat * UE_TO_LEAP_SCALE;	//mm->cm
}


void FLeapUtility::LeapSetMountToHMDOffset(FVector Offset)
{
	LeapMountOffset = Offset;
}

void FLeapUtility::LeapSetShouldAdjustForFacing(bool ShouldRotate)
{
	LeapShouldAdjustForFacing = ShouldRotate;
}

void FLeapUtility::LeapSetShouldAdjustForHMD(bool ShouldRotate, bool ShouldOffset)
{
	LeapShouldAdjustRotationForHMD = ShouldRotate;
	LeapShouldAdjustPositionForHMD = ShouldOffset;
}

void FLeapUtility::LeapSetShouldAdjustForMountOffset(bool ShouldAddOffset)
{
	LeapShouldAdjustForMountOffset = ShouldAddOffset;
}

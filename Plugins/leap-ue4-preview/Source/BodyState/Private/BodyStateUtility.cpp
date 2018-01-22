#include "BodyStatePrivatePCH.h"

DEFINE_LOG_CATEGORY(BodyStateLog);

FRotator BodyStateUtility::CombineRotators(FRotator A, FRotator B)
{
	FQuat AQuat = FQuat(A);
	FQuat BQuat = FQuat(B);

	return FRotator(BQuat*AQuat);
}
#pragma once
#include "Core.h"

DECLARE_LOG_CATEGORY_EXTERN(BodyStateLog, Log, All);

class BodyStateUtility
{
public:
	//Rotation
	static FRotator CombineRotators(FRotator A, FRotator B);
};
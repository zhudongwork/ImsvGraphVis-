// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "BodyStateBone.h"
#include "BodyStateArm.h"
#include "BodyStateEnums.h"
#include "BodyStateSkeleton.generated.h"

//Used for replication
USTRUCT(BlueprintType)
struct BODYSTATE_API FNamedBoneData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FBodyStateBoneData Data;

	UPROPERTY()
	EBodyStateBasicBoneType Name;
};

//Used for replication
USTRUCT(BlueprintType)
struct BODYSTATE_API FKeyedTransform
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	EBodyStateBasicBoneType Name;
};

//Used for replication
USTRUCT(BlueprintType)
struct BODYSTATE_API FNamedBoneMeta
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FBodyStateBoneMeta Meta;

	UPROPERTY()
	EBodyStateBasicBoneType Name;
};

//Used for replication
USTRUCT(BlueprintType)
struct BODYSTATE_API FNamedSkeletonData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FNamedBoneData> TrackedAdvancedBones;

	UPROPERTY()
	TArray<FKeyedTransform> TrackedBasicBones;

	UPROPERTY()
	TArray<FNamedBoneMeta> UniqueMetas;
};

/** Body Skeleton data, all bones are expected in component space*/
UCLASS(BlueprintType)
class BODYSTATE_API UBodyStateSkeleton : public UObject
{
	GENERATED_UCLASS_BODY()

	//Note: Storage of actual bone data should be here
	UPROPERTY(BlueprintReadOnly, Category = "BodyState Skeleton")
	TArray<UBodyStateBone*> Bones;		//All bones stored here

	//internal lookup for the bones
	TMap<EBodyStateBasicBoneType, UBodyStateBone*> BoneMap;

	//Used for reference point calibration e.g. hydra base origin
	UPROPERTY(BlueprintReadOnly, Category = "BodyState Skeleton")
	FTransform RootOffset;

	//Convenience bone getters

	//Root
	UFUNCTION(BlueprintPure, Category = "BodyState Skeleton")
	UBodyStateBone* RootBone();

	//Arms & Hands

	/** Get a structured convenience wrapper around left arm bones*/
	UFUNCTION(BlueprintPure, Category = "BodyState Skeleton")
	UBodyStateArm* LeftArm();

	/** Get a structured convenience wrapper around right arm bones*/
	UFUNCTION(BlueprintPure, Category = "BodyState Skeleton")
	UBodyStateArm* RightArm();

	//Spine & Head
	UFUNCTION(BlueprintPure, Category = "BodyState Skeleton")
	UBodyStateBone* Head();

	/*Get Bone data by enum*/
	UFUNCTION(BlueprintPure, Category = "BodyState Skeleton")
	class UBodyStateBone* BoneForEnum(EBodyStateBasicBoneType Bone);

	/*Get Bone data by name matching*/
	UFUNCTION(BlueprintPure, Category = "BodyState Skeleton")
	class UBodyStateBone* BoneNamed(const FString& Name);

	//Replication and Setting Data

	//Setting Bone Data
	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void ResetToDefaultSkeleton();

	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void SetDataForBone(const FBodyStateBoneData& BoneData, EBodyStateBasicBoneType Bone);

	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void SetTransformForBone(const FTransform& Transform, EBodyStateBasicBoneType Bone);

	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void SetMetaForBone(const FBodyStateBoneMeta& BoneMeta, EBodyStateBasicBoneType Bone);

	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void ChangeBasis(const FRotator& PreBase, const FRotator& PostBase, bool AdjustVectors = true);

	//Conversion
	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	FNamedSkeletonData GetMinimalNamedSkeletonData();	//key replication getter

	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void SetFromNamedSkeletonData(const FNamedSkeletonData& NamedSkeletonData);	//key replication setter

	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void SetFromOtherSkeleton(UBodyStateSkeleton* Other);

	//Replication
	UFUNCTION(Unreliable, Server, WithValidation)
	void ServerUpdateBodyState(const FNamedSkeletonData InBodyStateSkeleton);

	UFUNCTION(NetMulticast, Unreliable)
	void Multi_UpdateBodyState(const FNamedSkeletonData InBodyStateSkeleton);

protected:
	TArray<FNamedBoneData> TrackedBoneData();
	TArray<FKeyedTransform> TrackedBasicBones();
	TArray<FNamedBoneData> TrackedAdvancedBones();
	TArray<FNamedBoneMeta> UniqueBoneMetas();

private:
	UPROPERTY()
	UBodyStateArm* PrivateLeftArm;
	
	UPROPERTY()
	UBodyStateArm* PrivateRightArm;
};

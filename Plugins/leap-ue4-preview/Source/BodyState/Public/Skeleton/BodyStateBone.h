#pragma once

#include "BodyStateBone.generated.h"


USTRUCT(BlueprintType)
struct BODYSTATE_API FBodyStateBoneMeta
{
	GENERATED_USTRUCT_BODY()

	/** Is this meta data distinct from parents? */
	UPROPERTY()
	bool ParentDistinctMeta;

	/** Name of tracking type*/
	UPROPERTY()
	FString TrackingType;

	/** Accuracy in cm of tracking data if distinct */
	UPROPERTY()
	float Accuracy;


	FBodyStateBoneMeta()
	{
		ParentDistinctMeta = false;
		TrackingType = FString(TEXT("Unknown"));
		Accuracy = 0.f;
	}
};

USTRUCT(BlueprintType)
struct BODYSTATE_API FBodyStateBoneData
{
	GENERATED_USTRUCT_BODY()

	/** Transform holding main bone values*/
	UPROPERTY(BlueprintReadWrite, Category = "BodyState Bone Data")
	FTransform Transform;

	/** If this bone tracks more than just transform*/
	UPROPERTY(BlueprintReadWrite, Category = "BodyState Bone Data")
	bool AdvancedBoneType;

	/** Tracking Confidence */
	UPROPERTY(BlueprintReadWrite, Category = "BodyState Bone Data")
	float Confidence;

	/** Blending Alpha */
	UPROPERTY(BlueprintReadWrite, Category = "BodyState Bone Data")
	float Alpha;

	/** Bone Length */
	UPROPERTY(BlueprintReadWrite, Category = "BodyState Bone Data")
	float Length;

	FBodyStateBoneData()
	{
		Transform.SetScale3D(FVector(1.f));
		Confidence = 0.f;
		Alpha = 1.f;
		Length = 1.f;
		AdvancedBoneType = false;
	}

	/** If you're only tracking transform */
	void SetFromTransform(FTransform InTransform)
	{
		FBodyStateBoneData();
		Transform = InTransform;
		Confidence = 1.f;
	}
};

UCLASS(BlueprintType)
class BODYSTATE_API UBodyStateBone : public UObject
{
	GENERATED_UCLASS_BODY()

	/** Human readable name */
	UPROPERTY(BlueprintReadWrite, Category = "BodyState Bone")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "BodyState Bone")
	FBodyStateBoneData BoneData;

	UPROPERTY(BlueprintReadWrite, Category = "BodyState Bone")
	FBodyStateBoneMeta Meta;

	/** Parent Bone - If available, weak links */
	UPROPERTY(BlueprintReadWrite, Category = "BodyState Bone")
	UBodyStateBone* Parent;

	/** Children Bones - If available, weak links */
	UPROPERTY(BlueprintReadWrite, Category = "BodyState Bone")
	TArray<UBodyStateBone*> Children;

	/** Bone Position */
	UFUNCTION(BlueprintPure, Category = "BodyState Bone")
	FVector Position();

	UFUNCTION(BlueprintCallable, Category = "BodyState Bone")
	void SetPosition(const FVector& InPosition);

	/** Bone Orientation */
	UFUNCTION(BlueprintPure, Category = "BodyState Bone")
	FRotator Orientation();

	UFUNCTION(BlueprintCallable, Category = "BodyState Bone")
	void SetOrientation(const FRotator& InOrientation);

	/** Bone Scale */
	UFUNCTION(BlueprintPure, Category = "BodyState Bone")
	FVector Scale();

	UFUNCTION(BlueprintCallable, Category = "BodyState Bone")
	void SetScale(const FVector& InScale);

	/** Get Unique Meta from first unique parent*/
	UFUNCTION(BlueprintPure, Category = "BodyState Bone")
	FBodyStateBoneMeta UniqueMeta();

	/** Re-initialize from bone data */
	void InitializeFromBoneData(const FBodyStateBoneData& InData);
	void Initialize();

	/** Add a child with this as parent link to bone */
	void AddChild(UBodyStateBone* InChild);

	//Convenience Functions
	UFUNCTION(BlueprintCallable, Category = "BodyState Bone")
	virtual bool Enabled();

	UFUNCTION(BlueprintCallable, Category = "BodyState Bone")
	virtual void SetEnabled(bool Enable = true);

	UFUNCTION(BlueprintCallable, Category = "BodyState Bone")
	virtual void ShiftBone(FVector ShiftAmount);

	UFUNCTION(BlueprintCallable, Category = "BodyState Bone")
	virtual void ChangeBasis(const FRotator& PreBase, const FRotator& PostBase, bool AdjustVectors = true);

	UFUNCTION(BlueprintPure, Category = "BodyState Bone")
	virtual bool IsTracked();
};
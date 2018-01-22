
#pragma once

#include "Runtime/Engine/Classes/Animation/AnimInstance.h"
#include "BodyStateEnums.h"
#include "BodyStateSkeleton.h"
#include "BodyStateAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct BODYSTATE_API FBodyStateIndexedBone
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Indexed Bone")
	FName BoneName;

	UPROPERTY(BlueprintReadWrite, Category = "Indexed Bone")
	int32 ParentIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Indexed Bone")
	int32 Index;

	FBodyStateIndexedBone()
	{
		ParentIndex = -1;
		Index = -1;
	}
};

//C++ only struct used for cached bone lookup
struct CachedBoneLink
{
	FBoneReference MeshBone;
	UBodyStateBone* BSBone;
	//int32 TraverseCount;
};

USTRUCT(BlueprintType)
struct FMappedBoneAnimData
{
	GENERATED_BODY()

	/** Whether the mesh should deform to match the tracked data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Anim Struct")
	bool bShouldDeformMesh;

	/** Offset rotation base applied before given rotation (will rotate input) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance")
	FRotator PreBaseRotation;

	/** Offset rotation base applied after given rotation (consider this an offset) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance")
	FRotator PostBaseRotation;

	/** Matching list of body state bone keys mapped to local mesh bone names */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Anim Struct")
	TMap<EBodyStateBasicBoneType, FBoneReference> BoneMap;

	/** Skeleton driving our data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Anim Struct")
	class UBodyStateSkeleton* BodyStateSkeleton;

	//Data structure containing a parent -> child ordered bone list
	TArray<CachedBoneLink> CachedBoneList;

	FMappedBoneAnimData()
	{
		bShouldDeformMesh = true;
	}

	void SyncCachedList();
};


UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class UBodyStateAnimInstance : public UAnimInstance
{
public:
	GENERATED_UCLASS_BODY()

	/** Whether the anim instance should autodetect and fill the bonemap on anim init*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance")
	bool bAutoDetectBoneMapAtInit;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance")
	int32 DefaultBodyStateIndex;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance")
	EBodyStateHandType AutoMapHandTarget;

	/** Adjust rotation by currently defines offset base rotators */
	UFUNCTION(BlueprintPure, Category = "BS Anim Instance")
	FRotator AdjustRotationByBasis(const FRotator& InRotator);

	/** Link given mesh bone with body state bone enum. */
	UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	void AddBSBoneToMeshBoneLink(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData, EBodyStateBasicBoneType BSBone, FName MeshBone);

	/** Remove a link. Useful when e.g. autorigging gets 80% there but you need to remove a bone. */
	UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	void RemoveBSBoneLink(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData, EBodyStateBasicBoneType BSBone);

	UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	void SetAnimSkeleton(UBodyStateSkeleton* InSkeleton);

	//UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	TMap<EBodyStateBasicBoneType, FBoneReference> AutoDetectHandBones(USkeletalMeshComponent* Component, EBodyStateHandType HandType = EBodyStateHandType::BodyState_HAND_LEFT);

	/** Struct containing all variables needed at anim node time */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance")
	FMappedBoneAnimData MappedBoneData;

	UFUNCTION(BlueprintPure, Category = "BS Anim Instance")
	FString BoneMapSummary();

	//Manual sync
	UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	void SyncMappedBoneDataCache(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData);

protected:

	// traverse a bone index node until you hit -1, count the hops
	int32 TraverseLengthForIndex(int32 Index);
	
	//Internal Map with parent information
	TArray<FBodyStateIndexedBone> BoneLookupList;
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> IndexedBoneMap;
	TMap<EBodyStateBasicBoneType, FBoneReference> ToBoneReferenceMap(TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> InIndexedMap);
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> AutoDetectHandIndexedBones(USkeletalMeshComponent* Component, EBodyStateHandType HandType = EBodyStateHandType::BodyState_HAND_LEFT);

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
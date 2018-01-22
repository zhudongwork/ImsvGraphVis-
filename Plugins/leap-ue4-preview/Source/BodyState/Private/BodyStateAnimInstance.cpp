#include "BodyStatePrivatePCH.h"
#include "BodyStateBPLibrary.h"
#include "BodyStateAnimInstance.h"

UBodyStateAnimInstance::UBodyStateAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//Defaults
	bAutoDetectBoneMapAtInit = true;
	DefaultBodyStateIndex = 0;
	AutoMapHandTarget = EBodyStateHandType::BodyState_HAND_LEFT;
}

FRotator UBodyStateAnimInstance::AdjustRotationByBasis(const FRotator& InRotator)
{
	const FRotator Temp = BodyStateUtility::CombineRotators(MappedBoneData.PreBaseRotation, InRotator);
	return BodyStateUtility::CombineRotators(Temp, MappedBoneData.PostBaseRotation);
}

void UBodyStateAnimInstance::AddBSBoneToMeshBoneLink(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData, EBodyStateBasicBoneType BSBone, FName MeshBone)
{
	FBoneReference BoneRef;
	BoneRef.BoneName = MeshBone;
	BoneRef.Initialize(CurrentSkeleton);
	InMappedBoneData.BoneMap.Add(BSBone, MeshBone);
	InMappedBoneData.SyncCachedList();
}

void UBodyStateAnimInstance::RemoveBSBoneLink(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData, EBodyStateBasicBoneType BSBone)
{
	InMappedBoneData.BoneMap.Remove(BSBone);
	InMappedBoneData.SyncCachedList();
}

void UBodyStateAnimInstance::SetAnimSkeleton(UBodyStateSkeleton* InSkeleton)
{
	MappedBoneData.BodyStateSkeleton = InSkeleton;

	//Re-cache our results
	MappedBoneData.SyncCachedList();
}

TMap<EBodyStateBasicBoneType, FBoneReference> UBodyStateAnimInstance::AutoDetectHandBones(USkeletalMeshComponent* Component, EBodyStateHandType HandType /*= EBodyStateHandType::BodyState_HAND_LEFT*/)
{
	auto IndexedMap = AutoDetectHandIndexedBones(Component, HandType);
	return ToBoneReferenceMap(IndexedMap);
}

FString UBodyStateAnimInstance::BoneMapSummary()
{
	FString Result = TEXT("+== Bone Summary ==+");

	//Concatenate indexed bones
	for (auto Bone : IndexedBoneMap)
	{
		FBodyStateIndexedBone& IndexedBone = Bone.Value;
		Result += FString::Printf(TEXT("BoneString: %s:%d(%d)\n"), *IndexedBone.BoneName.ToString(), IndexedBone.Index, IndexedBone.ParentIndex);
	}
	return Result;
}

void UBodyStateAnimInstance::SyncMappedBoneDataCache(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData)
{
	InMappedBoneData.SyncCachedList();
}

//////////////
/////Protected

//Local data only used for auto-mapping algorithm
struct FIndexMax
{
	int32 Index;
	int32 Max;
};

struct FIndexParentsCount
{
	TArray<FIndexMax> ParentsList;

	int32 FindCount(int32 ParentIndex)
	{
		int32 DidFindIndex = -1;
		for (int32 i = 0; i < ParentsList.Num(); i++)
		{
			if (ParentsList[i].Index == ParentIndex)
			{
				return i;
			}
		}
		return DidFindIndex;
	}

	//run after filling our index
	TArray<int32> FindParentsWithCount(int32 Count)
	{
		TArray<int32> ResultArray;

		for (auto Parent : ParentsList)
		{
			if (Parent.Max == Count)
			{
				ResultArray.Add(Parent.Index);
			}
		}
		return ResultArray;
	}
};

TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> UBodyStateAnimInstance::AutoDetectHandIndexedBones(USkeletalMeshComponent* Component, EBodyStateHandType HandType)
{
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> AutoBoneMap;

	if (Component == nullptr)
	{
		return AutoBoneMap;
	}

	//Get bones and parent indices
	USkeletalMesh* SkeletalMesh = Component->SkeletalMesh;
	FReferenceSkeleton& RefSkeleton = SkeletalMesh->RefSkeleton;

	//Root bone
	int32 RootBone = -2;

	//Palm bone
	int32 PalmBone = -2;

	//Finger roots
	int32 ThumbBone = -2;
	int32 IndexBone = -2;
	int32 MiddleBone = -2;
	int32 RingBone = -2;
	int32 PinkyBone = -2;

	FIndexParentsCount ParentsCount;

	//Re-organize our bone information
	BoneLookupList.Empty(RefSkeleton.GetNum());
	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		FBodyStateIndexedBone Bone;
		Bone.BoneName = RefSkeleton.GetBoneName(i);
		Bone.ParentIndex = RefSkeleton.GetParentIndex(i);
		Bone.Index = i;
		BoneLookupList.Add(Bone);

		//Set our root bone
		if (Bone.ParentIndex == -1)
		{
			RootBone = i;
		}

		//Get the parent list count so we can identify our palm bone
		int32 ExistingCountIndex = ParentsCount.FindCount(Bone.ParentIndex);
		if (ExistingCountIndex != -1)
		{
			FIndexMax& IndexMax = ParentsCount.ParentsList[ExistingCountIndex];
			IndexMax.Max++;
		}
		else
		{
			FIndexMax IndexMax;
			IndexMax.Index = Bone.ParentIndex;
			IndexMax.Max = 1;
			ParentsCount.ParentsList.Add(IndexMax);
		}
	}

	//Find our palm bone
	TArray<int32> HandParents = ParentsCount.FindParentsWithCount(5);

	//Multiple hand parent bones found, let's pick the closest type
	if (HandParents.Num() > 1)
	{
		FString SearchString;
		if (HandType == EBodyStateHandType::BodyState_HAND_RIGHT)
		{
			SearchString = TEXT("r");
		}
		else
		{
			SearchString = TEXT("l");
		}

		//Check the multiple bones for L/R (or lowercase) match
		for (int32 Index : HandParents)
		{
			FBodyStateIndexedBone& IndexedBone = BoneLookupList[Index];
			bool HasSearchString = IndexedBone.BoneName.ToString().ToLower().Contains(SearchString);
			if (HasSearchString)
			{
				PalmBone = Index;
				break;
			}
		}
		//Palm bone still not set?
		if (PalmBone == -2)
		{
			//Pick the first one
			PalmBone = HandParents[0];
		}

		//Todo: expand to left/right, l/r, or 0/1 substring for searches
	}
	//Single bone of this type found, set that one
	else if (HandParents.Num() == 1)
	{
		PalmBone = HandParents[0];
	}
	else
	{
		//We couldn't figure out where the palm is, return an empty map
		return AutoBoneMap;
	}

	int32 WristBone = RefSkeleton.GetParentIndex(PalmBone);

	//Get all the child bones with that parent index
	for (auto Bone : BoneLookupList)
	{
		bool IsPalmChild = (Bone.ParentIndex == PalmBone);

		if (IsPalmChild)
		{
			if (ThumbBone == -2)
			{
				ThumbBone = Bone.Index;
			}
			else if (IndexBone == -2)
			{
				IndexBone = Bone.Index;
			}
			else if (MiddleBone == -2)
			{
				MiddleBone = Bone.Index;
			}
			else if (RingBone == -2)
			{
				RingBone = Bone.Index;
			}
			else
			{
				PinkyBone = Bone.Index;
			}
		}
	}

	//UE_LOG(LogTemp, Log, TEXT("Palm: %d"), PalmBone);
	//UE_LOG(LogTemp, Log, TEXT("T:%d, I:%d, M: %d, R: %d, P: %d"), ThumbBone, IndexBone, MiddleBone, RingBone, PinkyBone);

	//Based on the passed hand type map the indexed bones to our EBodyStateBasicBoneType enums
	if (HandType == EBodyStateHandType::BodyState_HAND_LEFT)
	{
		if (WristBone >= 0)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_HAND_WRIST_L, BoneLookupList[WristBone]);
		}
		if (ThumbBone >= 0 && (ThumbBone + 2) < IndexBone)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_L, BoneLookupList[ThumbBone]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_THUMB_1_PROXIMAL_L, BoneLookupList[ThumbBone + 1]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_L, BoneLookupList[ThumbBone + 2]);
		}
		if (IndexBone >= 0 && (IndexBone + 3) < MiddleBone)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_INDEX_0_METACARPAL_L, BoneLookupList[IndexBone]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_INDEX_1_PROXIMAL_L, BoneLookupList[IndexBone + 1]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_INDEX_2_INTERMEDIATE_L, BoneLookupList[IndexBone + 2]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_INDEX_3_DISTAL_L, BoneLookupList[IndexBone + 3]);
		}
		if (MiddleBone >= 0 && (MiddleBone + 3) < RingBone)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_L, BoneLookupList[MiddleBone]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_L, BoneLookupList[MiddleBone + 1]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_MIDDLE_2_INTERMEDIATE_L, BoneLookupList[MiddleBone + 2]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_MIDDLE_3_DISTAL_L, BoneLookupList[MiddleBone + 3]);
		}
		if (RingBone >= 0 && (RingBone + 3) < PinkyBone)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_RING_0_METACARPAL_L, BoneLookupList[RingBone]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_RING_1_PROXIMAL_L, BoneLookupList[RingBone + 1]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_RING_2_INTERMEDIATE_L, BoneLookupList[RingBone + 2]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_RING_3_DISTAL_L, BoneLookupList[RingBone + 3]);
		}
		if (PinkyBone >= 0 && (PinkyBone + 3) < BoneLookupList.Num())
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_L, BoneLookupList[PinkyBone]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_L, BoneLookupList[PinkyBone + 1]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_PINKY_2_INTERMEDIATE_L, BoneLookupList[PinkyBone + 2]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_PINKY_3_DISTAL_L, BoneLookupList[PinkyBone + 3]);
		}
	}
	//Right Hand
	else
	{
		if (WristBone >= 0)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_HAND_WRIST_R, BoneLookupList[WristBone]);
		}
		if (ThumbBone >= 0 && (ThumbBone + 2) < IndexBone)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_R, BoneLookupList[ThumbBone]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_THUMB_1_PROXIMAL_R, BoneLookupList[ThumbBone + 1]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_R, BoneLookupList[ThumbBone + 2]);
		}
		if (IndexBone >= 0 && (IndexBone + 3) < MiddleBone)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_INDEX_0_METACARPAL_R, BoneLookupList[IndexBone]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_INDEX_1_PROXIMAL_R, BoneLookupList[IndexBone + 1]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_INDEX_2_INTERMEDIATE_R, BoneLookupList[IndexBone + 2]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_INDEX_3_DISTAL_R, BoneLookupList[IndexBone + 3]);
		}
		if (MiddleBone >= 0 && (MiddleBone + 3) < RingBone)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_R, BoneLookupList[MiddleBone]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_R, BoneLookupList[MiddleBone + 1]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_MIDDLE_2_INTERMEDIATE_R, BoneLookupList[MiddleBone + 2]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_MIDDLE_3_DISTAL_R, BoneLookupList[MiddleBone + 3]);
		}
		if (RingBone >= 0 && (RingBone + 3) < PinkyBone)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_RING_0_METACARPAL_R, BoneLookupList[RingBone]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_RING_1_PROXIMAL_R, BoneLookupList[RingBone + 1]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_RING_2_INTERMEDIATE_R, BoneLookupList[RingBone + 2]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_RING_3_DISTAL_R, BoneLookupList[RingBone + 3]);
		}
		if (PinkyBone >= 0 && (PinkyBone + 3) < BoneLookupList.Num())
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_R, BoneLookupList[PinkyBone]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_R, BoneLookupList[PinkyBone + 1]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_PINKY_2_INTERMEDIATE_R, BoneLookupList[PinkyBone + 2]);
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_PINKY_3_DISTAL_R, BoneLookupList[PinkyBone + 3]);
		}
	}

	return AutoBoneMap;
}

int32 UBodyStateAnimInstance::TraverseLengthForIndex(int32 Index)
{
	if (Index == -1 || Index>= BoneLookupList.Num())
	{
		return 0;	//this is the root or invalid bone
	}
	else
	{
		FBodyStateIndexedBone& Bone = BoneLookupList[Index];
		
		//Add our parent traversal + 1
		return TraverseLengthForIndex(Bone.ParentIndex) + 1;
	}
}

TMap<EBodyStateBasicBoneType, FBoneReference> UBodyStateAnimInstance::ToBoneReferenceMap(TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> InIndexedMap)
{
	TMap<EBodyStateBasicBoneType, FBoneReference> ReferenceMap;

	for (auto BonePair : IndexedBoneMap)
	{
		FBoneReference BoneReference;
		BoneReference.BoneName = BonePair.Value.BoneName;
		BoneReference.Initialize(CurrentSkeleton);

		ReferenceMap.Add(BonePair.Key, BoneReference);
	}
	return ReferenceMap;
}


void UBodyStateAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	//Grab our skel mesh component
	USkeletalMeshComponent* Component = GetSkelMeshComponent();

	//Get our default bodystate skeleton
	MappedBoneData.BodyStateSkeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);

	//Try to auto-detect our bones
	if (bAutoDetectBoneMapAtInit)
	{
		IndexedBoneMap = AutoDetectHandIndexedBones(Component, AutoMapHandTarget);
		auto OldMap = MappedBoneData.BoneMap;
		MappedBoneData.BoneMap = ToBoneReferenceMap(IndexedBoneMap);

		//Reset specified keys from defaults
		for (auto Pair : OldMap)
		{
			Pair.Value.Initialize(CurrentSkeleton);
			MappedBoneData.BoneMap.Add(Pair.Key, Pair.Value);
		}
	}

	//Cache the results
	MappedBoneData.SyncCachedList();
}

void UBodyStateAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	//SN: may want to optimize this at some pt
	if (MappedBoneData.BodyStateSkeleton == nullptr)
	{
		MappedBoneData.BodyStateSkeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);

		//Cache the results
		MappedBoneData.SyncCachedList();
	}
}

void FMappedBoneAnimData::SyncCachedList()
{
	//Clear our current list
	CachedBoneList.Empty();

	//We require a bodystate skeleton to do the mapping
	if (BodyStateSkeleton == nullptr)
	{
		return;
	}

	//Todo: optimize multiple calls with no / small changes

	//1) traverse indexed bone list, store all the traverse lengths

	for (auto Pair : BoneMap)
	{
		CachedBoneLink TraverseResult;

		TraverseResult.MeshBone = Pair.Value;
		TraverseResult.BSBone = BodyStateSkeleton->BoneForEnum(Pair.Key);

		//Costly function and we don't need it after all, and it won't work anymore now that it depends on external data
		//TraverseResult.TraverseCount = TraverseLengthForIndex(TraverseResult.MeshBone.BoneIndex);

		CachedBoneList.Add(TraverseResult);
	}

	//2) reorder according to shortest traverse list
	CachedBoneList.Sort([](const CachedBoneLink& One, const CachedBoneLink& Two) {
		//return One.TraverseCount < Two.TraverseCount;
		return One.MeshBone.BoneIndex < Two.MeshBone.BoneIndex;
	});

	UE_LOG(LogTemp, Log, TEXT("Bone cache synced: %d"), CachedBoneList.Num());
}

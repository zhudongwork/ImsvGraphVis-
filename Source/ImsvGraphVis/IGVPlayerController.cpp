// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVPlayerController.h"

#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "GenericPlatformFile.h"
#include "PlatformFilemanager.h"
#include "Engine.h"
#include "IGVData.h"
#include "IGVGraphActor.h"
#include "IGVLog.h"

//UMaterialInterface* GetOutlineMaterial()
//{
//	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(
//		TEXT("/Game/Materials/PP_Outline_EdgeDetect.PP_Outline_EdgeDetect"));
//	return MaterialAsset.Succeeded() ? MaterialAsset.Object->GetMaterial() : nullptr;
//}

AIGVPlayerController::AIGVPlayerController()
{
	GraphActor = nullptr;
	LeapComponet = CreateDefaultSubobject<ULeapComponent>(TEXT("MyLeap"));
	gestureID = 0;
	handCount = 0;
	haveRecognizeOneGesture = false;
	haveActed = false;
}

void AIGVPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(TEXT("OpenFile"), IE_Released, this,
							   &AIGVPlayerController::IGV_OpenFile);
}
void AIGVPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	/*FString leapLabel = FString::Printf(TEXT("WO_QUAN"));
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, leapLabel);*/
	
	if(!haveRecognizeOneGesture) gestureID = OnJudge();
	OnJudge();
	if (haveRecognizeOneGesture && gestureID == 1 && !haveActed)
	{
		IGV_LOG_S(Warning, TEXT("enter"));
		
		if (fabs(LeftHandPosition.Z - RightHandPosition.Z) <= 10)
		{
			float temp[3];
			int maxMoveDir = 0;
			float moveDist=0;
			for (int i = 0; i < 3; i++)
			{
				temp[i] = RightHandPosition[i] - LastRightHandPosition[i];
				if (fabs(temp[i]) > moveDist)
				{
					maxMoveDir = i;
					moveDist = fabs(temp[i]);
				}
			}
			IGV_LOG_S(Warning, TEXT("moveDist£º%.2f    maxMoveDir: %d"), moveDist, maxMoveDir);
			if (maxMoveDir == 0 && moveDist>3)
			{
				IGV_LOG_S(Warning, TEXT("IGV_SetR"));
				float r = GraphActor->GetSphereRadius();
				if (temp[maxMoveDir] > 0 && r<1200.f)
				{
					r += 100.f;
					IGV_SetR(r);
					haveActed = true;
				}
				else if (temp[maxMoveDir] < 0 && r > 200.f)
				{
					r -= 100.f;
					IGV_SetR(r);
					haveActed = true;
				}
			}
			else if (maxMoveDir == 2 && moveDist > 2)
			{
				IGV_LOG_S(Warning, TEXT("IGV_SetTreemapNesting"));
				float pnest = GraphActor->GetTreemapNesting();
				if (temp[maxMoveDir] > 0 && pnest <= 0.4)
				{
					pnest += 0.09;
					IGV_SetTreemapNesting(pnest);
					haveActed = true;
				}
				else if (temp[maxMoveDir] > 0 && pnest >= 0.2)
				{
					pnest -= 0.09;
					IGV_SetTreemapNesting(pnest);
					haveActed = true;
				}
			}
			else if (maxMoveDir == 1 )
			{
				IGV_LOG_S(Warning, TEXT("IGV_SetFieldOfView"));
				float distenceY= fabs(LeftHandPosition.Y - RightHandPosition.Y);
				float value = GraphActor->GetFieldOfView();
				if (distenceY - lastDistence >= 10)
				{
					IGV_LOG_S(Warning, TEXT("Add"));
					value += 30.f;
					IGV_SetFieldOfView(value);
					haveActed = true;
				}
				else if (distenceY - lastDistence <= -10)
				{
					IGV_LOG_S(Warning, TEXT("Sub"));
					value -= 30.f;
					IGV_SetFieldOfView(value);

					haveActed = true;
				}
				lastDistence = distenceY;
			}
		}
		else if (fabs(LeftHandPosition.Z - RightHandPosition.Z) >= 20)
		{
			FVector2D LHand;
			FVector2D RHand;
			LHand.X = LeftHandPosition.Y;
			LHand.Y = LeftHandPosition.Z;
			RHand.X = RightHandPosition.Y;
			RHand.Y = RightHandPosition.Z;
			float distence = FVector2D::Distance(LHand, RHand);
			float value = GraphActor->GetAspectRatio();
			if (distence - diagonalDistence >= 10.f && value<21.0/9.0)
			{
				value += 1.f / 9.f;
				IGV_SetAspectRatio(value);
				haveActed = true;
			}
			else if (distence - diagonalDistence <= -10.f && value>1.0)
			{
				value -= 1.f / 9.f;
				IGV_SetAspectRatio(value);
				haveActed = true;
			}
		}
		LastLeftHandPosition = LeftHandPosition;
		LastRightHandPosition = RightHandPosition;
	}
}
void AIGVPlayerController::IGV_OpenFile()
{
	if (GraphActor == nullptr)
	{
		IGV_LOG_S(Warning, TEXT("Unable to find GraphActor instance"));
		return;
	}

	UIGVData::OpenFile(GraphActor);
}

void AIGVPlayerController::IGV_SetFieldOfView(float Value)
{
	if (GraphActor == nullptr)
	{
		IGV_LOG_S(Warning, TEXT("Unable to find GraphActor instance"));
		return;
	}

	if (Value < 60)
	{
		IGV_LOG_S(Warning, TEXT("Input value is too small. Minimum FOV should be 60 deg."));
		Value = 60;
	}

	/*if (Value > 180)
	{
		IGV_LOG_S(Warning, TEXT("Input value is too large. Maximum FOV should be 180 deg."));
		Value = 180;
	}*/

	GraphActor->FieldOfView = Value;
	GraphActor->UpdateTreemapLayout();
}

void AIGVPlayerController::IGV_SetAspectRatio(float Value)
{
	if (GraphActor == nullptr)
	{
		IGV_LOG_S(Warning, TEXT("Unable to find GraphActor instance"));
		return;
	}

	if (Value < 1)
	{
		IGV_LOG_S(Warning, TEXT("Input value is too small. Minimum aspect ratio should be 1."));
		Value = 1;
	}

	if (Value > 21.0 / 9.0)
	{
		IGV_LOG_S(Warning,
				  TEXT("Input value is too large. Maximum aspect ratio should be 21.0/9.0"));
		Value = 21.0 / 9.0;
	}

	GraphActor->AspectRatio = Value;
	GraphActor->UpdateTreemapLayout();
}

void AIGVPlayerController::IGV_SetTreemapNesting(float Value)
{
	if (GraphActor == nullptr)
	{
		IGV_LOG_S(Warning, TEXT("Unable to find GraphActor instance"));
		return;
	}

	if (Value < 0)
	{
		IGV_LOG_S(Warning, TEXT("Input value is too small. Minimum aspect ratio is 0."));
		Value = 0;
	}

	if (Value > .5)
	{
		IGV_LOG_S(Warning, TEXT("Input value is too large. Maximum aspect ratio should be 0.5"));
		Value = .5;
	}

	GraphActor->TreemapNesting = Value; 
	GraphActor->UpdateTreemapLayout();
}

void AIGVPlayerController::IGV_SetHalo(bool const Value)
{
	if (GraphActor == nullptr)
	{
		IGV_LOG_S(Warning, TEXT("Unable to find GraphActor instance"));
		return;
	}
	
	GraphActor->SetHalo(Value);
}

void AIGVPlayerController::IGV_SetR(float Value)
{
	float value = GraphActor->GetSphereRadius();
	GraphActor->SetSphereRadius(Value);
	//GraphActor->SetupGraph();
	//GraphActor->SetupNodes();
	//GraphActor->SetupEdges();
	//GraphActor->SetupClusters();
	//GraphActor->UpdateColors();
	GraphActor->UpdateTreemapLayout();
	
	//GraphActor->SetupEdgeMeshes();
	//GraphActor->ResetAmbientOcclusion();
	//GraphActor->BeginPlay()
	//GetOutlineMaterial();
}

int AIGVPlayerController::OnJudge()
{
	FLeapFrameData frame;
	//ULeapComponent *LeapComponet = CreateDefaultSubobject<ULeapComponent>(TEXT("MyLeap"));
	LeapComponet->GetLatestFrameData(frame);
	TArray<FLeapHandData> hands = frame.Hands;
	GetWorld()->GetPawnIterator()->Get()->GetName();
	handCount = hands.Num();
	
	for (int i = 0; i < handCount; i++)
	{
		if (handCount == 2)
		{
			RightHandPosition = hands[1].Palm.Position;
		}
		LeftHandPosition = hands[0].Palm.Position;
		const FLeapHandData hand = hands[i];
		if (hand.GrabStrength == 0)
		{
			haveRecognizeOneGesture = false;
			haveActed = false;
			lastDistence = fabs(LeftHandPosition.Y - RightHandPosition.Y);
			FVector2D LHand;
			FVector2D RHand;
			LHand.X = LeftHandPosition.Y;
			LHand.Y = LeftHandPosition.Z;
			RHand.X = RightHandPosition.Y;
			RHand.Y = RightHandPosition.Z;
			float diagonalDistence = FVector2D::Distance(LHand, RHand);
			LastLeftHandPosition = LeftHandPosition;
			LastRightHandPosition = RightHandPosition;
			//IGV_LOG_S(Warning, TEXT("extern"));
			//IGV_LOG_S(Warning, TEXT("Velocity:%.2f  %.2f  %.2f  "), hand.Palm.Position.X,hand.Palm.Position.Y,hand.Palm.Position.Z);
			return 0;
		}
		else if (!haveRecognizeOneGesture && handCount == 2)
		{
			if (hands[0].GrabStrength == 1 && hands[1].GrabStrength == 1)
			{
				haveRecognizeOneGesture = true;
				IGV_LOG_S(Warning, TEXT("gesture1"));
				return 1;
				
			}
		}
	}
	
	//for (int i = 0; i < hands.Num(); i++)
	//{
	//	const FLeapHandData hand = hands[i];
	//	/*if (hand.HandType.GetValue() == LEAP_HAND_RIGHT)
	//	{
	//		FString RightHandLabel = FString::Printf(TEXT("RightHand"));
	//		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, RightHandLabel);
	//	}*/
	//	float handGrabStrength = hand.GrabStrength;
	//	FString handGrabStrengthLabel = FString::Printf(TEXT("handGrabStrength:  %u"), handGrabStrength);
	//	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, handGrabStrengthLabel);
	//	const TArray<FLeapDigitData> fingers = hand.Digits;
	//	FString fingersLabel = FString::Printf(TEXT("fingers.Num:  %u"), fingers.Num());
	//	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, fingersLabel);
	//	for (int j = 0; j < fingers.Num(); j++)
	//	{
	//		const FLeapDigitData finger = fingers[j];
	//		if (finger.IsExtended)
	//		{
	//			FString fingerIdLabel = FString::Printf(TEXT("finger.Id:  %u"), finger.FingerId);
	//			GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, fingerIdLabel);
	//		}
	//	}
	//}
	/*if (handCount == 0)
	{
		FString leapLabel = FString::Printf(TEXT("0"));
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, leapLabel);
	}
	else if (handCount == 1)
	{
		FString leapLabel = FString::Printf(TEXT("1"));
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, leapLabel);
	}
	else
	{
		FString leapLabel = FString::Printf(TEXT("2"));
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, leapLabel);
	}*/
	return 0;
}


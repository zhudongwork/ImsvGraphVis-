// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVPlayerController.h"

#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "GenericPlatformFile.h"
#include "PlatformFilemanager.h"
#include "Engine.h"
#include "IGVData.h"
#include "IGVGraphActor.h"
#include "IGVLog.h"

AIGVPlayerController::AIGVPlayerController()
{
	GraphActor = nullptr;
	LeapComponet = CreateDefaultSubobject<ULeapComponent>(TEXT("MyLeap"));
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
	OnJudge();
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

	if (Value > 180)
	{
		IGV_LOG_S(Warning, TEXT("Input value is too large. Maximum FOV should be 180 deg."));
		Value = 180;
	}

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
void AIGVPlayerController::OnJudge()
{
	FLeapFrameData frame;
	//ULeapComponent *LeapComponet = CreateDefaultSubobject<ULeapComponent>(TEXT("MyLeap"));
	LeapComponet->GetLatestFrameData(frame);
	
	const TArray<FLeapHandData> hands = frame.Hands;
	
	GetWorld()->GetPawnIterator()->Get()->GetName();
	handCount = hands.Num();
	for (int i = 0; i < handCount; i++)
	{
		const FLeapHandData hand = hands[i];
		if (hand.GrabStrength == 0)
		{
			haveRecognizeOneGesture = false;
			haveActed = false;
		}
		else if (!haveRecognizeOneGesture && handCount == 2)
		{
			
			IGV_LOG_S(Warning, TEXT("enter"));
			if (hands[0].GrabStrength == 1 && hands[1].GrabStrength == 1)
			{
				IGV_LOG_S(Warning, TEXT("recognize"));
				FVector LeftHandPosition = hands[0].Palm.Position;
				FVector RightHandPosition = hands[1].Palm.Position;

				if (fabs(LeftHandPosition.Z - RightHandPosition.Z) <= 10)//Parallel gesture: change the treemap nesting
				{
					IGV_LOG_S(Warning, TEXT("%.2f"), fabs(LeftHandPosition.Y - RightHandPosition.Y));
					float distence = fabs(LeftHandPosition.Y - RightHandPosition.Y);
					if (!haveRecognizeOneGesture) lastDistence = distence;
					if (!haveActed)
					{
						float value = GraphActor->GetTreemapNesting();
						if (distence - lastDistence >= 10.f)
						{
							IGV_LOG_S(Warning, TEXT("Add"));
							value += 0.1;
							IGV_SetTreemapNesting(value);
							haveActed = true;
						}
						else if (distence - lastDistence <= -10.f)
						{
							IGV_LOG_S(Warning, TEXT("Sub"));
							value -= 0.1;
							IGV_SetTreemapNesting(value);
							
							haveActed = true;
						}
					}
				}
				else if (fabs(LeftHandPosition.Z - RightHandPosition.Z) >= 20.f)//diagonal gesture: change the aspectratio
				{
					FVector2D LHand;
					FVector2D RHand;
					LHand.X = LeftHandPosition.Y;
					LHand.Y = LeftHandPosition.Z;
					RHand.X = RightHandPosition.Y;
					RHand.Y = RightHandPosition.Z;
					float distence = FVector2D::Distance(LHand, RHand);
					if (!haveRecognizeOneGesture) diagonalDistence = distence;
					if (!haveActed)
					{
						float value = GraphActor->GetAspectRatio();
						if (distence - diagonalDistence >= 10.f)
						{
							value += 1.f / 9.f;
							IGV_SetAspectRatio(value);
							haveActed = true;
						}
						else if (distence - diagonalDistence <= -10.f)
						{
							value -= 1.f / 9.f;
							IGV_SetAspectRatio(value);
							haveActed = true;
						}
					}
				}
				haveRecognizeOneGesture = true;
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
}


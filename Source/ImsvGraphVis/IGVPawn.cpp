// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "IGVPawn.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Controller.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"

#include "IGVGraphActor.h"
#include "IGVLog.h"

AIGVPawn::AIGVPawn() : CursorDistanceScale(0.4)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	haveSetLastHandPosition = false;
	BaseEyeHeight = 0.0f;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->AttachToComponent(RootComponent,
									   FAttachmentTransformRules::KeepRelativeTransform);
	// Disable positional tracking
	CameraComponent->bUsePawnControlRotation = true;
	CameraComponent->bLockToHmd = false;
	LeapComponet = CreateDefaultSubobject<ULeapComponent>(TEXT("MyPawnLeap"));
	
	CursorMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cursor"));
	CursorMeshComponent->AttachToComponent(CameraComponent,
										   FAttachmentTransformRules::KeepRelativeTransform);
	CursorMeshComponent->SetRelativeLocation(FVector(400, 0, 0));

	CursorDirectionIndicatorMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CursorDirectionIndicator"));
	CursorDirectionIndicatorMeshComponent->AttachToComponent(
		CameraComponent, FAttachmentTransformRules::KeepRelativeTransform);
	CursorDirectionIndicatorMeshComponent->SetRelativeLocation(FVector(200, 0, 0));
	CursorDirectionIndicatorMeshComponent->SetVisibility(false);

	HelpTextRenderComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HelpText"));
	HelpTextRenderComponent->AttachToComponent(CameraComponent,
											   FAttachmentTransformRules::KeepRelativeTransform);
	HelpTextRenderComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	HelpTextRenderComponent->VerticalAlignment = EVerticalTextAligment::EVRTA_TextCenter;
	HelpTextRenderComponent->SetRelativeLocation(FVector(500, 0, 0));
	HelpTextRenderComponent->SetRelativeRotation((-FVector::ForwardVector).Rotation());
	HelpTextRenderComponent->SetVisibility(false);
}

void AIGVPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AIGVPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	APawn *leapPawn = GetWorld()->GetPawnIterator()->Get();
	fingerTipRotation = leapPawn->GetActorRotation();
	fingerTipPosition = leapPawn->GetActorLocation();
	/*FString nameLabel = GetWorld()->GetPawnIterator()->Get()->GetName();
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue, nameLabel);*/

	FLeapFrameData frame;
	//ULeapComponent *LeapComponet = CreateDefaultSubobject<ULeapComponent>(TEXT("MyLeap"));
	LeapComponet->GetLatestFrameData(frame);
	const TArray<FLeapHandData> hands = frame.Hands;
	for (int i = 0; i < hands.Num(); i++)
	{

		const FLeapHandData hand = hands[i];
		if (hand.HandType.GetValue() == LEAP_HAND_RIGHT && hands.Num()==1 )
		{
			const TArray<FLeapDigitData> fingers = hand.Digits;
			if (fingers[1].IsExtended && !fingers[0].IsExtended && !fingers[2].IsExtended && !fingers[3].IsExtended && !fingers[4].IsExtended)
			{
				FLeapPalmData palm = hand.Palm;
				rightHandPosition = palm.Position;
				//IGV_LOG_S(Warning, TEXT("X: %.2f  Y: %.2f  Z: %.2f"), rightHandPosition.X,rightHandPosition.Y,rightHandPosition.Z);
				if (!haveSetLastHandPosition)
				{
					LastRightHandPosition = rightHandPosition;
					haveSetLastHandPosition = true;
				}
				float tempX = rightHandPosition.X - LastRightHandPosition.X;
				float tempY = rightHandPosition.Y - LastRightHandPosition.Y;

				/*FString tempXLabel = FString::Printf(TEXT("tempX:  %u"), tempX);
				GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, tempXLabel);
				FString tempYLabel = FString::Printf(TEXT("tempY:  %u"), tempY);
				GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, tempYLabel);*/

				AddControllerYawInput(tempY);
				AddControllerPitchInput(-1.0*tempX);
				
				if (hand.GrabStrength> 0.3)
				{
					OnLeftMouseButtonReleased();
				}
			}
			//if(fingers[1].IsExtended) IGV_LOG_S(Warning, TEXT("fingers[1].IsExtended"));
			//IGV_LOG_S(Warning, TEXT("hand.GrabStrength: %.2f"), hand.GrabStrength);
			/*if (fingers[1].IsExtended && hand.GrabStrength > 0.5)
			{
				IGV_LOG_S(Warning, TEXT("66666666666"));
				continue;
			}*/
			
		}
		else
		{

		}
		LastRightHandPosition = rightHandPosition;
		/*float handGrabStrength = hand.GrabStrength;
		FString handGrabStrengthLabel = FString::Printf(TEXT("handGrabStrength:  %u"), handGrabStrength);
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, handGrabStrengthLabel);
		const TArray<FLeapDigitData> fingers = hand.Digits;
		FString fingersLabel = FString::Printf(TEXT("fingers.Num:  %u"), fingers.Num());
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, fingersLabel);
		for (int j = 0; j < fingers.Num(); j++)
		{
		const FLeapDigitData finger = fingers[j];
		if (finger.IsExtended)
		{
		FString fingerIdLabel = FString::Printf(TEXT("finger.Id:  %u"), finger.FingerId);
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, fingerIdLabel);
		}
		}*/
	}
	UpdateCursor();
}

void AIGVPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	/*InputComponent->BindAxis(TEXT("MouseX"), this, &AIGVPawn::AddControllerYawInput);
	InputComponent->BindAxis(TEXT("MouseY"), this, &AIGVPawn::AddControllerPitchInput);

	InputComponent->BindAction(TEXT("LeftMouseButton"), IE_Released, this,
							   &AIGVPawn::OnLeftMouseButtonReleased);*/
	/*FString RightHandLabel = FString::Printf(TEXT("RightHand"));
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, RightHandLabel);*/
	
}

void AIGVPawn::AddControllerYawInput(float Value)
{
	APlayerController* const PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController)
	{
		PickRayRotation.Yaw += Value * PlayerController->InputYawScale;
	}
}

void AIGVPawn::AddControllerPitchInput(float Value)
{
	APlayerController* const PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController)
	{
		PickRayRotation.Pitch += -Value * PlayerController->InputPitchScale;
	}
}

void AIGVPawn::UpdateCursor()
{
	APlayerController* const PlayerController = Cast<APlayerController>(Controller);

	PickRayOrigin = GetActorLocation();
	PickRayDirection = PickRayRotation.Vector().GetSafeNormal();

	FVector const CursorWorldPosition =
		PickRayOrigin + CursorDistanceScale * GraphActor->GetSphereRadius() * PickRayDirection;

	//CursorMeshComponent->SetWorldLocation(CursorWorldPosition);
	//CursorMeshComponent->SetWorldRotation(PickRayRotation);
	CursorMeshComponent->SetWorldLocation(fingerTipPosition);
	CursorMeshComponent->SetWorldRotation(fingerTipRotation);
	if ((GetViewRotation().Vector() | PickRayDirection) < 0.76604444311 /* cos 40 deg */)
	{
		CursorDirectionIndicatorMeshComponent->SetVisibility(true);
		FVector const RelativeDirection = CameraComponent->GetComponentTransform()
											  .InverseTransformVectorNoScale(PickRayDirection)
											  .GetSafeNormal();
		FVector const TargetDirection =
			FVector(0, RelativeDirection.Y, RelativeDirection.Z).GetSafeNormal();
		CursorDirectionIndicatorMeshComponent->SetRelativeLocation(FVector(200, 0, 0) +
																   TargetDirection * 85);
		CursorDirectionIndicatorMeshComponent->SetRelativeRotation(TargetDirection.Rotation());
	}
	else
	{
		CursorDirectionIndicatorMeshComponent->SetVisibility(false);
	}
}

void AIGVPawn::OnLeftMouseButtonReleased()
{
	if (GraphActor)
	{
		GraphActor->OnLeftMouseButtonReleased();
	}
}




#include "Player/AuraPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Input/AuraInputComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SplineComponent.h"
#include "AuraGameplayTags.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
	AutoRun();
}

void AAuraPlayerController::AutoRun()
{
	if(!bAutoRunnig)
		return;

	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination < AutoRunAcceptanceRadius)
		{
			bAutoRunnig = false;
		}
	}
}


UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}

	return AuraAbilitySystemComponent;
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(AuraContext); // assert if AuraContext is not set

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}
	
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent); // if cast fails, it will assert

	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);

	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D Axis = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);	

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, Axis.Y);
		ControlledPawn->AddMovementInput(RightDirection, Axis.X);
	}
}

void AAuraPlayerController::CursorTrace()
{
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	if (!CursorHit.bBlockingHit)
		return; 

	LastActor = ThisActor;
	ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());

	if (LastActor != ThisActor)
	{
		if (LastActor)
			LastActor->UnhighlightActor();

		if (ThisActor)
			ThisActor->HighlightActor();
	}

#pragma region origin
	///**
	//* Line trace from cursor. There are several scenarios:
	//*	A. LastActor is nullptr and ThisActor is nullptr
	//*		- Do nothing
	//*	B. LastActor is nullptr and ThisActor is valid
	//*		- Highlight ThisActor
	//*   C. LastActor is valid and ThisActor is nullptr
	//*       - Unhighlight LastActor
	//*   D. LastActor is valid and ThisActor is valid, but LastActor != ThisActor
	//*       - Unhighlight LastActor, Highlight ThisActor
	//*   E. LastActor is valid and ThisActor is valid, and LastActor == ThisActor
	//*       - Do nothing
	//*/

	//if (LastActor == nullptr)
	//{
	//	if (ThisActor != nullptr)
	//	{
	//		// Case B
	//		ThisActor->HighlightActor();
	//	}
	//	else
	//	{
	//		// Case A
	//		// Do nothing
	//	}
	//}
	//else
	//{
	//	if (ThisActor == nullptr)
	//	{
	//		// Case C
	//		LastActor->UnhighlightActor();
	//	}
	//	else
	//	{
	//		if (LastActor != ThisActor)
	//		{
	//			// Case D
	//			LastActor->UnhighlightActor();
	//			ThisActor->HighlightActor();
	//		}
	//		else
	//		{
	//			// Case E
	//			// Do nothing
	//		}
	//	}
	//}
#pragma endregion
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("AbilityInputTagPressed: %s"), *InputTag.ToString()));

	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		bTargeting  = ThisActor != nullptr ? true : false;
		bAutoRunnig = false;
	}

}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC())
			GetASC()->AbilityInputTagReleased(InputTag);

		return;
	}

	if (bTargeting)
	{
		if (GetASC())
			GetASC()->AbilityInputTagReleased(InputTag);
	}
	else
	{
		const APawn* ControlledPawn = GetPawn<APawn>();

		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);

				CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1]; 
				bAutoRunnig = true;
			}
		}

		FollowTime = 0.0f;
		bTargeting = false;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC())
			GetASC()->AbilityInputTagHeld(InputTag);

		return;
	}

	if (bTargeting)
	{
		if (GetASC())
			GetASC()->AbilityInputTagHeld(InputTag);
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();

		if (CursorHit.bBlockingHit)
			CachedDestination = CursorHit.ImpactPoint;

		if (APawn* ControlledPawn = GetPawn<APawn>())
		{
			if (FollowTime > ShortPressThreshold)
			{
				const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();

				ControlledPawn->AddMovementInput(WorldDirection);
			}
		}
	}
}

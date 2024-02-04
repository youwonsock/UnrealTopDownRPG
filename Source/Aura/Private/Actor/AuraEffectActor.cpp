


#include "Actor/AuraEffectActor.h"

#include "GameFramework/Character.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot")));
}

void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	//IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Target);

	//if (ASCInterface)
	//{
	//	ASCInterface->GetAbilitySystemComponent();

	//}

	// UAbilitySystemBlueprintLibrary : GAS에서 제공하는 각종 편의 함수가 포함된 라이브러리
	// 위에서 하는 것과 같은 기능을 하는 함수
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr)
		return;

	// Get Effect Spec
	check(GameplayEffectClass);
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext(); // meke effect context()
	EffectContextHandle.AddSourceObject(this); // add source object
    const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);

	FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*(EffectSpecHandle.Data.Get()) );

	// 런타임에 DurationPolicy를 변경하기 위한것이 아니라 확인용으로 사용
	const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if (bIsInfinite)
	{
		ActiveEffectHandles.Add(ActiveEffectHandle, GameplayEffectClass);
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (!InstantGameplayEffects.IsEmpty())
	{
		for (auto const EffectPars : InstantGameplayEffects)
		{
			if (EffectPars.Value == EEffectApplicationPolicy::ApplyOnOverlap)
			{
				ApplyEffectToTarget(TargetActor, EffectPars.Key);
			}
		}
	}

	if (!DurationGameplayEffects.IsEmpty())
	{
		for (auto const EffectPars : DurationGameplayEffects)
		{
			if (EffectPars.Value == EEffectApplicationPolicy::ApplyOnOverlap)
			{
				ApplyEffectToTarget(TargetActor, EffectPars.Key);
			}
		}
	}

	if (!InfiniteGameplayEffects.IsEmpty())
	{
		for (auto const EffectPars : InfiniteGameplayEffects)
		{
			if (EffectPars.Value.ApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
			{
				ApplyEffectToTarget(TargetActor, EffectPars.Key);
			}
		}
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (!InstantGameplayEffects.IsEmpty())
	{
		for (auto const EffectPars : InstantGameplayEffects)
		{
			if (EffectPars.Value == EEffectApplicationPolicy::ApplyOnEndOverlap)
			{
				ApplyEffectToTarget(TargetActor, EffectPars.Key);
			}
		}
	}

	if (!DurationGameplayEffects.IsEmpty())
	{
		for (auto const EffectPars : DurationGameplayEffects)
		{
			if (EffectPars.Value == EEffectApplicationPolicy::ApplyOnEndOverlap)
			{
				ApplyEffectToTarget(TargetActor, EffectPars.Key);
			}
		}
	}

	if (!InfiniteGameplayEffects.IsEmpty())
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!TargetASC) return;

		TArray<FActiveGameplayEffectHandle> HandlesToRemove;

		for (auto const EffectPars : InfiniteGameplayEffects)
		{
			if (EffectPars.Value.RemovePolicy == EEffectRemovePolicy::RemoveOnEndOverlap)
			{
				for (auto HandlePair : ActiveEffectHandles)
				{
					if (HandlePair.Key.GetOwningAbilitySystemComponent() == TargetASC && HandlePair.Value == EffectPars.Key)
					{
						TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);
						HandlesToRemove.Add(HandlePair.Key);
					}
				}
			}
		}

		for (auto& Handle : HandlesToRemove)
		{
			ActiveEffectHandles.FindAndRemoveChecked(Handle);
		}
	}
}





#include "Actor/AuraEffectActor.h"

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

void AAuraEffectActor::ApplyEffectToTarget(AActor* Target, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	//IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Target);

	//if (ASCInterface)
	//{
	//	ASCInterface->GetAbilitySystemComponent();

	//}

	// UAbilitySystemBlueprintLibrary : GAS에서 제공하는 각종 편의 함수가 포함된 라이브러리
	// 위에서 하는 것과 같은 기능을 하는 함수
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (TargetASC == nullptr)
		return;

	// Get Effect Spec
	check(GameplayEffectClass);
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext(); // meke effect context()
	EffectContextHandle.AddSourceObject(this); // add source object
    const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, 1.0f, EffectContextHandle);

	TargetASC->ApplyGameplayEffectSpecToSelf( *(EffectSpecHandle.Data.Get()) );
}


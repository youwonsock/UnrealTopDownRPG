
#pragma once

#include "GameplayEffectTypes.h"
#include "AuraAbilityType.generated.h"

USTRUCT(BlueprintType)
struct FAuraGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:

	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	bool IsBlockedHit() const { return bIsBlockedHit; }
	void SetBlockedHit(bool bBlockedHit) { bIsBlockedHit = bBlockedHit; }	

	bool IsCriticalHit() const { return bIsCriticalHit; }
	void SetCriticalHit(bool bCriticalHit) { bIsCriticalHit = bCriticalHit; }

	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }

	virtual FAuraGameplayEffectContext* Duplicate() const
	{
		FAuraGameplayEffectContext* NewContext = new FAuraGameplayEffectContext();

		*NewContext = *this;

		if (GetHitResult() != nullptr)
		{
			NewContext->AddHitResult(*GetHitResult(), true);
		}

		return NewContext;
	}

protected:
	UPROPERTY()
	bool bIsBlockedHit = false;

	UPROPERTY()
	bool bIsCriticalHit = false;
};

template<>
struct TStructOpsTypeTraits<FAuraGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FAuraGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
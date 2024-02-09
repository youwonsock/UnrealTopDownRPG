

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"

#include "AuraCharacter.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase
{
	GENERATED_BODY()

public:
	AAuraCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// ICombatInterface을(를) 통해 상속됨
	virtual int32 GetPlayerLevel() override;

protected:
	virtual void InitAbilityActorInfo() override;

};

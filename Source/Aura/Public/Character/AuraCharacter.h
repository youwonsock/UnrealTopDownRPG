

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

	// ICombatInterface��(��) ���� ��ӵ�
	virtual int32 GetPlayerLevel() override;

protected:
	virtual void InitAbilityActorInfo() override;

};

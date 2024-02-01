

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/EnemyInterface.h"

#include "AuraEnemy.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()
	
public:

	AAuraEnemy();

	// IEnemyInterface을(를) 통해 상속됨
	void HighlightActor() override;
	void UnhighlightActor() override;

protected:
	virtual void BeginPlay() override;
};

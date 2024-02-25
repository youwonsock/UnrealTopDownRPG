


#include "AbilitySystem/AuraAbilitySystemGlobals.h"
#include "AuraAbilityType.h"

FGameplayEffectContext* UAuraAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FAuraGameplayEffectContext();
}

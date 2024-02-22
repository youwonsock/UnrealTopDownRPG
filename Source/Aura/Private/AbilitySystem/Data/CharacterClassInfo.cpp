

#include "AbilitySystem/Data/CharacterClassInfo.h"

FCharacterClassDefaultInfo UCharacterClassInfo::GetClassDefaults(ECharacterClass CharacterClass) const
{
	return CharacterClassInformation.FindChecked(CharacterClass);
}

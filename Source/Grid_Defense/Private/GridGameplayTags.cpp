

#include "GridGameplayTags.h"

FGridGameplayTags FGridGameplayTags::GameplayTags;

void FGridGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.State_Slow = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("State.Slow"), FString("Slow"));
	GameplayTags.State_Stun = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("State.Stun"), FString("Stun"));

	GameplayTags.Enemy_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Enemy.Fire"), FString("Fire"));
	GameplayTags.Enemy_Nature = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Enemy.Nature"), FString("Nature"));
	GameplayTags.Enemy_Water = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Enemy.Water"), FString("Water"));

	GameplayTags.Damage_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Fire"), FString("Fire"));
	GameplayTags.Damage_Ice = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Ice"), FString("Ice"));
	GameplayTags.Damage_Lightning = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Damage.Lightning"), FString("Lightning"));
}

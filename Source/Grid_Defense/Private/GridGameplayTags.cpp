

#include "GridGameplayTags.h"

FGridGameplayTags FGridGameplayTags::GameplayTags;

void FGridGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.State_Slow = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("State.Slow"), FString("Slow"));
}

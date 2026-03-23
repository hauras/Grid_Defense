
#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

struct FGridGameplayTags
{
	
public:
	static const FGridGameplayTags& Get() { return GameplayTags;}
	static void InitializeNativeGameplayTags();

	FGameplayTag State_Slow;

private:
	static FGridGameplayTags GameplayTags;
	
};

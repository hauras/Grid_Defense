
#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

struct FGridGameplayTags
{
	
public:
	static const FGridGameplayTags& Get() { return GameplayTags;}
	static void InitializeNativeGameplayTags();

	FGameplayTag State_Slow;

	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Lightning;
	FGameplayTag Damage_Ice;

	FGameplayTag Enemy_Nature;
	FGameplayTag Enemy_Fire;
	FGameplayTag Enemy_Water;

	FGameplayTag State_Stun;

	FGameplayTag Tower_All;
	FGameplayTag Tower_Basic;
	FGameplayTag Tower_Ice;
	FGameplayTag Tower_Splash;
	FGameplayTag Tower_Lightning;
	
private:
	static FGridGameplayTags GameplayTags;
	
};

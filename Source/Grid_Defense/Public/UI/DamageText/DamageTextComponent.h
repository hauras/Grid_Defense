
#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DamageTextComponent.generated.h"

/**
 * 
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class GRID_DEFENSE_API UDamageTextComponent : public UWidgetComponent
{
	GENERATED_BODY()
public:

	void SetDamageText(float Damage, FLinearColor TextColor);
	void HideDamageText();
};

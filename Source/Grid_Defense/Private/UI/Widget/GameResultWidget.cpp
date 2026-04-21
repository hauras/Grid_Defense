// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/GameResultWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
void UGameResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 🌟 위젯이 생성될 때, 버튼에 클릭 이벤트를 달아줍니다.
	if (Btn_Restart)
	{
		Btn_Restart->OnClicked.AddDynamic(this, &UGameResultWidget::OnRestartClicked);
	}
	if (Btn_Quit)
	{
		Btn_Quit->OnClicked.AddDynamic(this, &UGameResultWidget::OnQuitClicked);
	}
}

void UGameResultWidget::OnRestartClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("다시 시작 버튼 클릭됨!"));

	// 1. 현재 열려있는 맵(레벨)의 이름을 가져와서 그대로 다시 엽니다.
	FName CurrentLevelName = FName(*GetWorld()->GetName());
	UGameplayStatics::OpenLevel(GetWorld(), CurrentLevelName);
}

void UGameResultWidget::OnQuitClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("게임 종료 버튼 클릭됨!"));

	// 1. 플레이어 컨트롤러를 가져와서 게임을 완전히 종료합니다.
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, true);
}
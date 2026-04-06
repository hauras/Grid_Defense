

#include "MainMenu/MainMenuWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Start)
	{
		Btn_Start->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartClicked);
	}
	if (Btn_Load)
	{
		Btn_Load->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLoadClicked);
	}
	if (Btn_QuitGame)
	{
		Btn_QuitGame->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}
}

void UMainMenuWidget::OnStartClicked()
{
	UGameplayStatics::OpenLevel(this, FName("TestMap"));

}

void UMainMenuWidget::OnLoadClicked()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("Slot1"), 0))
	{
		// 🌟 [여기가 핵심입니다!] 뒤에 true와 TEXT("LoadGame=True") 를 꼭 붙여주세요!
		UGameplayStatics::OpenLevel(this, FName("TestMap"), true, TEXT("LoadGame=True"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("세이브 파일이 없습니다!"));
	}
}

void UMainMenuWidget::OnQuitClicked()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, true);
	}
	
}

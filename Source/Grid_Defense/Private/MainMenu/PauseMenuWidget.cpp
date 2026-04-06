

#include "MainMenu/PauseMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Save/GridSaveGame.h"
#include "Tower/TowerBase.h"
#include "EngineUtils.h"
#include "GameMode/GridGameMode.h"
#include "Grid/GridManager.h"
#include "Nexus/Nexus.h"
void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Resume) Btn_Resume->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	if (Btn_Save) Btn_Save->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSaveClicked);
	if (Btn_ToMain) Btn_ToMain->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnToMainClicked);
}

void UPauseMenuWidget::OnResumeClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
    
	// 입력 모드를 다시 게임으로 변경하고 마우스 커서 숨기기
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		FInputModeGameOnly InputModeData;
		PC->SetInputMode(InputModeData);
		PC->bShowMouseCursor = false;
	}

	// 위젯 제거
	RemoveFromParent();
}

void UPauseMenuWidget::OnSaveClicked()
{
    UGridSaveGame* SaveInst = Cast<UGridSaveGame>(UGameplayStatics::CreateSaveGameObject(UGridSaveGame::StaticClass()));

    if (SaveInst)
    {
       // 1. 타워 저장 (기존 코드)
       for (TActorIterator<ATowerBase> It(GetWorld()); It; ++It)
       {
          ATowerBase* FoundTower = *It;

          if (FoundTower->IsPreview())
          {
             continue; 
          }

          FTowerSaveData NewData;
            
          NewData.TowerData = FoundTower->GetTowerData(); 
          NewData.GridX = FoundTower->GridX;         
          NewData.GridY = FoundTower->GridY;         

          SaveInst->SavedTowers.Add(NewData);
       }

       // 2. 맵 저장 (기존 코드)
       AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
       if (GridManager)
       {
          SaveInst->SavedMapLayout.Empty(); 
           
          for (const FGridInfo& Tile : GridManager->GetGridArray())
          {
             SaveInst->SavedMapLayout.Add(Tile.TileType);
          }
       }
       
       
       //  3. 게임 수치(골드, 웨이브, 넥서스 체력) 저장 추가!
       // ================================================
       // (1) 게임모드에서 골드와 웨이브 가져오기
       AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
       if (GM)
       {
       		SaveInst->SavedGold = GM->GetCurrentGold();
       }

       ANexus* Nexus = Cast<ANexus>(UGameplayStatics::GetActorOfClass(GetWorld(), ANexus::StaticClass()));
       if (Nexus)
       {
        
       }

       bool bIsSaved = UGameplayStatics::SaveGameToSlot(SaveInst, SaveInst->SaveSlotName, SaveInst->UserIndex);

       if (bIsSaved)
       {
          UE_LOG(LogTemp, Warning, TEXT("저장 성공! 타워 개수: %d / 골드, 웨이브, 맵 정보 저장 완료!"), SaveInst->SavedTowers.Num());
       }
    }
}

void UPauseMenuWidget::OnToMainClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, FName("MainMenu")); // 메인 메뉴 레벨 이름 확인!
}

#include "Camera/Camera.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ACamera::ACamera()
{
    PrimaryActorTick.bCanEverTick = true;

    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 2000.f;
    SpringArm->bDoCollisionTest = false; // 타워/적에 가려져도 카메라 점프 방지
    
    // 쿼터뷰 각도 설정 (약 50도 정도 숙임)
    SpringArm->SetRelativeRotation(FRotator(-50.f, 0.f, 0.f));
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritRoll = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);

    TargetZoom = SpringArm->TargetArmLength;
}

void ACamera::BeginPlay()
{
    Super::BeginPlay();
    
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void ACamera::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetZoom, DeltaTime, 5.0f);

    if (bEnableScroll)
    {
        HandleScroll(DeltaTime);
    }
}

void ACamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACamera::Move);
        EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ACamera::Zoom);
    }
}

void ACamera::Move(const FInputActionValue& Value)
{
    FVector2D MoveVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        FVector NewLocation = GetActorLocation();
        // 쿼터뷰 기준: X축(상하), Y축(좌우) 이동
        NewLocation.X += MoveVector.Y * MoveSpeed * GetWorld()->GetDeltaSeconds();
        NewLocation.Y += MoveVector.X * MoveSpeed * GetWorld()->GetDeltaSeconds();
		
        SetActorLocation(NewLocation);
    }
}

void ACamera::Zoom(const FInputActionValue& Value)
{
    float ZoomValue = Value.Get<float>();
	
    TargetZoom = FMath::Clamp(TargetZoom + (ZoomValue * -ZoomSpeed), MinZoom, MaxZoom);
}

void ACamera::HandleScroll(float DeltaTime)
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    float MouseX, MouseY;
    // 현재 마우스 위치 가져오기
    if (PC->GetMousePosition(MouseX, MouseY))
    {
        int32 ViewportSizeX, ViewportSizeY;
        PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

        FVector ScrollDirection = FVector::ZeroVector;

        // 마우스가 왼쪽 끝에 있을 때 (Y축 마이너스 이동)
        if (MouseX <= Margin)
        {
            ScrollDirection.Y = -1.f;
        }
        // 마우스가 오른쪽 끝에 있을 때 (Y축 플러스 이동)
        else if (MouseX >= ViewportSizeX - Margin)
        {
            ScrollDirection.Y = 1.f;
        }

        // 마우스가 위쪽 끝에 있을 때 (X축 플러스 이동)
        if (MouseY <= Margin)
        {
            ScrollDirection.X = 1.f;
        }
        // 마우스가 아래쪽 끝에 있을 때 (X축 마이너스 이동)
        else if (MouseY >= ViewportSizeY - Margin)
        {
            ScrollDirection.X = -1.f;
        }

        // 이동 방향이 있다면 카메라 위치 업데이트
        if (!ScrollDirection.IsNearlyZero())
        {
            FVector NewLocation = GetActorLocation() + (ScrollDirection.GetSafeNormal() * MoveSpeed * DeltaTime);
            SetActorLocation(NewLocation);
        }
    }
}


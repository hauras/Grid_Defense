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
    SpringArm->bDoCollisionTest = false;
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
}

void ACamera::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // [수정] 값이 같으면 보간 스킵
    if (!FMath::IsNearlyEqual(SpringArm->TargetArmLength, TargetZoom, 1.0f))
    {
        SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetZoom, DeltaTime, 5.0f);
    }

    if (bEnableScroll)
    {
        HandleScroll(DeltaTime);
    }
}

void ACamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACamera::Move);
        }
        
        if (ZoomAction)
        {
            EIC->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ACamera::Zoom);
        }
    }
}

// [수정] 공통 이동 로직 분리
void ACamera::MoveCamera(const FVector& Direction, float DeltaTime)
{
    FVector NewLocation = GetActorLocation() + (Direction * MoveSpeed * DeltaTime);
    SetActorLocation(NewLocation);
}

void ACamera::Move(const FInputActionValue& Value)
{
    FVector2D MoveVector = Value.Get<FVector2D>();
    if (!Controller) return;

    FVector Direction = FVector(MoveVector.Y, MoveVector.X, 0.f);
    MoveCamera(Direction, GetWorld()->GetDeltaSeconds());

    OnCameraMoved.Broadcast();

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
    if (!PC->GetMousePosition(MouseX, MouseY)) return;

    int32 ViewportSizeX, ViewportSizeY;
    PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

    FVector ScrollDirection = FVector::ZeroVector;

    if (MouseX <= Margin)               ScrollDirection.Y = -1.f;
    else if (MouseX >= ViewportSizeX - Margin) ScrollDirection.Y = 1.f;

    if (MouseY <= Margin)               ScrollDirection.X = 1.f;
    else if (MouseY >= ViewportSizeY - Margin) ScrollDirection.X = -1.f;

    if (!ScrollDirection.IsNearlyZero())
    {
        MoveCamera(ScrollDirection.GetSafeNormal(), DeltaTime);
        OnCameraMoved.Broadcast();
    }

}
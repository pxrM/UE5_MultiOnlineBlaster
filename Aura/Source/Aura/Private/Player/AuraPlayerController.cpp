// P


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;

	SplineCmp = CreateDefaultSubobject<USplineComponent>("SplineCmp");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();

	AutoMove();
}

void AAuraPlayerController::CursorTrace()
{
	// GetHitResultUnderCursor这个函数用于在当前光标（鼠标指针）所在的位置执行一次“射线检测”（Raycast）
	// ECC_Visibility 是用于检测的碰撞通道（Collision Channel）。ECC_Visibility 通常用于检测可见性相关的碰撞，这意味着它会忽略不可见的对象。
	// false 表示不忽略复杂碰撞（complex collision）。如果设置为 true，则只检测简单碰撞（simple collision）。
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if(!CursorHit.bBlockingHit) return;

	LastActor = CurrActor;
	CurrActor = Cast<IEnemyInterface>(CursorHit.GetActor());
	
	/*
	 * 从游标开始行跟踪。有几种情况:
	 *	A: LastActor为null && CurrActor为null	-不做任何事情
	 *	B: LastActor是空的&& CurrActor是有效的		-突出显示CurrActor
	 *	C: LastActor是有效的&& CurrActor是空的		-取消突出LastActor
	 *	D: 两个都有效，但是LastActor!=CurrActor	-取消突出LastActor，并突出CurrActor
	 *	E: 两个都有效，而且是同一个Actor			-不做任何事情
	 */
	// if(LastActor == nullptr)
	// {
	// 	if(CurrActor != nullptr)
	// 	{
	// 		CurrActor->HighlightActor();
	// 	}
	// }
	// else
	// {
	// 	if(CurrActor == nullptr)
	// 	{
	// 		LastActor->UnHighlightActor();
	// 	}
	// 	else if(LastActor != CurrActor)
	// 	{
	// 		LastActor->UnHighlightActor();
	// 		CurrActor->HighlightActor();
	// 	}
	// }
	if(LastActor != CurrActor)
	{
		if(LastActor) LastActor->UnHighlightActor();
		if(CurrActor) CurrActor->HighlightActor();
	}
}

void AAuraPlayerController::AutoMove()
{
	if(!bAutoRunning) return;
	
	if(APawn* ControlledPawn = GetPawn())
	{
		// 由于无法确保角色位置和样条曲线百分之百重合在一块，所以找到距离Pawn当前位置最近的样条曲线上的位置
		const FVector LocationOnSpline = SplineCmp->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		// 找到样条曲线在 LocationOnSpline 处的方向，并将其存储在 Direction 中
		const FVector Direction = SplineCmp->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		// 让角色通过此方向去移动
		ControlledPawn->AddMovementInput(Direction);

		// 如果小于设置的AutoRunAcceptanceRadius距离内，停止自动寻路。
		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if(DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(AuraContext);

	/*
	 * 绑定增强输入到玩家Controller
	 */
	// 从当前本地玩家对象中获取一个 用于增强输入管理的本地玩家子系统 类型的子系统
	if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// 向输入子系统中添加一个输入映射上下文
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	/*
	 * 设置鼠标显示
	 */
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	/*
	 * 设置游戏中的输入模式，使得鼠标不会被锁定到视口内，并且在捕获输入时鼠标指针仍然可见
	 */
	FInputModeGameAndUI InputModeData;
	// 设置鼠标在视口内的锁定行为。EMouseLockMode::DoNotLock 表示不锁定鼠标到视口内，即鼠标可以自由移动，不会被限制在视口内部。
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	// 设置了在捕获输入时是否隐藏鼠标指针。false 表示捕获输入时不隐藏鼠标指针，即鼠标指针在捕获输入时仍然可见。
	InputModeData.SetHideCursorDuringCapture(false);
	// 应用到当前的输入模式中。
	SetInputMode(InputModeData);
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if(AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
 
	UAuraInputComponent* AuraEnhancedInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	
	// 将 MoveAction 的输入动作绑定到当前玩家控制器对象的 Move 方法上，当该输入动作被触发时，将会执行 Move 方法中的代码逻辑
	AuraEnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraEnhancedInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	// 获取输入动作的值，将其转换为二维向量
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	// 获取当前玩家控制器的旋转值 
	const FRotator Rotator = GetControlRotation();
	// 创建一个只包含 Yaw 轴旋转值的旋转对象
	const FRotator YawRotator(0.f, Rotator.Yaw, 0.f);

	// 获取角色在世界空间中的前进方向的单位向量-1~1
	const FVector ForwardDirection = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X);
	// 获取角色在世界空间中的右侧方向的单位向量-1~1
	const FVector RightDirection = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y);

	if(APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::AbilityInputTagPressed(const FGameplayTag InputTag)
{
	// GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, *InputTag.ToString());
	if(InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		bTargeting = CurrActor ? true : false;
		bAutoRunning = false;
	}
}

void AAuraPlayerController::AbilityInputTagReleased(const FGameplayTag InputTag)
{
	// GEngine->AddOnScreenDebugMessage(2, 2.f, FColor::Red, *InputTag.ToString());
	
	if(InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB) == false)
	{
		if(GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
		return;
	}
	
	if(bTargeting)
	{
		if(GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
	}
	else
	{
		/* 单击自动移动 */
		APawn* ControlledPawn = GetPawn();
		if(ControlledPawn && FollowTime <= ShortPressThreshold)
		{
			if(UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				SplineCmp->ClearSplinePoints();
				for(const FVector& PointLoc : NavPath->PathPoints)
				{
					SplineCmp->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
					DrawDebugSphere(GetWorld(), PointLoc, 8.f, 8, FColor::Green, false, 5.f);
				}
				// 如果点击位置为自动寻路无法到达的位置，导航还是会生成一条路径，但是我们无法达到最终点，这样无法停止自动寻路
				// 需要在鼠标抬起时，将路径的终点设置给CachedDestination，这也是自动寻路的最终点。获取数组中的最后一个点
				CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
				bAutoRunning = true;
			}
		}
		FollowTime = 0.f;
		bTargeting = false;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(const FGameplayTag InputTag)
{
	// GEngine->AddOnScreenDebugMessage(3, 3.f, FColor::Orange, *InputTag.ToString());

	if(InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB) == false)
	{
		/* 其它输入 */
		if(GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;
	}

	if(bTargeting)
	{
		/* 选中目标攻击 */
		if(GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
	}
	else
	{
		/* 鼠标按住移动 */
		// 累计经过的时间 GetDeltaSeconds用于获取自上一帧以来的时间增量（通常以秒为单位）。
		FollowTime += GetWorld()->GetDeltaSeconds();
		// 获取目标点
		if(CursorHit.bBlockingHit)
		{
			// ImpactPoint 射线命中物体的世界坐标位置。
			CachedDestination = CursorHit.ImpactPoint;
		}
		// 向目标移动
		if(APawn* ControlledPawn = GetPawn())
		{
			// 获取目标方向并向其移动
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}
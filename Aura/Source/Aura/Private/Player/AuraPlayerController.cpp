// P


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(AuraContext);

	/*
	 * 绑定增强输入到玩家Controller
	 */
	// 从当前本地玩家对象中获取一个 用于增强输入管理的本地玩家子系统 类型的子系统
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(Subsystem);
	// 向输入子系统中添加一个输入映射上下文
	Subsystem->AddMappingContext(AuraContext, 0);

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

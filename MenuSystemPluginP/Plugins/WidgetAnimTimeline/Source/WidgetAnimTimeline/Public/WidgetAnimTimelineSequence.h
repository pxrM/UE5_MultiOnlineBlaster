#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "WidgetAnimTimelineSequence.generated.h"

class UWidgetAnimation;

UENUM(BlueprintType)
enum class EWidgetAnimTimelineEntryType : uint8
{
	// 直接播放目标 Widget 上的 UMG 动画。
	DirectAnimation,

	// 触发目标子 Widget 自己的时间轴阶段。
	ChildSequencePhase
};

UENUM(BlueprintType)
enum class EWidgetAnimTimelineInterruptMode : uint8
{
	// 播放前不处理同名动画，保持当前动画状态。
	None,

	// 播放前停止目标 Widget 上正在播放的同名动画。
	StopActiveAnimations,

	// 播放前把目标 Widget 上正在播放的同名动画推进到结尾并停止。
	FinishActiveAnimations
};

USTRUCT(BlueprintType)
struct WIDGETANIMTIMELINE_API FWidgetAnimTimelineEntry
{
	GENERATED_BODY()

	// 目标子 Widget 名称；None 表示使用拥有该时间轴配置的 Widget 自身。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	FName TargetWidgetName = NAME_None;
	
	// 选择该条目是直接播放 WidgetAnimation，还是触发子 Widget 的时间轴阶段。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	EWidgetAnimTimelineEntryType EntryType = EWidgetAnimTimelineEntryType::DirectAnimation;

	// EntryType 为 DirectAnimation 时播放的 UMG 动画名称。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline", meta = (EditCondition = "EntryType == EWidgetAnimTimelineEntryType::DirectAnimation", EditConditionHides))
	FName AnimationName = NAME_None;
	
	// EntryType 为 ChildSequencePhase 时，在目标子 Widget 时间轴中查找并播放的阶段名称。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline", meta = (EditCondition = "EntryType == EWidgetAnimTimelineEntryType::ChildSequencePhase", EditConditionHides))
	FName ChildPhaseName = NAME_None;

	// Timeline second at which this entry starts within its owning phase.
	// 该条目在所属阶段内的开始时间，单位为秒。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float StartTime = 0.0f;

	// 动画播放倍率；值越大播放越快。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline", meta = (ClampMin = "0.001", UIMin = "0.1"))
	float PlaybackRate = 1.0f;
	
	// 循环次数，遵循 UUserWidget::PlayAnimation 语义：0 表示无限循环；编辑器预览中会按一轮显示时长。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline", meta = (ClampMin = "0", UIMin = "0"))
	int32 NumLoopsToPlay = 1;

	// 播放该条目前，对目标 Widget 上同名动画的打断处理方式。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	EWidgetAnimTimelineInterruptMode InterruptMode = EWidgetAnimTimelineInterruptMode::StopActiveAnimations;
};

USTRUCT(BlueprintType)
struct WIDGETANIMTIMELINE_API FWidgetAnimTimelinePhase
{
	GENERATED_BODY()

	// 阶段名称；运行时通过该名称播放对应阶段。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	FName PhaseName = NAME_None;

	// 阶段内的时间轴条目列表，会按各自 StartTime 调度执行。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	TArray<FWidgetAnimTimelineEntry> Entries;
};

USTRUCT(BlueprintType)
struct WIDGETANIMTIMELINE_API FWidgetAnimTimelineConfig
{
	GENERATED_BODY()

	// 该 Widget 拥有的所有动画时间轴阶段。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	TArray<FWidgetAnimTimelinePhase> Phases;

	// Optional phase played when the host widget is constructed.
	// 宿主 Widget 构建完成时自动播放的阶段名称；None 表示不自动播放。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	FName AutoPlayPhaseName = NAME_None;
};

UCLASS(BlueprintType)
class WIDGETANIMTIMELINE_API UWidgetAnimTimelinePlayer : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Widget Anim Timeline")
	void Initialize(UUserWidget* InOwnerWidget, const FWidgetAnimTimelineConfig& InConfig);

	UFUNCTION(BlueprintCallable, Category = "Widget Anim Timeline")
	bool PlayPhase(FName PhaseName);

	UFUNCTION(BlueprintCallable, Category = "Widget Anim Timeline")
	void Stop();

private:
	struct FActiveTimelineAnimation
	{
		TWeakObjectPtr<UUserWidget> TargetWidget;
		TWeakObjectPtr<UWidgetAnimation> Animation;
		EWidgetAnimTimelineInterruptMode InterruptMode = EWidgetAnimTimelineInterruptMode::StopActiveAnimations;
		FTimerHandle CleanupTimer;
	};

	bool PlayPhaseInternal(FName PhaseName, TSet<FString>& PhaseStack);
	void ScheduleEntry(const FWidgetAnimTimelineEntry& Entry, const TSet<FString>& PhaseStack);
	void ExecuteEntry(FWidgetAnimTimelineEntry Entry, TSet<FString> PhaseStack);
	static void ApplyInterruptMode(UUserWidget* TargetWidget, UWidgetAnimation* Animation, EWidgetAnimTimelineInterruptMode InterruptMode);
	void TrackActiveAnimation(UUserWidget* TargetWidget, UWidgetAnimation* Animation, const FWidgetAnimTimelineEntry& Entry, float PlaybackRate, int32 NumLoopsToPlay);
	void RemoveActiveAnimation(UUserWidget* TargetWidget, UWidgetAnimation* Animation);
	void CleanupFinishedAnimation(TWeakObjectPtr<UUserWidget> TargetWidget, TWeakObjectPtr<UWidgetAnimation> Animation);
	UUserWidget* ResolveTargetWidget(FName TargetWidgetName) const;
	static UWidgetAnimation* ResolveAnimation(UUserWidget* TargetWidget, FName AnimationName);
	const FWidgetAnimTimelinePhase* FindPhase(FName PhaseName) const;
	FString MakePhaseStackKey(FName PhaseName) const;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> OwnerWidget;

	UPROPERTY(Transient)
	FWidgetAnimTimelineConfig Config;

	UPROPERTY(Transient)
	TArray<FTimerHandle> ActiveTimers;

	TArray<FActiveTimelineAnimation> ActiveAnimations;
	TArray<TWeakObjectPtr<UWidgetAnimTimelinePlayer>> ActiveChildPlayers;
};

UCLASS(Abstract, Blueprintable)
class WIDGETANIMTIMELINE_API UWidgetAnimTimelineHostWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Widget Anim Timeline")
	UWidgetAnimTimelinePlayer* GetAnimTimelinePlayer() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget Anim Timeline")
	FWidgetAnimTimelineConfig AnimTimelineConfig;

	UPROPERTY(Transient)
	TObjectPtr<UWidgetAnimTimelinePlayer> AnimTimelinePlayer;
};

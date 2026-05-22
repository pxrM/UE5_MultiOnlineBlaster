#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "WidgetAnimTimelineSequence.generated.h"

class UWidgetAnimation;

UENUM(BlueprintType)
enum class EWidgetAnimTimelineEntryType : uint8
{
	DirectAnimation,
	ChildSequencePhase
};

UENUM(BlueprintType)
enum class EWidgetAnimTimelineInterruptMode : uint8
{
	None,
	StopActiveAnimations,
	FinishActiveAnimations
};

USTRUCT(BlueprintType)
struct WIDGETANIMTIMELINE_API FWidgetAnimTimelineEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	FName TargetWidgetName = NAME_None;

	// Selects whether this entry plays a WidgetAnimation directly or triggers a child timeline phase.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	EWidgetAnimTimelineEntryType EntryType = EWidgetAnimTimelineEntryType::DirectAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline", meta = (EditCondition = "EntryType == EWidgetAnimTimelineEntryType::DirectAnimation", EditConditionHides))
	FName AnimationName = NAME_None;

	// Phase name resolved from a child UWidgetAnimTimelineHostWidget when EntryType is ChildSequencePhase.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline", meta = (EditCondition = "EntryType == EWidgetAnimTimelineEntryType::ChildSequencePhase", EditConditionHides))
	FName ChildPhaseName = NAME_None;

	// Timeline second at which this entry starts within its owning phase.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline", meta = (ClampMin = "0.001", UIMin = "0.1"))
	float PlaybackRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	int32 NumLoopsToPlay = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	EWidgetAnimTimelineInterruptMode InterruptMode = EWidgetAnimTimelineInterruptMode::StopActiveAnimations;
};

USTRUCT(BlueprintType)
struct WIDGETANIMTIMELINE_API FWidgetAnimTimelinePhase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	FName PhaseName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	TArray<FWidgetAnimTimelineEntry> Entries;
};

USTRUCT(BlueprintType)
struct WIDGETANIMTIMELINE_API FWidgetAnimTimelineConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline")
	TArray<FWidgetAnimTimelinePhase> Phases;

	// Optional phase played when the host widget is constructed.
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
	void ScheduleEntry(const FWidgetAnimTimelineEntry& Entry);
	void ExecuteEntry(FWidgetAnimTimelineEntry Entry);
	UUserWidget* ResolveTargetWidget(FName TargetWidgetName) const;
	UWidgetAnimation* ResolveAnimation(UUserWidget* TargetWidget, FName AnimationName) const;
	const FWidgetAnimTimelinePhase* FindPhase(FName PhaseName) const;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> OwnerWidget;

	UPROPERTY(Transient)
	FWidgetAnimTimelineConfig Config;

	UPROPERTY(Transient)
	TArray<FTimerHandle> ActiveTimers;
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

#pragma once

#include "CoreMinimal.h"
#include "Character/LyraCharacterMovementComponent.h"
#include "GameplayTagContainer.h"
#include "LyraFPSCharacterMovementComponent.generated.h"

class UAbilitySystemComponent;


USTRUCT(BlueprintType)
struct FLyraFPSPredictedMoveFlags
{
	GENERATED_BODY()
	enum : uint8
	{
		None			= 0,
		Sprint			= 1 << 0,
		ADS				= 1 << 1,
		Walk			= 1 << 2,
		Crouch			= 1 << 3,
	};
};

UCLASS(Config = Game)
class LYRAFPSGAMERUNTIME_API ULyraFPSCharacterMovementComponent : public ULyraCharacterMovementComponent
{
	GENERATED_BODY()

public:
	ULyraFPSCharacterMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual float GetMaxSpeed() const override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

public:
	// ----------------------------
	// 给 Input / GameplayAbility 调用的接口
	// ----------------------------
	UFUNCTION(BlueprintCallable, Category = "Lyra|Movement")
	void SetPredictedMoveFlag(uint8 Flag, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Lyra|Movement")
	void ClearPredictedMoveFlags();

	UFUNCTION(BlueprintPure, Category = "Lyra|Movement")
	bool HasPredictedMoveFlag(uint8 Flag) const;

	UFUNCTION(BlueprintPure, Category = "LyraFPS|Movement")
	uint8 GetPredictedMovementFlags() const { return PredictedMoveFlags; }
	
	// 便捷接口
	/** 自动处理walk/jog/sprint状态 */
	UFUNCTION(BlueprintCallable, Category = "LyraFPS|Movement")
	void SetPredictedGaitFlag(uint8 NewGaitFlag);
	
	UFUNCTION(BlueprintCallable, Category = "Lyra|Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Lyra|Movement")
	void StopSprint();

	UFUNCTION(BlueprintCallable, Category = "Lyra|Movement")
	void StartADS();

	UFUNCTION(BlueprintCallable, Category = "Lyra|Movement")
	void StopADS();
	

public:
	// NetworkMoveData 反序列化后调用。
	
	// 服务端接收客户端 move 时，会把自定义数据写回 MovementComponent。
	// InPackedSpeedScale：除去本地预测Input、Tag外的第三种预测方式-直接设置倍率
	void ApplyNetworkMoveData(uint8 InMoveFlags,  uint16 InPackedSpeedScale);

protected:
	// ----------------------------
	// 速度配置
	// ----------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Movement|Speed")
	float WalkSpeedMultiplier = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Movement|Speed")
	float JogSpeedMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Movement|Speed")
	float SprintSpeedMultiplier = 1.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Movement|Speed")
	float ADSSpeedMultiplier = 0.8f;
	
#pragma region MovementTag
public:
	UAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;
	bool HasOwnerMatchingGameplayTag(const FGameplayTag& Tag) const;
	int32 GetOwnerGameplayTagCount(const FGameplayTag& Tag) const;
	
	float GetSpeedScaleFromGAS() const;
	bool IsMovementBlockedByGAS() const;
	bool IsKnockedDownByGAS() const;
protected:
	// ----------------------------
	// GAS / Lyra GameplayTag 状态，这部分用于被动减速/加速的情况，它不需要本地预测。
	// ----------------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Movement|Tags")
	FGameplayTag MovementDisabledTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Movement|Tags")
	FGameplayTag KnockedDownTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Movement|Tags")
	FGameplayTag StunnedTag;
	
protected:
	// 倒地倍率。最终速度 = Super::GetMaxSpeed() * KnockedDownSpeedMultiplier
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Movement|Speed")
	float KnockedDownSpeedMultiplier = 0.4f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Movement|Speed")
	float CrouchSpeedMultiplier = 0.55f;
#pragma endregion MovementTag
	
	


	//通过参数直接设置速度缩放
	//方便更精细化的移速预测(负重动态变化)
	//可以去除以将带宽减少：24->8
#pragma region PredictedSpeed
public:
	UFUNCTION(BlueprintCallable, Category = "Lyra|Movement")
	void SetPredictedSpeedScale(float NewSpeedScale);

	UFUNCTION(BlueprintPure, Category = "Lyra|Movement")
	float GetPredictedSpeedScale() const { return PredictedSpeedScale; }
protected:
	static uint16 PackPredictedSpeedScale(float InPredictedSpeedScale);
	static float UnpackPredictedSpeedScale(uint16 InPredictedPackedSpeedScale);
protected:
	UPROPERTY(Transient)
	float PredictedSpeedScale = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Movement|Speed")
	float MinPredictedSpeedScale = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Movement|Speed")
	float MaxPredictedSpeedScale = 3.0f;
#pragma endregion PredictedSpeed
	
	/** 网络预测相关 */
#pragma region Predict
protected:
	float GetSpeedScaleFromPredictedMoveFlags() const;

protected:
	// 当前预测输入态
	// 通过 SavedMove + NetworkMoveData 进入移动 RPC。
	UPROPERTY(Transient)
	uint8 PredictedMoveFlags = 0;
	
protected:
	// ----------------------------
	// SavedMove
	// ----------------------------
	class FSavedMove_LyraFPS : public FSavedMove_Character
	{
	public:
		typedef FSavedMove_Character Super;

		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		// 是否合并 Move数据
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
		// 采集当前帧数据
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, const FVector& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		// Replay 时恢复
		virtual void PrepMoveFor(ACharacter* Character) override;

	public:
		uint8 SavedPredictedMoveFlags = 0;
		uint16 SavedPackedSpeedScale = 1000;
	};
	
	class FNetworkPredictionData_Client_LyraFPS : public FNetworkPredictionData_Client_Character
	{
	public:
		typedef FNetworkPredictionData_Client_Character Super;

		FNetworkPredictionData_Client_LyraFPS(const UCharacterMovementComponent& ClientMovement);

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	// ----------------------------
	// NetworkMoveData
	// ----------------------------

	struct FCharacterNetworkMoveData_LyraFPS : public FCharacterNetworkMoveData
	{
		typedef FCharacterNetworkMoveData Super;

		uint8 MoveFlags = 0;
		uint16 PackedSpeedScale = 1000;

		virtual void ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType) override;
		virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType) override;
	};

	struct FCharacterNetworkMoveDataContainer_LyraFPS : public FCharacterNetworkMoveDataContainer
	{
		//NewMove PendingMove OldMove
		FCharacterNetworkMoveData_LyraFPS MoveData[3];

		FCharacterNetworkMoveDataContainer_LyraFPS();
	};

protected:
	FCharacterNetworkMoveDataContainer_LyraFPS LyraNetworkMoveDataContainer;
#pragma endregion Predict
};
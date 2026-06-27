// Fill out your copyright notice in the Description page of Project Settings.



#include "Gameplay/Cosmetic/LyraPawnComp_CharacterParts_FPS.h"


#include "GameFramework/Character.h"
#include "Gameplay/Cosmetic/CharacterPartData_FPS.h"


// Sets default values for this component's properties
ULyraPawnComp_CharacterParts_FPS::ULyraPawnComp_CharacterParts_FPS()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void ULyraPawnComp_CharacterParts_FPS::BeginPlay()
{
	Super::BeginPlay();

	// ...
	GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::WarmUpInitialAnimPose);
}

void ULyraPawnComp_CharacterParts_FPS::WarmUpInitialAnimPose()
{
	if (auto ownerChar=Cast<ACharacter>(GetOwner()))
	{
		USkeletalMeshComponent* MeshComp = ownerChar-> GetMesh();
		if (!MeshComp || !MeshComp->GetSkeletalMeshAsset() || !MeshComp->GetAnimInstance())
		{
			return;
		}

		// 只做一次：评估 AnimBP 当前状态，不推进时间。
		MeshComp->TickPose(0.0f, false);

		// 把刚评估出的 Pose 写进骨骼变换，避免离屏生成后首次可见仍是 RefPose/APose。
		MeshComp->RefreshBoneTransforms(nullptr);

		MeshComp->UpdateComponentToWorld();
		MeshComp->MarkRenderDynamicDataDirty();
	}
	
}

int32 ULyraPawnComp_CharacterParts_FPS::AddCharacterPart_FPS(const FLyraCharacterPart_FPS& NewPart)
{
	FLyraAppliedCharacterPartEntry_FPS& NewEntry = LocalParts.AddDefaulted_GetRef();
	NewEntry.Part = NewPart;
	NewEntry.PartHandle = PartHandleCounter++;
	if (SpawnActorForEntry_FPS(NewEntry))
	{
		BroadcastChanged();
		return NewEntry.PartHandle;
	}
	return INDEX_NONE;
}

void ULyraPawnComp_CharacterParts_FPS::RemoveCharacterPart_FPS(int32 Handle)
{
	for (int32 i=0;i< LocalParts.Num();++i)
	{
		if (LocalParts[i].PartHandle== Handle)
		{
			LocalParts.RemoveAt(i);
		}
	}
}

bool ULyraPawnComp_CharacterParts_FPS::SpawnActorForEntry_FPS(FLyraAppliedCharacterPartEntry_FPS& Entry)
{
	if (Entry.Part.PartClass != nullptr)
	{
		if (USceneComponent* ComponentToAttachTo = GetSceneComponentToAttachTo_FPS(Entry.Part))
		{
			// const FTransform SpawnTransform = ComponentToAttachTo->GetSocketTransform(Entry.Part.SocketName);

			UChildActorComponent* PartComponent = NewObject<UChildActorComponent>(GetOwner());

			PartComponent->SetupAttachment(ComponentToAttachTo, Entry.Part.SocketName);
			PartComponent->SetChildActorClass(Entry.Part.PartClass);
			PartComponent->RegisterComponent();

			if (AActor* SpawnedActor = PartComponent->GetChildActor())
			{
				SpawnedActor->SetOwner(GetOwner());
				switch (Entry.Part.CollisionMode)
				{
				case ECharacterCustomizationCollisionMode::UseCollisionFromCharacterPart:
					// Do nothing
					break;

				case ECharacterCustomizationCollisionMode::NoCollision:
					SpawnedActor->SetActorEnableCollision(false);
					break;
				}

				// Set up a direct tick dependency to work around the child actor component not providing one
				if (USceneComponent* SpawnedRootComponent = SpawnedActor->GetRootComponent())
				{
					SpawnedRootComponent->AddTickPrerequisiteComponent(ComponentToAttachTo);
				}
			}

			Entry.SpawnedComponent = PartComponent;
			return true;
		}
	}
	return false;
}


USceneComponent* ULyraPawnComp_CharacterParts_FPS::GetSceneComponentToAttachTo(
	const FLyraCharacterPart& Part) const
{
	//通过组件Tag查找父组件
	if (Part.PartClass.Get())
	{
		TArray<FName>& PartTags = Cast<AActor>(Part.PartClass.GetDefaultObject())->Tags;
		if (PartTags.Num() > 0)
		{
			if (AActor* OwnerActor = GetOwner())
			{
				if (ACharacter* OwningCharacter = Cast<ACharacter>(OwnerActor))
				{
					if (USkeletalMeshComponent* MeshComponent = Cast<USkeletalMeshComponent>(
						OwningCharacter->FindComponentByTag(USkeletalMeshComponent::StaticClass(), PartTags[0])))
					{
						return MeshComponent;
					}
				}
			}
		}
	}

	
	return Super::GetSceneComponentToAttachTo(Part);
}

USceneComponent* ULyraPawnComp_CharacterParts_FPS::GetSceneComponentToAttachTo_FPS(
	const FLyraCharacterPart_FPS& Part) const
{
	if (IsValid(Part.CustomAttachComp))
	{
		if (AActor* OwnerActor = GetOwner())
		{
			if (auto customComp = Cast<USceneComponent>(OwnerActor->GetComponentByClass(Part.CustomAttachComp)))
			{
				return customComp;
			}
		}
	}
	if (USkeletalMeshComponent* MeshComponent = GetParentMeshComponent())
	{
		return MeshComponent;
	}
	else if (AActor* OwnerActor = GetOwner())
	{
		return OwnerActor->GetRootComponent();
	}
	return nullptr;
}

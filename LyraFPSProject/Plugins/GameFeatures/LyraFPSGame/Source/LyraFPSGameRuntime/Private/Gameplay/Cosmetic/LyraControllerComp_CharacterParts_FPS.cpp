// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Cosmetic/LyraControllerComp_CharacterParts_FPS.h"
#include "Cosmetics/LyraPawnComponent_CharacterParts.h"
#include "Gameplay/Cosmetic/CharacterPartData_FPS.h"
#include "Gameplay/Cosmetic/LyraPawnComp_CharacterParts_FPS.h"
#include "GameFramework/CheatManagerDefines.h"
#include "Cosmetics/LyraCosmeticDeveloperSettings.h"


// Sets default values for this component's properties
ULyraControllerComp_CharacterParts_FPS::ULyraControllerComp_CharacterParts_FPS(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


// Called when the game starts
void ULyraControllerComp_CharacterParts_FPS::BeginPlay()
{
	Super::BeginPlay();

	// 原版Lyra仅在服务器生成，但FPS模型需要仅在所属客户端生成
	// 仅修改判断部分，核心逻辑相同
	
	// Listen for pawn possession changed events
	if (HasAuthority())
	{
		if (AController* OwningController = GetController<AController>())
		{
			OwningController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);

			if (APawn* ControlledPawn = GetPawn<APawn>())
			{
				OnPossessedPawnChanged(nullptr, ControlledPawn);
			}
		}

		ApplyDeveloperSettings();
	}
	
	//Local
	if (AController* OwningController = GetController<AController>())
	{
		if (OwningController->IsLocalController())
		{
			OwningController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged_Local);

			if (APawn* ControlledPawn = GetPawn<APawn>())
			{
				OnPossessedPawnChanged_Local(nullptr, ControlledPawn);
			}
		}
	}
}

void ULyraControllerComp_CharacterParts_FPS::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveAllCharacterParts();
	Super::EndPlay(EndPlayReason);
}

ULyraPawnComponent_CharacterParts* ULyraControllerComp_CharacterParts_FPS::GetPawnCustomizer_FPS() const
{
	APawn* ControlledPawn = GetPawn<APawn>();
	if (ControlledPawn)
	{
		auto charPart = ControlledPawn->FindComponentByClass<ULyraPawnComponent_CharacterParts>();
		return charPart;
	}
	return nullptr;
}

void ULyraControllerComp_CharacterParts_FPS::AddCharacterPart(const FLyraCharacterPart_FPS& NewPart)
{
	AddCharacterPartInternal(NewPart, ECharacterPartSource::Natural);
}

void ULyraControllerComp_CharacterParts_FPS::AddCharacterPartInternal(const FLyraCharacterPart_FPS& NewPart,
                                                                      ECharacterPartSource Source)
{
	//本地
	if (NewPart.bIsLocally)
	{
		if (auto ownerController = GetController<APlayerController>())
		{
			if (ownerController->IsLocalController())
			{
				FLyraControllerCharacterPartEntry_FPS NewEntry;
				NewEntry.Part = NewPart;
				NewEntry.Source = Source;
				if (auto PawnCustomizer = Cast<ULyraPawnComp_CharacterParts_FPS>(GetPawnCustomizer_FPS()))
				{
					if (Source != ECharacterPartSource::NaturalSuppressedViaCheat)
					{
						NewEntry.Handle = PawnCustomizer->AddCharacterPart_FPS(NewPart);
					}
				}
				CharacterParts_Local.Add(NewEntry);
			}
		}
	}
	//全局
	else
	{
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			FLyraControllerCharacterPartEntry NewEntry;
			NewEntry.Part = NewPart.ToPart();
			NewEntry.Source = Source;
			if (auto PawnCustomizer = Cast<ULyraPawnComp_CharacterParts_FPS>(GetPawnCustomizer_FPS()))
			{
				if (Source != ECharacterPartSource::NaturalSuppressedViaCheat)
				{
					NewEntry.Handle = PawnCustomizer->AddCharacterPart(NewPart.ToPart());
				}
			}
			CharacterParts.Add(NewEntry);
		}
	}
}

void ULyraControllerComp_CharacterParts_FPS::RemoveCharacterPart(const FLyraCharacterPart_FPS& PartToRemove)
{
	for (auto EntryIt = CharacterParts.CreateIterator(); EntryIt; ++EntryIt)
	{
		if (FLyraCharacterPart_FPS::AreEquivalentParts(EntryIt->Part, PartToRemove))
		{
			if (ULyraPawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer_FPS())
			{
				PawnCustomizer->RemoveCharacterPart(EntryIt->Handle);
			}

			EntryIt.RemoveCurrent();
			break;
		}
	}
}

void ULyraControllerComp_CharacterParts_FPS::RemoveAllCharacterParts()
{
	if (ULyraPawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer_FPS())
	{
		for (FLyraControllerCharacterPartEntry& Entry : CharacterParts)
		{
			PawnCustomizer->RemoveCharacterPart(Entry.Handle);
		}
	}

	CharacterParts.Reset();
}

void ULyraControllerComp_CharacterParts_FPS::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	// Remove from the old pawn
	if (auto OldCustomizer = OldPawn ? OldPawn->FindComponentByClass<ULyraPawnComponent_CharacterParts>() : nullptr)
	{
		for (FLyraControllerCharacterPartEntry& Entry : CharacterParts)
		{
			OldCustomizer->RemoveCharacterPart(Entry.Handle);
			Entry.Handle.Reset();
		}
	}

	// Apply to the new pawn
	if (auto NewCustomizer = NewPawn ? NewPawn->FindComponentByClass<ULyraPawnComponent_CharacterParts>() : nullptr)
	{
		for (FLyraControllerCharacterPartEntry& Entry : CharacterParts)
		{
			// Don't readd if it's already there, this can get called with a null oldpawn
			if (!Entry.Handle.IsValid() && Entry.Source != ECharacterPartSource::NaturalSuppressedViaCheat)
			{
				Entry.Handle = NewCustomizer->AddCharacterPart(Entry.Part);
			}
		}
	}
}

void ULyraControllerComp_CharacterParts_FPS::OnPossessedPawnChanged_Local(APawn* OldPawn, APawn* NewPawn)
{
	if (auto OldCustomizer = OldPawn ? OldPawn->FindComponentByClass<ULyraPawnComp_CharacterParts_FPS>() : nullptr)
	{
		for (auto& Entry : CharacterParts_Local)
		{
			OldCustomizer->RemoveCharacterPart_FPS(Entry.Handle);
			Entry.Handle = INDEX_NONE;
		}
	}

	// Apply to the new pawn
	if (auto NewCustomizer = NewPawn ? NewPawn->FindComponentByClass<ULyraPawnComp_CharacterParts_FPS>() : nullptr)
	{
		for (FLyraControllerCharacterPartEntry_FPS& LocalEntry : CharacterParts_Local)
		{
			// Don't readd if it's already there, this can get called with a null oldpawn
			if (LocalEntry.Handle==INDEX_NONE && LocalEntry.Source != ECharacterPartSource::NaturalSuppressedViaCheat)
			{
				LocalEntry.Handle = NewCustomizer->AddCharacterPart_FPS(LocalEntry.Part);
			}
		}
	}
}

void ULyraControllerComp_CharacterParts_FPS::ApplyDeveloperSettings()
{
#if UE_WITH_CHEAT_MANAGER
	const ULyraCosmeticDeveloperSettings* Settings = GetDefault<ULyraCosmeticDeveloperSettings>();

	// Suppress or unsuppress natural parts if needed
	const bool bSuppressNaturalParts = (Settings->CheatMode == ECosmeticCheatMode::ReplaceParts) && (Settings->
		CheatCosmeticCharacterParts.Num() > 0);
	SetSuppressionOnNaturalParts(bSuppressNaturalParts);

	// Remove anything added by developer settings and re-add it
	ULyraPawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer_FPS();
	for (auto It = CharacterParts.CreateIterator(); It; ++It)
	{
		if (It->Source == ECharacterPartSource::AppliedViaDeveloperSettingsCheat)
		{
			if (PawnCustomizer != nullptr)
			{
				PawnCustomizer->RemoveCharacterPart(It->Handle);
			}
			It.RemoveCurrent();
		}
	}

	// Add new parts
	for (const FLyraCharacterPart& BasePartDesc : Settings->CheatCosmeticCharacterParts)
	{
		FLyraCharacterPart_FPS PartDescFPS;
		static_cast<FLyraCharacterPart&>(PartDescFPS) = BasePartDesc;
		AddCharacterPartInternal(PartDescFPS, ECharacterPartSource::AppliedViaDeveloperSettingsCheat);
	}
#endif
}


void ULyraControllerComp_CharacterParts_FPS::AddCheatPart(const FLyraCharacterPart_FPS& NewPart,
                                                          bool bSuppressNaturalParts)
{
#if UE_WITH_CHEAT_MANAGER
	SetSuppressionOnNaturalParts(bSuppressNaturalParts);
	AddCharacterPartInternal(NewPart, ECharacterPartSource::AppliedViaCheatManager);
#endif
}

void ULyraControllerComp_CharacterParts_FPS::ClearCheatParts()
{
#if UE_WITH_CHEAT_MANAGER
	ULyraPawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer_FPS();

	// Remove anything added by cheat manager cheats
	for (auto It = CharacterParts.CreateIterator(); It; ++It)
	{
		if (It->Source == ECharacterPartSource::AppliedViaCheatManager)
		{
			if (PawnCustomizer != nullptr)
			{
				PawnCustomizer->RemoveCharacterPart(It->Handle);
			}
			It.RemoveCurrent();
		}
	}

	ApplyDeveloperSettings();
#endif
}

void ULyraControllerComp_CharacterParts_FPS::SetSuppressionOnNaturalParts(bool bSuppressed)
{
#if UE_WITH_CHEAT_MANAGER
	ULyraPawnComponent_CharacterParts* PawnCustomizer = GetPawnCustomizer_FPS();

	for (FLyraControllerCharacterPartEntry& Entry : CharacterParts)
	{
		if ((Entry.Source == ECharacterPartSource::Natural) && bSuppressed)
		{
			// Suppress
			if (PawnCustomizer != nullptr)
			{
				PawnCustomizer->RemoveCharacterPart(Entry.Handle);
				Entry.Handle.Reset();
			}
			Entry.Source = ECharacterPartSource::NaturalSuppressedViaCheat;
		}
		else if ((Entry.Source == ECharacterPartSource::NaturalSuppressedViaCheat) && !bSuppressed)
		{
			// Unsuppress
			if (PawnCustomizer != nullptr)
			{
				Entry.Handle = PawnCustomizer->AddCharacterPart(Entry.Part);
			}
			Entry.Source = ECharacterPartSource::Natural;
		}
	}
#endif
}

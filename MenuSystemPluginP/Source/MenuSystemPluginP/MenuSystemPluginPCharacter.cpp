// Copyright Epic Games, Inc. All Rights Reserved.

#include "MenuSystemPluginPCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
// My Add
#include "OnlineSubsystemSteam.h"
#include "OnlineSessionSettings.h"


//////////////////////////////////////////////////////////////////////////
// AMenuSystemPluginPCharacter

AMenuSystemPluginPCharacter::AMenuSystemPluginPCharacter()
	:CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &AMenuSystemPluginPCharacter::OnCreateSessionComplete))
	, FindSessionCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete))
	, JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJionSessionComplete))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// My Add
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
		//��֤�Ƿ��ҵ���������ϵͳ���鿴����ʹ�õ���ϵͳ
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,//�Ը���Ϊkey�������Ͳ��ᶥ���֮ǰ����Ϣ
				15.f,//��ʾ15��
				FColor::Blue,
				FString::Printf(TEXT("Found subsystem %s"), *OnlineSubsystem->GetSubsystemName().ToString())
			);
		}
	}
}

void AMenuSystemPluginPCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AMenuSystemPluginPCharacter::CreateGameSession()
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}
	// ���Ự�Ƿ��Ѿ�����,�������������
	// ��������Ҫʱ�����ͨ�����紫�������������ٸûỰ
	// ��ʱ���������������CreateSession����ûỰ���ܻ�û�б����٣���������������ǻᴴ��ʧ��
	// ���ڲ��������������
	auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession)
	{
		OnlineSessionInterface->DestroySession(NAME_GameSession);
	}
	// ��Ӵ���ί��
	OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	// �������߻Ự������
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = false; //�Ƿ�ֻ�ھ�����
	SessionSettings->NumPublicConnections = 4; //�趨������������
	SessionSettings->bAllowJoinInProgress = true; //�Ƿ��������Ϸ;�м���
	SessionSettings->bAllowJoinViaPresence = true; //�Ƿ�������Ҽ���
	SessionSettings->bShouldAdvertise = true; //����Steam�����Ự
	SessionSettings->bUsesPresence = true; //��ʾ�û�״̬��Ϣ
	SessionSettings->bUseLobbiesIfAvailable = true; //
	//���ûỰ�Ĺؼ��ֺͶ�Ӧ��ֵ
	SessionSettings->Set(FName("MathType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	// ��ȡ���ص�һ�����
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	// �����Ự
	OnlineSessionInterface->CreateSession(
		*LocalPlayer->GetPreferredUniqueNetId(),//��ȡΨһ����id
		NAME_GameSession,//�Ự��
		*SessionSettings //�Ự������
	);
}

void AMenuSystemPluginPCharacter::JoinGameSession()
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}
	OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegate);

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = 1000;	//���������Ѱ����
	SessionSearch->bIsLanQuery = false;	// �Ƿ�ʹ�þ�����
	//SEARCH_PRESENCE��ֻ��Ѱ���е���Ϸ����ֻ������ͬID����ϷSession
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
}

void AMenuSystemPluginPCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, FString::Printf(TEXT("Created session: %s"), *SessionName.ToString()));
		}

		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(FString("/Game/ThirdPerson/Maps/MLobbyMap?listen"));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("Failed to create session!")));
		}
	}
}

void AMenuSystemPluginPCharacter::OnFindSessionComplete(bool bWasSuccessful)
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}
	// �����������
	for (auto Result : SessionSearch->SearchResults)
	{
		FString Id = Result.GetSessionIdStr();
		FString UserName = Result.Session.OwningUserName;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan, FString::Printf(TEXT("Id: %s, User: %s"), *Id, *UserName));
		}
		FString MathType;
		Result.Session.SessionSettings.Get(FName("MathType"), MathType);
		if (MathType == FString("FreeForAll"))
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan, FString::Printf(TEXT("Joining Match Type: %s"), *MathType));
			}
			//���ί��
			OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
			//����JoinSessionί�У�������OnJionSessionComplete����
			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result);
		}
	}
}

void AMenuSystemPluginPCharacter::OnJionSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}
	//��ȡIP��ַ�������ͻ�����ת����IP��ַ�ļ�����������
	FString Address;
	if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString::Printf(TEXT("Connect: %s"), *Address));
		}
		APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (PlayerController)
		{
			PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);	//�ͻ�����ת����IP��ַ�ļ�����������
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMenuSystemPluginPCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMenuSystemPluginPCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMenuSystemPluginPCharacter::Look);

	}

}

void AMenuSystemPluginPCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMenuSystemPluginPCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}





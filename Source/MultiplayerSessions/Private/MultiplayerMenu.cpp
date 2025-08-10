// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerMenu.h"

#include "DebugHelper.h"
#include "ListViewEntryWidget.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/SpinBox.h"
#include "Components/StackBox.h"
#include "Components/TextBlock.h"

void UMultiplayerMenu::MultiplayerMenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, TSoftObjectPtr<UWorld> LobbyLevel)
{
	PathToLobby = LobbyLevel.GetLongPackageName().Append("?listen");
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;

	SetupMultiplayerSubsystem();
}

void UMultiplayerMenu::MultiplayerMenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;

	SetupMultiplayerSubsystem();
}

bool UMultiplayerMenu::Initialize()
{
	if(!Super::Initialize()) return false;

	if(HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UMultiplayerMenu::HostButtonClicked);
	}

	if(JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UMultiplayerMenu::JoinButtonClicked);
	}

	if(PlayerNumberInput)
	{
		PlayerNumberInput->OnValueChanged.AddDynamic(this, &UMultiplayerMenu::PlayerNumberValueChanged);
	}
	
	return true;
}

void UMultiplayerMenu::OnCreateSession(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		DebugHelper::PrintToLog("Session Created Successfully", FColor::Green);

		MultiplayerSessionsSubsystem->StartSession();
	}
	else
	{
		DebugHelper::PrintToLog("Session Creation Failed", FColor::Red);

		HostButton->SetIsEnabled(true);
		JoinButton->SetIsEnabled(true);
	}
}

void UMultiplayerMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	    if(MultiplayerSessionsSubsystem == nullptr) return;

    if(ServerList)
    {
        ServerList->ClearChildren();
    }

	for (auto Result : SessionResults)
	{
        if(!bIsJoining) return;

        // MultiplayerSessionsSubsystem->JoinSession(Result);

        FString Id = Result.GetSessionIdStr();
        FString UserName = Result.Session.OwningUserName;
        FString SettingsValue;
        FString GameType;

        Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
        Result.Session.SessionSettings.Get(FName("GameType"), GameType);

        DebugHelper::PrintToLog(FString::Printf(TEXT("Session %s - %s | Map: %s"), *Id, *UserName, *SettingsValue), FColor::Green);

        if(ServerList && GameType == FString("DeathEcho"))
        {
            //create a new widget 
            if(ListEntryWidget)
            {
                UWorld* World = GetWorld();
                if (World)
                {
                    UListViewEntryWidget * NewWidget = CreateWidget<UListViewEntryWidget>(World, ListEntryWidget);
                    if (NewWidget)
                    {
                        
                        //from NewWidget get a reference to the ServerTitle Text Block widget and change the server title
                        UTextBlock * ServerTitle = Cast<UTextBlock>(NewWidget->GetWidgetFromName("ServerTitle"));

                        if(ServerTitle)
                        {
                            ServerTitle->SetText(FText::FromString(FString::Printf(TEXT("Server: %s | Map: %s"),*UserName, *SettingsValue)));
                        }

                        NewWidget->Session = Result;

                        UButton * ServerJoinButton = Cast<UButton>(NewWidget->GetWidgetFromName("JoinButton"));

                        if(ServerJoinButton)
                        {
                            ServerJoinButton->SetToolTipText(FText::FromString(Id));
                        }

                        //add the new widget to ServerList StackBox
                        ServerList->AddChild(NewWidget);
                    }
                }
            }
        }
	}

    if(!bWasSuccessful)
    {
        // JoinButton->SetIsEnabled(true);
        HostButton->SetIsEnabled(true);
        JoinText->SetText(FText::FromString("Search"));
    }
}

void UMultiplayerMenu::JoinSession(FOnlineSessionSearchResult Session)
{
	FString Id = Session.GetSessionIdStr();
	FString UserName = Session.Session.OwningUserName;
	FString SettingsValue;

	Session.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

	DebugHelper::PrintToLog(FString::Printf(TEXT("Session %s - %s | Map: %s"), *Id, *UserName, *SettingsValue), FColor::Green);

	MultiplayerSessionsSubsystem->JoinSession(Session);//add the join session complete delegate
}

void UMultiplayerMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem * Subsystem = IOnlineSubsystem::Get();

	if(Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

		if(SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController * PlayerController = GetGameInstance()->GetFirstLocalPlayerController();

			if(PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}        
		}
	}

	if(Result != EOnJoinSessionCompleteResult::Success)
	{
		// JoinButton->SetIsEnabled(true);
		HostButton->SetIsEnabled(true);
		JoinText->SetText(FText::FromString("Search"));
	}

	bIsJoining = false;
}

void UMultiplayerMenu::OnDestroySession(bool bWasSuccessful)
{
	bWasSuccessful ? DebugHelper::PrintToLog("Session Destroyed Successfully", FColor::Green) : DebugHelper::PrintToLog("Session Destroy Failed", FColor::Red);
}

void UMultiplayerMenu::OnStartSession(bool bWasSuccessful)
{
	bWasSuccessful ? DebugHelper::PrintToLog("Session Started Successfully", FColor::Green) : DebugHelper::PrintToLog("Session Start Failed", FColor::Red);

	UWorld * World = GetWorld();

	if(World)
	{
		// World->ServerTravel("/Game/Maps/Arena_2?listen");//this should be the lobby level
		World->ServerTravel(PathToLobby);
	}
}

void UMultiplayerMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	JoinButton->SetIsEnabled(false);

	if(MapSelect)
	{
		MatchType = MapSelect->GetSelectedOption();
	}

	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();//in case destroy a session if its alive
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMultiplayerMenu::JoinButtonClicked()
{
	if(bIsJoining)
	{
		JoinCanceled();
		return;
	}

	HostButton->SetIsEnabled(false);
	JoinText->SetText(FText::FromString("Cancel"));

	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();//in case destroy a session if its alive
		MultiplayerSessionsSubsystem->FindSessions(10);//The size of the search results shouldn't really matter.  Epic's documentation for BuildUniqueId is Used to keep different builds from seeing each other during searches
	}

	bIsJoining = true;
}

void UMultiplayerMenu::JoinCanceled()
{
	bIsJoining = false;

	if(MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->GetSessionInterface())
	{
		MultiplayerSessionsSubsystem->GetSessionInterface()->CancelFindSessions();
	}

	HostButton->SetIsEnabled(true);
	JoinButton->SetIsEnabled(true);
	JoinText->SetText(FText::FromString("Search"));
}

void UMultiplayerMenu::SetupMultiplayerSubsystem()
{
	UGameInstance * GameInstance = GetGameInstance();

	if(GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if(MultiplayerSessionsSubsystem)//bind the custom delegates to the functions
	{
		DebugHelper::PrintToLog("SetupMultiplayerSubsystem", FColor::Green);
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UMultiplayerMenu::OnCreateSession);//moze i &ThisClass
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &UMultiplayerMenu::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &UMultiplayerMenu::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UMultiplayerMenu::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &UMultiplayerMenu::OnStartSession);
	}
}

void UMultiplayerMenu::PlayerNumberValueChanged(float NewValue)
{
	NumPublicConnections = NewValue;
}

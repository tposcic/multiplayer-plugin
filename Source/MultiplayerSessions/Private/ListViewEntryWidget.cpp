#include "ListViewEntryWidget.h"
#include "Components/Button.h"


void UListViewEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if(JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &UListViewEntryWidget::JoinGame);
    }
}


void UListViewEntryWidget::JoinGame()
{
    FString Id = Session.GetSessionIdStr();

    UGameInstance * GameInstance = GetGameInstance();

    if(GameInstance)
    {
        UMultiplayerSessionsSubsystem * MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();

        if(MultiplayerSessionsSubsystem)
        {
            JoinButton->SetIsEnabled(false);
            MultiplayerSessionsSubsystem->JoinSession(Session);//add the join session complete delegate
        }
    }
}
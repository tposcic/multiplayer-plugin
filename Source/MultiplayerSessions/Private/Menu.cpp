// Copyright: Toni Poscic

#include "Menu.h"
#include "Components/Slider.h"
#include "ListViewEntryWidget.h"
#include "MultiplayerMenu.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, TSoftObjectPtr<UWorld> LobbyLevel)
{
    SetupWidget();

    if (MultiplayerWidget)
    {
        MultiplayerWidget->MultiplayerMenuSetup(NumberOfPublicConnections, TypeOfMatch, LobbyLevel);
    }
}

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
    SetupWidget();

    if (MultiplayerWidget)
    {
        MultiplayerWidget->MultiplayerMenuSetup(NumberOfPublicConnections, TypeOfMatch, LobbyPath);
    }
}

void UMenu::SetupWidget()
{
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    SetIsFocusable(true); // previously bIsFocusable = true; -> now deprecated

    UWorld * World = GetWorld();

    if(World)
    {
        APlayerController * PlayerController = World->GetFirstPlayerController();

        if(PlayerController)
        {
            FInputModeUIOnly InputModeData;//we set the input mode to UI only so that we don't apply input to any pawns in the world and just focus on the UI
            InputModeData.SetWidgetToFocus(TakeWidget());//we set the widget to focus to the widget we are using
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);//we set the mouse lock behavior to not lock to the viewport (kao windowed mode)
            PlayerController->SetInputMode(InputModeData);//we set the input mode to the player controller
            PlayerController->SetShowMouseCursor(true);//we show the mouse cursor
        }
    }
}

bool UMenu::Initialize()
{
    if(!Super::Initialize()) return false;

    return true;
}

void UMenu::MenuTearDown()
{
    RemoveFromParent();

    UWorld * World = GetWorld();

    if(World)
    {
        APlayerController * PlayerController = World->GetFirstPlayerController();

        if(PlayerController)
        {
            FInputModeGameOnly InputModeData;//we set the input mode to game only so that we don't apply input to the UI and just focus on the pawns in the world
            PlayerController->SetInputMode(InputModeData);//we set the input mode to the player controller
            PlayerController->SetShowMouseCursor(false);//we hide the mouse cursor
        }
    }
}

void UMenu::NativeDestruct()
{
	MenuTearDown();
 
	Super::NativeDestruct();
}
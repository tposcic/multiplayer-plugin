// Copyright: Toni Poscic

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Menu.generated.h"

class UMultiplayerMenu;
/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/**
	 * Needs to be called in blueprints
	 * @param NumberOfPublicConnections 
	 * @param TypeOfMatch 
	 * @param LobbyLevel 
	 */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void MenuSetup(int32 NumberOfPublicConnections = 2, FString TypeOfMatch = FString(TEXT("DeathMatch")), TSoftObjectPtr<UWorld> LobbyLevel = nullptr);
	
	/**
	 * Sets up the menu with the specified parameters.
	 *
	 * @param NumberOfPublicConnections The number of public connections for the lobby.
	 * @param TypeOfMatch The type of match for the lobby.
	 * @param LobbyPath The path to the lobby.
	 */
	void MenuSetup(int32 NumberOfPublicConnections = 2, FString TypeOfMatch = FString(TEXT("DeathMatch")), FString LobbyPath = FString(TEXT("/Game/Maps/Lobby")));

protected:
	/**
	 * Initializes the menu.
	 * This function is called when the menu is being initialized.
	 * It sets up the button click events for the Host and Join buttons.
	 *
	 * @return True if the initialization was successful, false otherwise.
	 */
	virtual bool Initialize() override;
	/**
	 * Called when the menu is being destroyed.
	 * Performs any necessary cleanup before the menu is destroyed.
	 */
	virtual void NativeDestruct() override;
	
	UPROPERTY()
	TObjectPtr<UMultiplayerMenu> MultiplayerWidget;

private:
	/**
	 * Tears down the menu by removing it from the parent widget and resetting the input mode.
	 * This function is called when the menu is being closed.
	 */
	void MenuTearDown();

	/**
	 * Sets up the widget for the menu.
	 * Adds the widget to the viewport, sets its visibility to visible, and makes it focusable.
	 * Sets the input mode to UI only, locks the mouse to the viewport, and shows the mouse cursor.
	 */
	void SetupWidget();
};

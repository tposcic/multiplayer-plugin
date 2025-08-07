// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OptionsMenu.generated.h"

class UTextBlock;
class USlider;
class UBaseSettings;
class UButton;
class UComboBoxString;
/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UOptionsMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void SaveGraphicsSettings();
	
	UBaseSettings * GetSettings();

protected:
	virtual bool Initialize() override;
private:
	UPROPERTY(meta = (BindWidget))
	UButton * SaveGraphicsButton;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString * ResolutionSelect;

	UPROPERTY(meta = (BindWidget))
	USlider * MouseSensitivitySlider;

	UPROPERTY(meta = (BindWidget))
	USlider * GlobalVolumeSlider;
	
	UPROPERTY(meta = (BindWidget))
	UComboBoxString * DetailsSelect;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString * FullScreenModeSelect;

	UPROPERTY(meta = (BindWidget))
	UTextBlock * VersionText;
	
	UPROPERTY()
	UBaseSettings * Settings;

	UFUNCTION()
	void GraphicsQualityUpdate(int32 QualityLevel);
};

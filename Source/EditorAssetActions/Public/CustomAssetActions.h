// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimComposite.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Blueprint/UserWidget.h"
#include "Materials/MaterialInstance.h"
#include "Particles/ParticleSystem.h"
#include "Niagara/Classes/NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "CustomAssetActions.generated.h"

#if WITH_EDITOR

UCLASS()
class UCustomAssetActions : public UAssetActionUtility
{
	GENERATED_BODY()

public:

	/**
	 * Rename the selected asset
	 * @param SearchPattern - The pattern to search for
	 * @param ReplacePattern - The pattern to replace with
	 * @param SearchCase - should the case be ignored?
	 * */
	UFUNCTION(CallInEditor)
	void RenameSelectedAssets(const FString SearchPattern, const FString ReplacePattern, ESearchCase::Type SearchCase);
	
	/**
	 * Adds a prefix to the selected assets
	 **/
	UFUNCTION(CallInEditor)
	void AddAssetPrefix();

	/**
	 * Clean up Assets by moving them to a new folder
	 **/
	UFUNCTION(CallInEditor)
	void CleanUpFolder(FString ParentFolder = FString("/Game/"));

	/**
	 * Duplicate the selected asset
	 **/
	UFUNCTION(CallInEditor)
	void DuplicateSelectedAsset(uint32 NumberOfDuplicates, bool bSave);

	/**
	 *	Removes the selected assets if they are not referenced by any other asset
	 **/
	UFUNCTION(CallInEditor)
	void RemoveUnusedAssets(bool bDeleteImmediately = false);

	/**
	 * Checks if the Texture is a power of 2
	*/
	UFUNCTION(CallInEditor)
	void CheckIfTextureIsValidPowerOf2();
	

private:

	TMap<UClass*, FString> ClassToPrefixMap =
	{
		{UAnimationAsset::StaticClass(), "A_"},
		{UAnimSequence::StaticClass(), "A_"},
		{UAnimMontage::StaticClass(), "A_"},
		{UAnimComposite::StaticClass(), "A_"},
		{UAnimInstance::StaticClass(), "A_"},
		{UAnimBlueprint::StaticClass(), "ABP_"},
		{UBlueprint::StaticClass(), TEXT("BP_")},
		{UStaticMesh::StaticClass(), TEXT("SM_")},
		{UMaterial::StaticClass(), TEXT("M_")},
		{UMaterialInstance::StaticClass(), TEXT("MI_")},
		{UMaterialFunctionInterface::StaticClass(), TEXT("MF_")},
		{UParticleSystem::StaticClass(), TEXT("PS_")},
		{UNiagaraSystem::StaticClass(), TEXT("NS_")},
		{UNiagaraEmitter::StaticClass(), TEXT("NE_")},
		{USoundCue::StaticClass(), TEXT("SC_")},
		{USoundWave::StaticClass(), TEXT("S_")},
		{UTexture::StaticClass(), TEXT("T_")},
		{UTexture2D::StaticClass(), TEXT("T_")},
		{UUserWidget::StaticClass(), TEXT("UW_")},
		{UMorphTarget::StaticClass(), TEXT("MT_")},
		{USkeletalMeshComponent::StaticClass(), TEXT("SK_")}
	};

	UFUNCTION()
	void PrintToScreen(const FString Message, const FColor Color = FColor::White);

	UFUNCTION()
	void GiveFeedback(FString Method, uint32 Counter);

	UFUNCTION()
	bool IsPowerOfTwo(int32 Value);
};

#endif
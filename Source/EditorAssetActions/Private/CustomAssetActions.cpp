// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomAssetActions.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "Misc/ScopedSlowTask.h"

#if WITH_EDITOR
#pragma region RenameSelectAssets

void UCustomAssetActions::RenameSelectedAssets(const FString SearchPattern, const FString ReplacePattern,
	ESearchCase::Type SearchCase)
{
	if(SearchPattern.IsEmpty() || SearchPattern.Equals(ReplacePattern, SearchCase))
	{
		return;
	}

	TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();

	uint32 Counter = 0;

	FScopedSlowTask SlowTask(SelectedAssets.Num(), FText::FromString("Renaming assets..."));
	SlowTask.MakeDialog(true);
	for (auto SelectedAsset : SelectedAssets)
	{
		if (SlowTask.ShouldCancel())
		{
			break;
		}
		SlowTask.EnterProgressFrame(1.0f, FText::FromString(SelectedAsset->GetName()));
		FString AssetName = SelectedAsset->GetName();
		if(AssetName.Contains(SearchPattern, SearchCase))
		{
			FString NewName = AssetName.Replace(*SearchPattern, *ReplacePattern, SearchCase);
			UEditorUtilityLibrary::RenameAsset(SelectedAsset, NewName);
			Counter++;
		}
	}
	GiveFeedback(TEXT("Renamed %d assets"), Counter);
}

#pragma endregion

#pragma region AddAssetPrefix

void UCustomAssetActions::AddAssetPrefix()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	bool bRunning = true;
	uint32 Counter = 0;

	FScopedSlowTask SlowTask(SelectedObjects.Num()+1, FText::FromString(TEXT("Adding Prefix to Assets")));
	SlowTask.MakeDialog(true);
	
	for(UObject* SelectedObject : SelectedObjects)
	{
		if (SlowTask.ShouldCancel())
		{
			break;
		}
		SlowTask.EnterProgressFrame(1.0f, FText::FromString(TEXT("Adding Prefix to Assets")));
		
		if(ensure(SelectedObject))
		{
			const FString* Prefix = ClassToPrefixMap.Find(SelectedObject->GetClass());
			if(ensure(Prefix) && !Prefix->IsEmpty())
			{
				FString NewName = *Prefix + SelectedObject->GetName();
				UEditorUtilityLibrary::RenameAsset(SelectedObject, NewName);
				Counter++;
			}
			else
			{
				PrintToScreen("No prefix found for class " + SelectedObject->GetClass()->GetName(), FColor::Red);
			}
		}
	}
	GiveFeedback("Added prefix to ", Counter);
}

#pragma endregion

#pragma region RemoveUnusedAssets

void UCustomAssetActions::RemoveUnusedAssets(bool bDeleteImmediately)
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	TArray<UObject*> UnusedAssets = TArray<UObject*>();

	FScopedSlowTask SlowTask(SelectedObjects.Num(), FText::FromString(TEXT("Removing Unused Assets")));
	SlowTask.MakeDialog(true);
	for (UObject* SelectedObject : SelectedObjects)
	{
		if (ensure(SelectedObject))
		{
			if (UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedObject->GetPathName(), true).Num() <= 0)
			{
				UnusedAssets.Add(SelectedObject);
			}
		}
	}

	uint32 Counter = 0;

	for (auto SelectedObject : UnusedAssets)
	{
		if (SlowTask.ShouldCancel())
		{
			break;
		}
		SlowTask.EnterProgressFrame(1.0f, FText::FromString(TEXT("Removing Unused Asset: " + SelectedObject->GetName())));
		
		if (bDeleteImmediately)
		{
			if (UEditorAssetLibrary::DeleteLoadedAsset(SelectedObject))
			{
				++Counter;
			}
			else
			{
				PrintToScreen("Error deleting asset " + SelectedObject->GetName(), FColor::Red);
			}
		}
		else
		{
			FString NewPath = FPaths::Combine(TEXT("/Game"), TEXT("Bin"), SelectedObject->GetName());
			if (UEditorAssetLibrary::RenameLoadedAsset(SelectedObject, NewPath))
			{
				++Counter;
			}
			else
			{
				PrintToScreen("Error moving asset " + SelectedObject->GetName(), FColor::Red);
			}
		}
	}
	GiveFeedback(bDeleteImmediately ? "Deleted " : "Moved to bin:", Counter);
}

#pragma endregion 

#pragma region PowerOfTwo

void UCustomAssetActions::CheckIfTextureIsValidPowerOf2()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();

	uint32 Counter = 0;

	FScopedSlowTask SlowTask(SelectedObjects.Num(), FText::FromString(TEXT("Checking if textures are power of 2")));
	SlowTask.MakeDialog(true);
	
	for (auto SelectedObject : SelectedObjects)
	{
		if (SlowTask.ShouldCancel())
		{
			break;
		}
		SlowTask.EnterProgressFrame(1.0f, FText::FromString(TEXT("Checking if texture is power of 2: " + SelectedObject->GetName())));
		
		if (ensure(SelectedObject))
		{
			UTexture* Texture = dynamic_cast<UTexture*>(SelectedObject);
			if (ensure(Texture))
			{
				int32 Width = static_cast<int32>(Texture->GetSurfaceWidth());
				int32 Height = static_cast<int32>(Texture->GetSurfaceHeight());

				if(!IsPowerOfTwo(Width) || !IsPowerOfTwo(Height))
				{
					PrintToScreen("Texture " + Texture->GetName() + " is not a power of 2", FColor::Red);
				}
				{
					++Counter;
				}
			}
			else
			{
				PrintToScreen(SelectedObject->GetPathName() + " is not a texture", FColor::Red);
			}
		}
	}
	GiveFeedback("Power of 2 check complete. ", Counter);
}

#pragma endregion

#pragma region FolderCleanup

void UCustomAssetActions::CleanUpFolder(FString ParentFolder)
{
	// Checks if the folder is in the project content folder
	if (!ParentFolder.StartsWith(TEXT("/Game")))
	{
		ParentFolder = FPaths::Combine(TEXT("/Game"), ParentFolder);
	}

	TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();

	uint32 Counter = 0;

	FScopedSlowTask SlowTask(SelectedAssets.Num(), FText::FromString(TEXT("Cleaning up folder")));
	SlowTask.MakeDialog(true);	
	for (auto SelectedAsset : SelectedAssets)
	{
		if (SlowTask.ShouldCancel())
		{
			break;
		}
		SlowTask.EnterProgressFrame(1.0f, FText::FromString(TEXT("Cleaning up folder")));
		
		if (ensure(SelectedAsset))
		{
			FString NewPath = FPaths::Combine(ParentFolder, SelectedAsset->GetClass()->GetName(), SelectedAsset->GetName());

			if(UEditorAssetLibrary::RenameLoadedAsset(SelectedAsset, NewPath))
			{
				Counter++;
			}
			else
			{
				PrintToScreen("Failed to move asset " + SelectedAsset->GetName(), FColor::Red);
			}
		}
	}
	GiveFeedback("Moved ", Counter);
}

#pragma endregion

#pragma region DuplicateAssets

void UCustomAssetActions::DuplicateSelectedAsset(uint32 NumberOfDuplicates, bool bSave)
{
	double start = FPlatformTime::Seconds();
	TArray<FAssetData> AssetDataArray = UEditorUtilityLibrary::GetSelectedAssetData();

	uint32 Counter = 0;

	FScopedSlowTask SlowTask(AssetDataArray.Num() * NumberOfDuplicates, FText::FromString(TEXT("Duplicating assets")));
	SlowTask.MakeDialog(true);
	
	for (auto AssetData : AssetDataArray)
	{
		if (SlowTask.ShouldCancel())
		{
			break;
		}

		SlowTask.EnterProgressFrame(NumberOfDuplicates, FText::FromString(TEXT("Duplicating asset: " + AssetData.AssetName.ToString())));
		
		for (uint32 i = 0; i < NumberOfDuplicates; i++)
		{
			FString NewFileName = AssetData.AssetName.ToString().AppendChar('_').Append(FString::FromInt(i));
			FString NewPath = FPaths::Combine(AssetData.PackagePath.ToString(), NewFileName);
			
			if (ensure(UEditorAssetLibrary::DuplicateAsset(AssetData.GetSoftObjectPath().ToString(), NewPath)))
			{
				Counter++;
				if (bSave)
				{
					UEditorAssetLibrary::SaveAsset(NewPath, false);
				}
			}
		}
	}
	GiveFeedback("Duplicated ", Counter);

	double end = FPlatformTime::Seconds();
	PrintToScreen("Time taken: " + FString::SanitizeFloat(end - start));
}

#pragma endregion 

#pragma region Utilies
void UCustomAssetActions::PrintToScreen(const FString Message, const FColor Color)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, Color, Message);
}

void UCustomAssetActions::GiveFeedback(FString Method, uint32 Counter)
{
	FString Message = FString("No Matching Assets Found");
	FColor Color = Counter < 0 ? FColor::Red : FColor::Green;
	if(Counter > 0)
	{
		Message = Method.AppendChar(' ');
		Message.AppendInt(Counter);
		Message.Append(Counter == 1 ? FString(" Asset") : FString(" Assets"));
	}
	PrintToScreen(Message, Color);
}

bool UCustomAssetActions::IsPowerOfTwo(int32 Value)
{
	if (Value == 0)
	{
		return false;
	}
	return (Value & (Value - 1)) == 0;
}
#pragma endregion

#endif
// Fill out your copyright notice in the Description page of Project Settings.


#include "EditorUtilityWidget/CombineTextures.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialFactoryNew.h"
#include <Materials\MaterialExpressionTextureSample.h>
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget.h"
#include "ImageUtils.h" //for saving images
#include "Kismet/KismetRenderingLibrary.h"
#include "MaterialDomain.h"
#include "ObjectTools.h"


#pragma region TextureCombineCore

void UCombineTextures::CreateCombinedTextureFromSelectedTextures()
{

	//DebugHeader::Print(TEXT("Creating Material from textures"), FColor::Cyan);
	if (bCustomTextureName)
	{

		if (TextureName.IsEmpty() || TextureName.Equals(TEXT("M_")))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a valid name"));
			return;
		}

	}

	//These are our local variables that make sure that we have a counter on each of the Assets, paths, arrays we use 
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*> SelectedTexturesArray;
	FString SelectedTextureFolderPath;
	uint32 PinsConnectedCounter = 0;
	uint32 MaterialExpressionEditorXValue = 200;
	uint32 MaterialExpressionEditorYValue = 200;
	ETextureRenderTargetFormat NameOfPNGQuality = RTF_RGBA8;
	//UTextureRenderTarget2D* RenderTarget = CreateRenderTarget(GEditor->GetEditorWorldContext().World(), 1024, 1024, EPixelFormat::PF_B8G8R8A8);
	UTextureRenderTarget2D* RenderTarget = CreateRenderTarget(GEditor->GetEditorWorldContext().World(), FName("EditorRenderTarget"), 1024, 1024, NameOfPNGQuality);
	

	//You might not need this section. This is litteraly just a section that checks how many FAssets you have selected
	//And it checks for the name, and resets the name of the name of the material
	if (!ProcessSelectedData(SelectedAssetsData, SelectedTexturesArray, SelectedTextureFolderPath)) { TextureName = TEXT("T_");  return; };


	if (CheckIsNameUsed(SelectedTextureFolderPath, TextureName)) { TextureName = TEXT("T_");  return; };

	//Here we are calling our material create function to create our Material assets and storing it in CreatedMaterial
	UMaterial* CreatedMaterial = CreateMaterialAsset(TextureName, SelectedTextureFolderPath);


	if (!CreatedMaterial)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Failed to create material"));
		return;
	}


	for (UTexture2D* SelectedTexture : SelectedTexturesArray)
	{
		if (!SelectedTexture) continue;

		//Switching between the enum
		switch (ChannelPackingType)
		{

		case E_ChannelPackingType::ECPT_CombineTextures:

			CreateCombinedTextureMaterial(CreatedMaterial, SelectedTexturesArray, PinsConnectedCounter, RenderTarget, SelectedTextureFolderPath);

			break;

		case E_ChannelPackingType::ECPT_MAX:
			
			break;



		default:
			break;
		}

	}


	//Change Quality of PNG
	for (UTexture2D* SelectedTexture : SelectedTexturesArray)
	{
		if (!SelectedTexture) continue;

		switch (PNG_Quality)
		{

		case E_TextureRenderTargetQuality::RTF_R8:
			NameOfPNGQuality = RTF_R8;
			break;
		case E_TextureRenderTargetQuality::RTF_RG8:
			NameOfPNGQuality = RTF_RG8;
			break;
		case E_TextureRenderTargetQuality::RTF_RGBA8:
			NameOfPNGQuality = RTF_RGBA8;
			break;
		case E_TextureRenderTargetQuality::RTF_RGBA8_SRGB:
			NameOfPNGQuality = RTF_RGBA8_SRGB;
			break;
		case E_TextureRenderTargetQuality::RTF_R16f:
			NameOfPNGQuality = RTF_R16f;
			break;
		case E_TextureRenderTargetQuality::RTF_RG16f:
			NameOfPNGQuality = RTF_RG16f;
			break;
		case E_TextureRenderTargetQuality::RTF_RGBA16f:
			NameOfPNGQuality = RTF_RGBA16f;
			break;
		case E_TextureRenderTargetQuality::RTF_R32f:
			NameOfPNGQuality = RTF_R32f;
			break;
		case E_TextureRenderTargetQuality::RTF_RG32f:
			NameOfPNGQuality = RTF_RG32f;
			break;
		case E_TextureRenderTargetQuality::RTF_RGBA32f:
			NameOfPNGQuality = RTF_RGBA32f;
			break;
		case E_TextureRenderTargetQuality::RTF_RGB10A2:
			NameOfPNGQuality = RTF_RGB10A2;
			break;
		default:
			NameOfPNGQuality = RTF_RGBA8;
			break;
		}

	}


	if (PinsConnectedCounter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully connected ") + FString::FromInt(PinsConnectedCounter) + (TEXT(" pins")));
	}

	//DebugHeader::Print(SelectedTextureFolderPath, FColor::Cyan);

	//set name back to default when creating a material
	TextureName = TEXT("T_");

}


bool UCombineTextures::ProcessSelectedData(const TArray<FAssetData>& SelectedDataToProccess,
	TArray<UTexture2D*>& OutSelectedTexturesArray, FString& OutSelectedTexturePackagePath)
{
	if (SelectedDataToProccess.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No texture selected"));
		return false;
	}

	if(SelectedDataToProccess.Num() > 3)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Too many textures selected, only select 3 to combine"));
		return false; 
	
	}

	bool bMaterialNameSet = false;

	for (const FAssetData& SelectedData : SelectedDataToProccess)
	{
		UObject* SelectedAsset = SelectedData.GetAsset();

		if (!SelectedAsset) continue;

		UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedAsset);

		if (!SelectedTexture)
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please select only textures\n") +
				SelectedAsset->GetName() + TEXT(" is not a texture"));

			return false;
		}

		OutSelectedTexturesArray.Add(SelectedTexture);

		if (OutSelectedTexturePackagePath.IsEmpty())
		{
			OutSelectedTexturePackagePath = SelectedData.PackagePath.ToString();
		}

		if (!bCustomTextureName && !bMaterialNameSet)
		{
			TextureName = SelectedAsset->GetName();
			TextureName.RemoveFromStart(TEXT("T_"));
			TextureName.InsertAt(0, TEXT("M_"));

			bMaterialNameSet = true;
		}
	}

	return true;
}



//return true if the material name is the same as already existing material
bool UCombineTextures::CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck)
{
	TArray <FString> ExistingAssetsPath = UEditorAssetLibrary::ListAssets(FolderPathToCheck, false);

	for (const FString& ExistingAssetPath : ExistingAssetsPath)
	{
		const FString ExistingAssetName = FPaths::GetBaseFilename(ExistingAssetPath);

		if (ExistingAssetName.Equals(MaterialNameToCheck))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, MaterialNameToCheck + TEXT(" is already used by asset"));

			return true;

		}

	}

	return false;

}

//Function for creating a material asset
UMaterial* UCombineTextures::CreateMaterialAsset(const FString& NameOfTheMaterial, const FString& PathToPutMaterial)
{
	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();

	//#include "Factories/MaterialFactoryNew.h" for the fourth input
	UObject* CreatedObject =
		AssetToolsModule.Get().CreateAsset(NameOfTheMaterial, PathToPutMaterial, UMaterial::StaticClass(), MaterialFactory);

	return Cast<UMaterial>(CreatedObject);
}

//Function for what should be in the material asset
void UCombineTextures::CreateCombinedTextureMaterial(UMaterial* CreatedMaterial, TArray<UTexture2D*> SelectedTexturesArray, uint32 PinsConnectedCounter, UTextureRenderTarget2D* RenderTarget, FString FilePath)
{
	
	
	if (!CreatedMaterial || SelectedTexturesArray.Num() == 0) return;

	
	// ----------------------
	// 1. Create Texture Sample Nodes for each texture
	// ----------------------
	for (const FString& TextureArrayName : TextureArray)
	{
			


		for (UTexture2D* SelectedTextures : SelectedTexturesArray)
		{


			if (SelectedTextures->GetName().Contains(TextureArrayName))
			{
				// ----------------------
				// Declare our struct 
				// ----------------------
				FMaterialExpressions Expressions{};


				// ----------------------
				// Create Sample texture node and assign each selection to each of the sample textures 
				// ----------------------
				Expressions.TextureSampleR = NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);
				Expressions.TextureSampleR->Texture = SelectedTexturesArray.IsValidIndex(0) ? SelectedTexturesArray[0] : nullptr;
				Expressions.TextureSampleR->MaterialExpressionEditorX = 100;
				Expressions.TextureSampleR->MaterialExpressionEditorY = 100;
				//Expressions.TextureSampleR->Desc = TEXT("Texture_") + FString::FromInt(PinsConnectedCounter); //Does not work yet
				CreatedMaterial->GetExpressionCollection().AddExpression(Expressions.TextureSampleR);


				Expressions.TextureSampleG = NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);
				Expressions.TextureSampleG->Texture = SelectedTexturesArray.IsValidIndex(1) ? SelectedTexturesArray[1] : nullptr;
				Expressions.TextureSampleG->MaterialExpressionEditorX = 100;
				Expressions.TextureSampleG->MaterialExpressionEditorY = 300;
				CreatedMaterial->GetExpressionCollection().AddExpression(Expressions.TextureSampleG);

				Expressions.TextureSampleB = NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);
				Expressions.TextureSampleB->Texture = SelectedTexturesArray.IsValidIndex(2) ? SelectedTexturesArray[2] : nullptr;
				Expressions.TextureSampleB->MaterialExpressionEditorX = 100;
				Expressions.TextureSampleB->MaterialExpressionEditorY = 500;
				CreatedMaterial->GetExpressionCollection().AddExpression(Expressions.TextureSampleB);

				// ----------------------
				// Extract R, G, B components from each texture
				// ----------------------
				UMaterialExpressionComponentMask* MaskR = NewObject<UMaterialExpressionComponentMask>(CreatedMaterial);
				MaskR->Input.Expression = Expressions.TextureSampleR;
				MaskR->R = 1;  // Extract R channel
				MaskR->MaterialExpressionEditorX = 300;
				MaskR->MaterialExpressionEditorY = 100;

				UMaterialExpressionComponentMask* MaskG = NewObject<UMaterialExpressionComponentMask>(CreatedMaterial);
				MaskG->Input.Expression = Expressions.TextureSampleG;
				MaskG->G = 1;  // Extract G channel
				MaskG->MaterialExpressionEditorX = 300;
				MaskG->MaterialExpressionEditorY = 300;

				UMaterialExpressionComponentMask* MaskB = NewObject<UMaterialExpressionComponentMask>(CreatedMaterial);
				MaskB->Input.Expression = Expressions.TextureSampleB;
				MaskB->B = 1;  // Extract B channel
				MaskB->MaterialExpressionEditorX = 300;
				MaskB->MaterialExpressionEditorY = 500;

				// ----------------------
				// Append the extracted components to form a new texture
				// ----------------------
				UMaterialExpressionAppendVector* AppendRG = NewObject<UMaterialExpressionAppendVector>(CreatedMaterial);
				AppendRG->A.Expression = MaskR;  // R from the first texture
				AppendRG->B.Expression = MaskG;  // G from the second texture
				AppendRG->MaterialExpressionEditorX = 500;
				AppendRG->MaterialExpressionEditorY = 200;

				UMaterialExpressionAppendVector* AppendRGB = NewObject<UMaterialExpressionAppendVector>(CreatedMaterial);
				AppendRGB->A.Expression = AppendRG;  // RG combined
				AppendRGB->B.Expression = MaskB;     // B from the third texture
				AppendRGB->MaterialExpressionEditorX = 700;
				AppendRGB->MaterialExpressionEditorY = 300;

				// ----------------------
				// Connect the final Append node to the material's BaseColor (or other property)
				// ----------------------
				CreatedMaterial->MaterialDomain = MD_UI;
				CreatedMaterial->SetShadingModel(MSM_Unlit);
				CreatedMaterial->GetExpressionInputForProperty(MP_EmissiveColor)->Connect(0, AppendRGB);

				// ----------------------
				// Add all expressions to the material
				// ----------------------
				CreatedMaterial->GetExpressionCollection().AddExpression(MaskR);
				CreatedMaterial->GetExpressionCollection().AddExpression(MaskG);
				CreatedMaterial->GetExpressionCollection().AddExpression(MaskB);
				CreatedMaterial->GetExpressionCollection().AddExpression(AppendRG);
				CreatedMaterial->GetExpressionCollection().AddExpression(AppendRGB);

				// ----------------------
				// Recompile and update the material
				// ----------------------
				CreatedMaterial->PostEditChange();
				CreatedMaterial->MarkPackageDirty();

			}

		}
	}
	



	// ----------------------
	// Bool statement to check if the material has been created and rendered to render target then delete it
	// ----------------------
	bool bRenderSuccess = false;
	bool bSaveSuccess = false;

	// Render the material to the render target
	if (RenderMaterialToEdtiroRenderTarget(RenderTarget, CreatedMaterial))
	{
		bRenderSuccess = true;
	}

	// Save the render target to a file
	if (SaveRenderTargetToFile(RenderTarget, FilePath))
	{
		bSaveSuccess = true;
	}

	// Check if both operations succeeded
	if (bRenderSuccess && bSaveSuccess)
	{
		// If both functions ran successfully, delete the material
		ObjectTools::DeleteSingleObject(CreatedMaterial);
	}

}


#pragma endregion



#pragma region RenderTarget

//Creating render target 
//Render material to render target 
//Save the render target to a file 

//For creating render target
UTextureRenderTarget2D* UCombineTextures::CreateRenderTarget(UObject* Outer, FName Name, int32 Width, int32 Height, ETextureRenderTargetFormat Format)
{
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(Outer, Name, RF_Transient);

	RenderTarget->RenderTargetFormat = Format;
	RenderTarget->InitAutoFormat(Width, Height);
	RenderTarget->UpdateResourceImmediate(true);

	return RenderTarget;

}

//for saving the material to the render target
bool UCombineTextures::RenderMaterialToEdtiroRenderTarget(UTextureRenderTarget2D* RenderTarget, UMaterialInterface* Material)
{
	if(!RenderTarget || !Material)
	{
		UE_LOG(LogTemp, Warning, TEXT("RenderTarget or Material is null"));
		return false;
	
	}

	//Get the editor world
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if(EditorWorld)
	{
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(EditorWorld, RenderTarget, Material);
		return true;
	
	}

	return false;
}



//for exporting/importing the PNG file from render target
bool UCombineTextures::SaveRenderTargetToFile(UTextureRenderTarget2D* RenderTarget, const FString& FilePath)
{
	if (!RenderTarget) return false;

	FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();

	TArray<FColor> OutBMP; 
	FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);

	RenderTargetResource->ReadPixels(OutBMP, ReadPixelFlags);

	//// Set the alpha value of each pixel to 0 (fully transparent)
	//for (FColor& Pixel : OutBMP)
	//{
	//	Pixel.A = 0;  // Alpha value: 0 (fully transparent)
	//}

	//for (int32 i = 0; i < OutBMP.Num(); ++i)
	//{
	//	FColor& Pixel = OutBMP[i];
	//	Pixel = FColor(Pixel.R, Pixel.G, Pixel.B, Pixel.A = 0U);  // Set alpha to 0 (fully transparent)
	//}


	FIntPoint DestSize(RenderTarget->SizeX, RenderTarget->SizeY);
	TArray64<uint8> CompressedBitmap;
	FImageUtils::PNGCompressImageArray(DestSize.X, DestSize.Y, OutBMP, CompressedBitmap);
	
	FString FinalFilePath = FilePath; //might not need

	//FString FileName = "MyTexture.png";
	FString FileName = TextureName + ".png";
	FString Directory = FPaths::ProjectContentDir() + "Textures/";
	FString FullFilePath = Directory + FileName;


	if (FFileHelper::SaveArrayToFile(CompressedBitmap, *FullFilePath)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Rendertarget saved as PNG to: %s"), *FullFilePath);
		return true;
	}
	
	return false;

}


#pragma endregion


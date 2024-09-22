

// ----------------------
// Code made by Alex Nygaard Andersen
// ----------------------


#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include <Materials\MaterialExpressionTextureSample.h>
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "CombineTextures.generated.h"


//Bugs to fix: 
//Problem with array for names 
//make more error handlers 


//Create enum for choosing between channel packing
	UENUM(BlueprintType)
		enum class E_ChannelPackingType : uint8
	{
		ECPT_CombineTextures UMETA(DisplayName = "CombineTextures"),

		ECPT_MAX UMETA(DisplayName = "DefaultMAX")

	};

	////Create Enum for choosing between Quality of PNG
	UENUM(BlueprintType)
		enum class E_TextureRenderTargetQuality : uint8
	{
		/** R channel, 8 bit per channel fixed point, range [0, 1]. */
		RTF_R8  UMETA(DisplayName = "RTF_R8"),
		/** RG channels, 8 bit per channel fixed point, range [0, 1]. */
		RTF_RG8  UMETA(DisplayName = "RTF_RG8"),
		/** RGBA channels, 8 bit per channel fixed point, range [0, 1]. */
		RTF_RGBA8  UMETA(DisplayName = "RTF_RGBA8"),
		/** RGBA channels, 8 bit per channel fixed point, range [0, 1]. RGB is encoded with sRGB gamma curve. A is always stored as linear. */
		RTF_RGBA8_SRGB  UMETA(DisplayName = "RTF_RGBA8_SRGB"),
		/** R channel, 16 bit per channel floating point, range [-65504, 65504] */
		RTF_R16f  UMETA(DisplayName = "RTF_R16f"),
		/** RG channels, 16 bit per channel floating point, range [-65504, 65504] */
		RTF_RG16f  UMETA(DisplayName = "RTF_RG16f"),
		/** RGBA channels, 16 bit per channel floating point, range [-65504, 65504] */
		RTF_RGBA16f  UMETA(DisplayName = "RTF_RGBA16f"),
		/** R channel, 32 bit per channel floating point, range [-3.402823 x 10^38, 3.402823 x 10^38] */
		RTF_R32f  UMETA(DisplayName = "RTF_R32f"),
		/** RG channels, 32 bit per channel floating point, range [-3.402823 x 10^38, 3.402823 x 10^38] */
		RTF_RG32f UMETA(DisplayName = "RTF_RG32f"),
		/** RGBA channels, 32 bit per channel floating point, range [-3.402823 x 10^38, 3.402823 x 10^38] */
		RTF_RGBA32f  UMETA(DisplayName = "RTF_RGBA32f"),
		/** RGBA channels, 10 bit per channel fixed point and 2 bit of alpha */
		RTF_RGB10A2  UMETA(DisplayName = "RTF_RGB10A2 "),
	};


	/**
	 *
	 */
	UCLASS()
		class MAGICTOOL_API UCombineTextures : public UEditorUtilityWidget
	{
		GENERATED_BODY()


	public:

#pragma region TextureCombineCore

		UFUNCTION(BlueprintCallable, Category = "CreateCombinedTextureFromSelectedTextures")
		void CreateCombinedTextureFromSelectedTextures();

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateCombinedTextureFromSelectedTextures")
		bool bCustomTextureName = true;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateCombinedTextureFromSelectedTextures", meta = (EditCondition = "bCustomTextureName"))
		FString TextureName = TEXT("T_");

		//Creating the enum
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateCombinedTextureFromSelectedTextures")
		E_ChannelPackingType ChannelPackingType = E_ChannelPackingType::ECPT_CombineTextures;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateCombinedTextureFromSelectedTextures")
		E_TextureRenderTargetQuality PNG_Quality = E_TextureRenderTargetQuality::RTF_RGBA8;

#pragma endregion


#pragma region SupportedTexturesNames

		//Array for Textures
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
		TArray<FString> TextureArray =
		{
			TEXT("_T"),
			TEXT("T_"),
			TEXT("_Texture"),
			TEXT("Texture_")

		};

#pragma endregion 


	private:

#pragma region TextureCombineMethods

		//Methods for processing selection in editor, name and creating assets
		bool ProcessSelectedData(const TArray<FAssetData>& SelectedDataToProcess, TArray<UTexture2D*>& OutSelectedTexturesArray, FString& OutSelectedTexturePackagePath);
		bool CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck);

		//Creating Material
		UMaterial* CreateMaterialAsset(const FString& NameOfTheMaterial, const FString& PathToPutMaterial);
		void CreateCombinedTextureMaterial(UMaterial* CreatedMaterial, TArray<UTexture2D*> SelectedTexturesArray, uint32 PinsConnectedCounter, UTextureRenderTarget2D* RenderTarget, FString FilePath);

		//Render target
		UTextureRenderTarget2D* CreateRenderTarget(UObject* Outer, FName Name, int32 Width, int32 Height, ETextureRenderTargetFormat Format = RTF_RGBA8);
		bool RenderMaterialToEdtiroRenderTarget(UTextureRenderTarget2D* RenderTarget, UMaterialInterface* Material);
		bool SaveRenderTargetToFile(UTextureRenderTarget2D* RenderTarget, const FString& FilePath);

		//Struct for storing various variables to use in CreateCombinedTextureMaterial
		struct FMaterialExpressions
		{
			UMaterialExpressionTextureSample* TextureSample; 
			UMaterialExpressionTextureSample* TextureSampleR;
			UMaterialExpressionTextureSample* TextureSampleG;
			UMaterialExpressionTextureSample* TextureSampleB;
			UMaterialExpressionComponentMask* MaskR;
			UMaterialExpressionComponentMask* MaskG;
			UMaterialExpressionComponentMask* MaskB;
			UMaterialExpressionAppendVector* AppendRG;
			UMaterialExpressionAppendVector* AppendRGB;
		};

#pragma endregion


	};

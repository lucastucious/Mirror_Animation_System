// Copyright 2017-2021 Rexocrates. All Rights Reserved.


#include "MASFunctionLibrary.h"

#include "MirrorTable.h"

#include "Animation/AnimSequence.h"

#include "Misc/PackageName.h"

#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#endif

#include "MASUtils.h"
#include "AnimationRuntime.h"

#define LOCTEXT_NAMESPACE "MASLibrary"

UMASFunctionLibrary::UMASFunctionLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMASFunctionLibrary::BulkMirrorEditorOnly(const TArray <UAnimSequence*> SourceAnims, const UMirrorTable* MirrorTable, TArray <UAnimSequence*>& OutNewAnims)
{
	if (SourceAnims.Num() == 0) return;
	if (MirrorTable == NULL) return;

#if WITH_EDITOR
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");

	for (int32 i = 0; i < SourceAnims.Num(); i++)
	{
		// Create the asset
		FString Name;
		FString PackageName;


		FString Suffix = TEXT("_Mirrored");

		AssetToolsModule.Get().CreateUniqueAssetName(SourceAnims[i]->GetOutermost()->GetName(), Suffix, /*out*/ PackageName, /*out*/ Name);
		const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);


		UObject* NewAsset = AssetToolsModule.Get().DuplicateAsset(Name, PackagePath, SourceAnims[i]);

		if (NewAsset != NULL)
		{
			UAnimSequence* MirrorAnimSequence = Cast<UAnimSequence>(NewAsset);
			CreateMirrorSequenceFromAnimSequence(MirrorAnimSequence, MirrorTable);

			OutNewAnims.Add(MirrorAnimSequence);

			// Notify asset registry of new asset
			FAssetRegistryModule::AssetCreated(MirrorAnimSequence);

			// Display notification so users can quickly access
			if (GIsEditor)
			{
				FNotificationInfo Info(FText::Format(LOCTEXT("AnimationMirrored", "Successfully Mirrored Animation"), FText::FromString(MirrorAnimSequence->GetName())));
				Info.ExpireDuration = 8.0f;
				Info.bUseLargeFont = false;
				//Info.Hyperlink = FSimpleDelegate::CreateLambda([=]() { FAssetEditorManager::Get().OpenEditorForAssets(TArray<UObject*>({ MirrorAnimSequence })); });
				Info.Hyperlink = FSimpleDelegate::CreateLambda([=]() { GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAssets(TArray<UObject*>({ MirrorAnimSequence })); });

				Info.HyperlinkText = FText::Format(LOCTEXT("OpenNewAnimationHyperlink", "Open {0}"), FText::FromString(MirrorAnimSequence->GetName()));
				TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
				if (Notification.IsValid())
				{
					Notification->SetCompletionState(SNotificationItem::CS_Success);
				}
			}
		}
	}
#endif
}

MIRRORANIMATIONSYSTEMDEV_API void UMASFunctionLibrary::BulkMirror_CS_EditorOnly(
	const TArray<UAnimSequence*> SourceAnims, 
	const TEnumAsByte<EAxis::Type> MirrorAxis, const FString Substring_A, const FString Substring_B, const bool Symmetrical, TArray<UAnimSequence*>& OutNewAnims)
{
	if (SourceAnims.Num() == 0) return;
	if (MirrorAxis == EAxis::None) return;

#if WITH_EDITOR
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");

	for (int32 i = 0; i < SourceAnims.Num(); i++)
	{
		// Create the asset
		FString Name;
		FString PackageName;


		FString Suffix = TEXT("_Mirrored");

		AssetToolsModule.Get().CreateUniqueAssetName(SourceAnims[i]->GetOutermost()->GetName(), Suffix, /*out*/ PackageName, /*out*/ Name);
		const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);


		UObject* NewAsset = AssetToolsModule.Get().DuplicateAsset(Name, PackagePath, SourceAnims[i]);

		if (NewAsset != NULL)
		{
			UAnimSequence* MirrorAnimSequence = Cast<UAnimSequence>(NewAsset);
			CreateMirrorSequenceFromAnimSequence_CS(MirrorAnimSequence, MirrorAxis, Substring_A, Substring_B, Symmetrical);

			OutNewAnims.Add(MirrorAnimSequence);

			// Notify asset registry of new asset
			FAssetRegistryModule::AssetCreated(MirrorAnimSequence);

			// Display notification so users can quickly access
			if (GIsEditor)
			{
				FNotificationInfo Info(FText::Format(LOCTEXT("AnimationMirrored", "Successfully Mirrored Animation"), FText::FromString(MirrorAnimSequence->GetName())));
				Info.ExpireDuration = 8.0f;
				Info.bUseLargeFont = false;
				//Info.Hyperlink = FSimpleDelegate::CreateLambda([=]() { FAssetEditorManager::Get().OpenEditorForAssets(TArray<UObject*>({ MirrorAnimSequence })); });
				Info.Hyperlink = FSimpleDelegate::CreateLambda([=]() { GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAssets(TArray<UObject*>({ MirrorAnimSequence })); });

				Info.HyperlinkText = FText::Format(LOCTEXT("OpenNewAnimationHyperlink", "Open {0}"), FText::FromString(MirrorAnimSequence->GetName()));
				TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
				if (Notification.IsValid())
				{
					Notification->SetCompletionState(SNotificationItem::CS_Success);
				}
			}
		}
	}
#endif
}

#if WITH_EDITOR

void UMASFunctionLibrary::CreateMirrorSequenceFromAnimSequence(UAnimSequence* MirrorSequence, const UMirrorTable* MirrorTable)
{
	//Check if it's valid
	if ((MirrorSequence != NULL) && (MirrorTable != NULL) && (MirrorSequence->GetSkeleton() != NULL))
	{
		//Make the duplicate that I will edit
		//UAnimSequence* MirrorSequence = FromAnimSequence;
		const auto& Skel = MirrorSequence->GetSkeleton()->GetReferenceSkeleton();

		int NumMirrorBones = MirrorTable->MirrorBones.Num();

		int NumFrames = MirrorSequence->GetNumberOfFrames();
		IAnimationDataController& MirrorSequenceController = MirrorSequence->GetController();
		TArray<FBoneAnimationTrack> SourceBoneAnimData = MirrorSequenceController.GetModel()->GetBoneAnimationTracks();
		/************* SourceRawAnimDatas should be replaced by SourceBoneAnimData ************/
		// TArray<FRawAnimSequenceTrack> SourceRawAnimDatas = MirrorSequence->GetRawAnimationData();
		
		TArray<FName> TrackNames;
		MirrorSequenceController.GetModel()->GetBoneTrackNames(TrackNames);

		for (int i = 0; i < NumMirrorBones; i++)
		{
			FMirrorBone CurrentBone = MirrorTable->MirrorBones[i];

			if (Skel.FindBoneIndex(CurrentBone.BoneName) == INDEX_NONE)
			{
				continue;
			}

			if (CurrentBone.IsTwinBone)
			{
				if (Skel.FindBoneIndex(CurrentBone.TwinBoneName) == INDEX_NONE)
				{
					continue;
				}

				int32 TrackIndex = TrackNames.IndexOfByKey(CurrentBone.BoneName);
				int32 TwinTrackIndex = TrackNames.IndexOfByKey(CurrentBone.TwinBoneName);

				if (TrackIndex == INDEX_NONE && TwinTrackIndex == INDEX_NONE)
				{
					continue;
				}

				TArray <FVector3f> MirrorPosKeys;
				TArray <FQuat4f> MirrorRotKeys;
				TArray <FVector3f> MirrorScaleKeys;

				TArray <FVector3f> TwinMirrorPosKeys;
				TArray <FQuat4f> TwinMirrorRotKeys;
				TArray <FVector3f> TwinMirrorScaleKeys;

				// Original Bone
				if (TrackIndex != INDEX_NONE)
				{
					auto& MirroredRawTrack = SourceBoneAnimData[TrackIndex];

					for (int u = 0; u < NumFrames; u++)
					{
						FTransform3f MirrorTM;

						bool bSetPos = false;
						bool bSetRot = false;
						bool bSetScale = false;

						if (MirroredRawTrack.InternalTrackData.PosKeys.IsValidIndex(u))
						{
							MirrorTM.SetTranslation(MirroredRawTrack.InternalTrackData.PosKeys[u]);
							bSetPos = true;
						}
						if (MirroredRawTrack.InternalTrackData.RotKeys.IsValidIndex(u))
						{
							MirrorTM.SetRotation(MirroredRawTrack.InternalTrackData.RotKeys[u]);
							bSetRot = true;
						}
						if (MirroredRawTrack.InternalTrackData.ScaleKeys.IsValidIndex(u))
						{
							MirrorTM.SetScale3D(MirroredRawTrack.InternalTrackData.ScaleKeys[u]);
							bSetScale = true;
						}

						MirrorTM.Mirror(CurrentBone.MirrorAxis, CurrentBone.FlipAxis);
						
						
						FRotator3f BoneNewRotation = MirrorTM.Rotator(); // quaternion to rotator

						BoneNewRotation.Yaw += CurrentBone.RotationOffset.Yaw;
						BoneNewRotation.Roll += CurrentBone.RotationOffset.Roll;
						BoneNewRotation.Pitch += CurrentBone.RotationOffset.Pitch;

						MirrorTM.SetRotation(BoneNewRotation.Quaternion());
						MirrorTM.SetScale3D(MirrorTM.GetScale3D().GetAbs());
						MirrorTM.NormalizeRotation();

						if (bSetPos)
						{
							MirrorPosKeys.Add(FVector3f(MirrorTM.GetTranslation()));
						}
						if (bSetRot)
						{
							MirrorRotKeys.Add(FQuat4f(MirrorTM.GetRotation()));
						}
						if (bSetScale)
						{
							MirrorScaleKeys.Add(FVector3f(MirrorTM.GetScale3D()));
						}
					}
				}
				else
				{
					auto RefTM = Skel.GetRefBonePose()[Skel.FindBoneIndex(CurrentBone.BoneName)];

					RefTM.Mirror(CurrentBone.MirrorAxis, CurrentBone.FlipAxis);

					FRotator BoneNewRotation = RefTM.Rotator();

					BoneNewRotation.Yaw += CurrentBone.RotationOffset.Yaw;
					BoneNewRotation.Roll += CurrentBone.RotationOffset.Roll;
					BoneNewRotation.Pitch += CurrentBone.RotationOffset.Pitch;

					RefTM.SetRotation(FQuat(BoneNewRotation));
					RefTM.SetScale3D(RefTM.GetScale3D().GetAbs());
					RefTM.NormalizeRotation();

					MirrorPosKeys.Add(FVector3f(RefTM.GetTranslation()));
					MirrorRotKeys.Add(FQuat4f(RefTM.GetRotation()));
				}

				// Twin Bone
				if (TwinTrackIndex != INDEX_NONE)
				{
					auto& TwinMirroredRawTrack = SourceBoneAnimData[TwinTrackIndex];

					for (int u = 0; u < NumFrames; u++)
					{
						FTransform3f TwinMirrorTM;

						bool TwinbSetPos = false;
						bool TwinbSetRot = false;
						bool TwinbSetScale = false;

						if (TwinMirroredRawTrack.InternalTrackData.PosKeys.IsValidIndex(u))
						{
							TwinMirrorTM.SetTranslation(TwinMirroredRawTrack.InternalTrackData.PosKeys[u]);
							TwinbSetPos = true;
						}
						if (TwinMirroredRawTrack.InternalTrackData.RotKeys.IsValidIndex(u))
						{
							TwinMirrorTM.SetRotation(TwinMirroredRawTrack.InternalTrackData.RotKeys[u]);
							TwinbSetRot = true;
						}
						if (TwinMirroredRawTrack.InternalTrackData.ScaleKeys.IsValidIndex(u))
						{
							TwinMirrorTM.SetScale3D(TwinMirroredRawTrack.InternalTrackData.ScaleKeys[u]);
							TwinbSetScale = true;
						}

						TwinMirrorTM.Mirror(CurrentBone.MirrorAxis, CurrentBone.FlipAxis);

						FRotator3f TwinBoneNewRotation = TwinMirrorTM.Rotator();

						TwinBoneNewRotation.Yaw += CurrentBone.RotationOffset.Yaw;
						TwinBoneNewRotation.Roll += CurrentBone.RotationOffset.Roll;
						TwinBoneNewRotation.Pitch += CurrentBone.RotationOffset.Pitch;

						TwinMirrorTM.SetRotation(TwinBoneNewRotation.Quaternion());
						TwinMirrorTM.SetScale3D(TwinMirrorTM.GetScale3D().GetAbs());
						TwinMirrorTM.NormalizeRotation();

						if (TwinbSetPos)
						{
							TwinMirrorPosKeys.Add(FVector3f(TwinMirrorTM.GetTranslation()));
						}
						if (TwinbSetRot)
						{
							TwinMirrorRotKeys.Add(FQuat4f(TwinMirrorTM.GetRotation()));
						}
						if (TwinbSetScale)
						{
							TwinMirrorScaleKeys.Add(FVector3f(TwinMirrorTM.GetScale3D()));
						}
					}
				}
				else
				{
					auto RefTM = Skel.GetRefBonePose()[Skel.FindBoneIndex(CurrentBone.TwinBoneName)];

					RefTM.Mirror(CurrentBone.MirrorAxis, CurrentBone.FlipAxis);

					FRotator TwinBoneNewRotation = RefTM.Rotator();

					TwinBoneNewRotation.Yaw += CurrentBone.RotationOffset.Yaw;
					TwinBoneNewRotation.Roll += CurrentBone.RotationOffset.Roll;
					TwinBoneNewRotation.Pitch += CurrentBone.RotationOffset.Pitch;

					RefTM.SetRotation(FQuat(TwinBoneNewRotation));
					RefTM.SetScale3D(RefTM.GetScale3D().GetAbs());
					RefTM.NormalizeRotation();

					TwinMirrorPosKeys.Add(FVector3f(RefTM.GetTranslation()));
					TwinMirrorRotKeys.Add(FQuat4f(RefTM.GetRotation()));
				}

				// Original Bone -> Twin Bone
				{
					FRawAnimSequenceTrack NewTrack;

					NewTrack.PosKeys = CurrentBone.MirrorTranslation ? MirrorPosKeys : TwinMirrorPosKeys;
					NewTrack.RotKeys = MirrorRotKeys;
					NewTrack.ScaleKeys = MirrorScaleKeys;

					//MirrorSequence->AddNewRawTrack(CurrentBone.TwinBoneName, &NewTrack);
					MirrorSequenceController.AddBoneTrack(CurrentBone.TwinBoneName);
					MirrorSequenceController.SetBoneTrackKeys(CurrentBone.BoneName,NewTrack.PosKeys,NewTrack.RotKeys,NewTrack.ScaleKeys);
				}

				// Twin Bone -> Original Bone
				{
					FRawAnimSequenceTrack NewTrack;

					NewTrack.PosKeys = CurrentBone.MirrorTranslation ? TwinMirrorPosKeys : MirrorPosKeys;
					NewTrack.RotKeys = TwinMirrorRotKeys;
					NewTrack.ScaleKeys = TwinMirrorScaleKeys;

					MirrorSequenceController.AddBoneTrack(CurrentBone.BoneName);
					MirrorSequenceController.SetBoneTrackKeys(CurrentBone.BoneName,NewTrack.PosKeys,NewTrack.RotKeys,NewTrack.ScaleKeys);
				}
			}
			else
			{
				int32 TrackIndex = TrackNames.IndexOfByKey(CurrentBone.BoneName);

				if (TrackIndex == INDEX_NONE)
				{
					continue;
				}

				FBoneAnimationTrack MirroredRawTrack = SourceBoneAnimData[TrackIndex];

				//MirrorAllFrames
				TArray <FVector3f> MirrorPosKeys;
				TArray <FQuat4f> MirrorRotKeys;
				TArray <FVector3f> MirrorScaleKeys;

				for (int u = 0; u < NumFrames; u++)
				{
					//Mirror Transform
					FTransform3f MirrorTM;

					bool bSetPos = false;
					bool bSetRot = false;
					bool bSetScale = false;

					if (MirroredRawTrack.InternalTrackData.PosKeys.IsValidIndex(u))
					{
						MirrorTM.SetTranslation(MirroredRawTrack.InternalTrackData.PosKeys[u]);
						bSetPos = true;
					}
					if (MirroredRawTrack.InternalTrackData.RotKeys.IsValidIndex(u))
					{
						MirrorTM.SetRotation(MirroredRawTrack.InternalTrackData.RotKeys[u]);
						bSetRot = true;
					}
					if (MirroredRawTrack.InternalTrackData.ScaleKeys.IsValidIndex(u))
					{
						MirrorTM.SetScale3D(MirroredRawTrack.InternalTrackData.ScaleKeys[u]);
						bSetScale = true;
					}

					MirrorTM.Mirror(CurrentBone.MirrorAxis, CurrentBone.FlipAxis);

					FRotator3f BoneNewRotation = MirrorTM.Rotator();

					BoneNewRotation.Yaw += CurrentBone.RotationOffset.Yaw;
					BoneNewRotation.Roll += CurrentBone.RotationOffset.Roll;
					BoneNewRotation.Pitch += CurrentBone.RotationOffset.Pitch;

					MirrorTM.SetRotation(BoneNewRotation.Quaternion());
					//MirrorTM.NormalizeRotation();
					MirrorTM.SetScale3D(MirrorTM.GetScale3D().GetAbs());

					MirrorTM.NormalizeRotation();

					//Setting it up Main
					if (bSetPos)
					{
						MirrorPosKeys.Add(MirrorTM.GetTranslation());
					}
					if (bSetRot)
					{
						MirrorRotKeys.Add(MirrorTM.GetRotation());
					}
					if (bSetScale)
					{
						MirrorScaleKeys.Add(MirrorTM.GetScale3D());
					}

					/////////////////////////////////
				}

				MirroredRawTrack.InternalTrackData.PosKeys = MirrorPosKeys;
				MirroredRawTrack.InternalTrackData.RotKeys = MirrorRotKeys;
				MirroredRawTrack.InternalTrackData.ScaleKeys = MirrorScaleKeys;

				//Finally Setting it in the AnimSequence

				//MirrorSequenceController.AddBoneTrack(CurrentBone.BoneName, &MirroredRawTrack);
				MirrorSequenceController.AddBoneCurve(CurrentBone.BoneName);
				MirrorSequenceController.SetBoneTrackKeys(CurrentBone.BoneName,MirroredRawTrack.InternalTrackData.PosKeys,MirroredRawTrack.InternalTrackData.RotKeys,MirroredRawTrack.InternalTrackData.ScaleKeys);
			}
		}
		//MirrorSequence->ClearBakedTransformData();
		MirrorSequenceController.RemoveAllCurvesOfType(ERawCurveTrackTypes::RCT_Transform);
		MirrorSequence->MarkPackageDirty();
	}
}


static FTransform3f GetAnimBoneTM(UAnimSequence* AnimSeq, const int32 BoneTreeIndex, const float AnimTime)
{
	USkeleton* Skeleton = AnimSeq->GetSkeleton();
	//int32 BoneTreeIndex = Skeleton->GetSkeletonBoneIndexFromMeshBoneIndex(SkelMesh, BoneTreeIndex);
	int32 BoneTrackIndex = Skeleton->GetRawAnimationTrackIndex(BoneTreeIndex, AnimSeq);
	if (BoneTrackIndex == INDEX_NONE)
	{
		return FTransform3f(Skeleton->GetReferenceSkeleton().GetRefBonePose()[BoneTreeIndex]);
	}
	FTransform BoneTM = FTransform::Identity;
	AnimSeq->GetBoneTransform(BoneTM, BoneTrackIndex, AnimTime, true);
	return FTransform3f(BoneTM);
}

static FTransform3f GetAnimBoneCSTM(UAnimSequence* AnimSeq, const int32 BoneTreeIndex, const float AnimTime)
{
	USkeleton* Skeleton = AnimSeq->GetSkeleton();
	const auto& RefSkeleton = Skeleton->GetReferenceSkeleton();
	FTransform3f BoneTMWS = GetAnimBoneTM(AnimSeq, BoneTreeIndex, AnimTime);
	int32 CurrBone = BoneTreeIndex;
	while (true)
	{
		const int32 Parent(RefSkeleton.GetParentIndex(CurrBone));
		if (Parent < 0) break;
		else
		{
			
			BoneTMWS = BoneTMWS * GetAnimBoneTM(AnimSeq, Parent, AnimTime);

			CurrBone = Parent;
		}
	}
	return BoneTMWS;
}

MIRRORANIMATIONSYSTEMDEV_API void UMASFunctionLibrary::CreateMirrorSequenceFromAnimSequence_CS(
	UAnimSequence* MirrorSequence,
	const TEnumAsByte<EAxis::Type> MirrorAxis,
	const FString Substring_A, 
	const FString Substring_B, 
	const bool Symmetrical)
{
	IAnimationDataController& Controller = MirrorSequence->GetController();
	const int32 NumFrames = Controller.GetModel()->GetNumberOfFrames();
	const float DT = MirrorSequence->GetPlayLength() / NumFrames;

	USkeleton* Skeleton = MirrorSequence->GetSkeleton();
	const auto& RefSkeleton = Skeleton->GetReferenceSkeleton();

	
	
	TArray <bool> Already; Already.SetNumZeroed(Skeleton->GetReferenceSkeleton().GetRawBoneNum());

	TArray<FIntPoint> TwinPairs;
	TArray<int32> NonTwinIDs;
	TArray<EAxis::Type> NonTwinFlipAxis;
	FMASUtils::CSMirrorSettings(RefSkeleton, MirrorAxis, Substring_A, Substring_B, TwinPairs, NonTwinIDs, NonTwinFlipAxis);

	const bool DeltaStep = !Symmetrical;

	FVector3f TwinMirrorScale = FVector3f(1.f);
	FVector TargetAxis = FVector::ZeroVector;

	check(MirrorAxis != EAxis::None);
	{
		TwinMirrorScale[MirrorAxis - 1] = -1.f;
		TargetAxis[MirrorAxis - 1] = 1.f;
	}
	FTransform3f TwinMirrorModTM(FQuat4f::Identity, FVector3f::ZeroVector, TwinMirrorScale);


	TMap<int32, FRawAnimSequenceTrack> BoneTracks;
	
	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		BoneTracks.Add(i, FRawAnimSequenceTrack());
	}

	for (int32 j = 0; j < NumFrames; j++)
	{
		TArray <FTransform3f> NewCSTMs; NewCSTMs.SetNum(RefSkeleton.GetNum());

		for (int32 i = 0; i < NonTwinIDs.Num(); i++)
		{
			int32 BoneTreeIndex = NonTwinIDs[i];
			int32 BoneTrackIndex = Skeleton->GetRawAnimationTrackIndex(BoneTreeIndex, MirrorSequence);

			if (BoneTrackIndex == INDEX_NONE)
			{
				const int32 ParentIndex = RefSkeleton.GetParentIndex(BoneTreeIndex);
				if (ParentIndex != INDEX_NONE)
				{
					NewCSTMs[BoneTreeIndex] = FTransform3f(RefSkeleton.GetRefBonePose()[BoneTreeIndex]) * NewCSTMs[ParentIndex];
				}
				else
				{
					NewCSTMs[BoneTreeIndex] = FTransform3f(RefSkeleton.GetRefBonePose()[BoneTreeIndex]);
				}

				continue;
			}

			FTransform3f CSTM = GetAnimBoneCSTM(MirrorSequence, BoneTreeIndex, DT * j);
			CSTM.Mirror(MirrorAxis, NonTwinFlipAxis[i]);

			NewCSTMs[BoneTreeIndex] = CSTM;
		}


		for (int32 i = 0; i < TwinPairs.Num(); i++)
		{
			const int32 BoneIndex = TwinPairs[i].X;
			const FCompactPoseBoneIndex CmptBoneIndex(BoneIndex);

			const int32 TwinBoneIndex = TwinPairs[i].Y;

			const FCompactPoseBoneIndex TwinCmptBoneIndex(TwinBoneIndex);

			const FTransform3f RefTM = FTransform3f(FAnimationRuntime::GetComponentSpaceTransformRefPose(RefSkeleton, BoneIndex));
			const FTransform3f TwinRefTM = FTransform3f(FAnimationRuntime::GetComponentSpaceTransformRefPose(RefSkeleton, TwinBoneIndex));

			const FTransform3f TM = GetAnimBoneCSTM(MirrorSequence, BoneIndex, DT * j); 
			//Output.Pose.GetComponentSpaceTransform(CmptBoneIndex);
			const FTransform3f TwinTM = GetAnimBoneCSTM(MirrorSequence, TwinBoneIndex, DT * j); 
			//Output.Pose.GetComponentSpaceTransform(TwinCmptBoneIndex);

			const int32 ParentIndex = RefSkeleton.GetParentIndex(BoneIndex);
			const int32 TwinParentIndex = RefSkeleton.GetParentIndex(TwinBoneIndex);

			const bool SameParent = ParentIndex == TwinParentIndex;

			// twin 1�
			{
				const UE::Math::TTransform<float> MirrRef = RefTM * TwinMirrorModTM;
				const FTransform3f Delta = TwinRefTM.GetRelativeTransform(MirrRef);
				const FQuat4f DeltaQuat = Delta.GetRotation();

				FTransform3f MirrTM = TM * TwinMirrorModTM;

				MirrTM.SetRotation(MirrTM.GetRotation() * FQuat4f(DeltaQuat));
				MirrTM.SetScale3D(TwinTM.GetScale3D());

				if (DeltaStep)
				{
					if (SameParent)
					{
						FTransform3f RefBS = RefTM;
						RefBS = RefBS * TwinMirrorModTM;
						const FVector3f PosDelta = MirrTM.GetLocation() - RefBS.GetLocation();
						MirrTM.SetLocation(FVector3f(TwinRefTM.GetLocation()) + PosDelta);
					}
					else
					{
						const FTransform3f& ParentTwinTM = NewCSTMs[RefSkeleton.GetParentIndex(TwinBoneIndex)];
						const FTransform3f& IParentTM =// Output.Pose.GetComponentSpaceTransform(FCompactPoseBoneIndex(ParentIndex));
							GetAnimBoneCSTM(MirrorSequence, ParentIndex, DT * j);
						FTransform3f RefBS = FTransform3f(RefSkeleton.GetRefBonePose()[BoneIndex]) * IParentTM;
						RefBS = RefBS * TwinMirrorModTM;
						RefBS.SetRotation(RefBS.GetRotation() * DeltaQuat);
						RefBS.SetScale3D(TwinTM.GetScale3D());

						MirrTM = (MirrTM.GetRelativeTransform(RefBS) * FTransform3f(RefSkeleton.GetRefBonePose()[TwinBoneIndex])) * ParentTwinTM;
					}
				}

				NewCSTMs[TwinBoneIndex] = MirrTM;
			}

			// twin 2�
			{
				FTransform3f TwinMirrRef = TwinRefTM * TwinMirrorModTM;
				const FQuat4f TwinDeltaQuat = TwinMirrRef.GetRotation().Inverse() * RefTM.GetRotation();

				FTransform3f TwinMirrTM = TwinTM * TwinMirrorModTM;

				TwinMirrTM.SetRotation(TwinMirrTM.GetRotation() * TwinDeltaQuat);
				TwinMirrTM.SetScale3D(TM.GetScale3D());

				if (DeltaStep)
				{
					if (SameParent)
					{
						FTransform3f TwinRefBS = TwinRefTM;
						TwinRefBS = TwinRefBS * TwinMirrorModTM;
						const FVector3f PosDelta = TwinMirrTM.GetLocation() - TwinRefBS.GetLocation();
						TwinMirrTM.SetLocation(RefTM.GetLocation() + PosDelta);
					}
					else
					{
						const FTransform3f& ParentTM = NewCSTMs[RefSkeleton.GetParentIndex(BoneIndex)];
						const FTransform3f& IParentTwinTM = //Output.Pose.GetComponentSpaceTransform(FCompactPoseBoneIndex(TwinParentIndex));
							GetAnimBoneCSTM(MirrorSequence, TwinParentIndex, DT * j);
						FTransform3f TwinRefBS = FTransform3f(RefSkeleton.GetRefBonePose()[TwinBoneIndex]) * IParentTwinTM;
						TwinRefBS = TwinRefBS * TwinMirrorModTM;
						TwinRefBS.SetRotation(TwinRefBS.GetRotation() * TwinDeltaQuat);
						TwinRefBS.SetScale3D(TM.GetScale3D());

						TwinMirrTM = (TwinMirrTM.GetRelativeTransform(TwinRefBS) * FTransform3f(RefSkeleton.GetRefBonePose()[BoneIndex]) * ParentTM);
					}
				}

				NewCSTMs[BoneIndex] = TwinMirrTM;
			}
		}


		for (int32 i = 0; i < NewCSTMs.Num(); i++)
		{
			const int32 ParentIndex = RefSkeleton.GetParentIndex(i);
			FTransform3f BSTM;
			if (ParentIndex != INDEX_NONE) BSTM = NewCSTMs[i].GetRelativeTransform(NewCSTMs[ParentIndex]);
			else BSTM = NewCSTMs[i];

			auto& BoneTrack = BoneTracks[i];
			BoneTrack.PosKeys.Add(BSTM.GetLocation());
			BoneTrack.RotKeys.Add(BSTM.GetRotation());
			BoneTrack.ScaleKeys.Add(BSTM.GetScale3D());
		}
	}

	for (auto Pair : BoneTracks)
	{
		const FName TrackName = Skeleton->GetReferenceSkeleton().GetBoneName(Pair.Key);
		//MirrorSequence->AddNewRawTrack(TrackName, &Pair.Value);
		Controller.AddBoneCurve(TrackName);
		Controller.SetBoneTrackKeys(TrackName,Pair.Value.PosKeys,Pair.Value.RotKeys,Pair.Value.ScaleKeys);
	
	}
	// Have to also apply to pelvis and spine_01

	MirrorSequence->MarkPackageDirty();
}

#endif

#undef LOCTEXT_NAMESPACE
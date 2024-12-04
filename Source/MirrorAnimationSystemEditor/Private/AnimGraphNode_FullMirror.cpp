// Copyright 2017-2021 Rexocrates. All Rights Reserved.
#include "AnimGraphNode_FullMirror.h"
#include "MirrorAnimationSystemEditor.h"


#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_FullMirror::UAnimGraphNode_FullMirror(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

}

FLinearColor UAnimGraphNode_FullMirror::GetNodeTitleColor() const
{
	return FLinearColor::Red;
}

FText UAnimGraphNode_FullMirror::GetTooltipText() const
{
	return LOCTEXT("Mirrors_the_designated_bones", "Mirrors the pose based on the designated Mirror Table");
}

FText UAnimGraphNode_FullMirror::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("FullMirror_Pose", "Full Mirror Pose");
}

FString UAnimGraphNode_FullMirror::GetNodeCategory() const
{
	return TEXT("Tools");
}

#undef LOCTEXT_NAMESPACE

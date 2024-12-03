// Copyright 2017-2021 Rexocrates. All Rights Reserved.
#include "AnimGraphNode_FullMirrorCS.h"
#include "MirrorAnimationSystemEditor.h"


#define LOCTEXT_NAMESPACE "A3Nodes"


UAnimGraphNode_FullMirrorCS::UAnimGraphNode_FullMirrorCS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FLinearColor UAnimGraphNode_FullMirrorCS::GetNodeTitleColor() const
{
	return FLinearColor::Blue;
}

FText UAnimGraphNode_FullMirrorCS::GetControllerDescription() const
{
	return LOCTEXT("AnimGraphNode_MirrorCS", "Mirror Pose CS");
}

FText UAnimGraphNode_FullMirrorCS::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_MirrorCS_Tooltip", "Mirror the pose in Component Space.");
}

FText UAnimGraphNode_FullMirrorCS::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FText NodeTitle;
	if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
	{
		NodeTitle = GetControllerDescription();
	}
	else
	{
		NodeTitle = FText(LOCTEXT("AnimGraphNode_MirrorCS_Title", "Mirror Pose CS"));
	}
	return NodeTitle;
}

void UAnimGraphNode_FullMirrorCS::ValidateAnimNodePostCompile(class FCompilerResultsLog& MessageLog, class UAnimBlueprintGeneratedClass* CompiledClass, int32 CompiledNodeIndex)
{
	Super::ValidateAnimNodePostCompile(MessageLog, CompiledClass, CompiledNodeIndex);
}

bool UAnimGraphNode_FullMirrorCS::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	return Super::IsCompatibleWithGraph(TargetGraph);
}

#undef LOCTEXT_NAMESPACE

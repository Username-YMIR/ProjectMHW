#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Weapons/Common/MHWeaponComboTypes.h"
#include "MHLongSwordComboGraph.generated.h"

class UAnimMontage;

// 태도 콤보 노드
USTRUCT(BlueprintType)
struct FMHLongSwordComboNode
{
    GENERATED_BODY()

    // 모션 태그
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    FGameplayTag MoveTag;

    // 재생 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    TSoftObjectPtr<UAnimMontage> Montage;

    // 몽타주 섹션(없으면 None)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    FName SectionName = NAME_None;

    // 기본 입력 다음 후보(우선순위 순)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Next")
    TArray<FGameplayTag> PrimaryNextMoves;

    // 보조 입력 다음 후보(우선순위 순)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Next")
    TArray<FGameplayTag> SecondaryNextMoves;

    // 특수 입력 다음 후보(우선순위 순)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Next")
    TArray<FGameplayTag> SpecialNextMoves;
};

// 태도 콤보 그래프 데이터
UCLASS(BlueprintType)
class PROJECTMHW_API UMHLongSwordComboGraph : public UDataAsset
{
    GENERATED_BODY()

public:
    // 시작 모션 후보(우선순위 순)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Entry")
    TArray<FGameplayTag> EntryMoves_Primary;

    // 시작 모션 후보(우선순위 순)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Entry")
    TArray<FGameplayTag> EntryMoves_Secondary;

    // 시작 모션 후보(우선순위 순)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Entry")
    TArray<FGameplayTag> EntryMoves_Special;

    // 전체 노드 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    TArray<FMHLongSwordComboNode> Nodes;

public:
    // 태그로 노드 찾기
    const FMHLongSwordComboNode* FindNode(const FGameplayTag& InMoveTag) const;

    // 입력 타입에 따른 다음 후보 배열 반환
    static const TArray<FGameplayTag>& GetNextList(const FMHLongSwordComboNode& Node, EMHComboInputType InputType);

    // 입력 타입에 따른 시작 후보 배열 반환
    const TArray<FGameplayTag>& GetEntryList(EMHComboInputType InputType) const;

    // 에디터에서 기본 콤보 데이터 자동 채우기
    UFUNCTION(CallInEditor, Category = "Combo|Tools")
    void PopulateDefaults_LongSword();
};

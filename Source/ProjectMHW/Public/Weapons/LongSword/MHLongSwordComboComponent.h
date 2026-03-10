#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Weapons/Common/MHWeaponComboTypes.h"
#include "MHLongSwordComboComponent.generated.h"

class UMHLongSwordComboGraph;
struct FMHLongSwordComboNode;

// 태도 콤보 관리 컴포넌트
UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent))
class PROJECTMHW_API UMHLongSwordComboComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMHLongSwordComboComponent();

public:
    // 콤보 그래프 지정
    void SetComboGraph(UMHLongSwordComboGraph* InGraph);

    // 콤보 그래프 반환
    const UMHLongSwordComboGraph* GetComboGraph() const { return ComboGraph; }

    // 입력 버퍼 저장
    bool BufferInput(EMHComboInputType InputType); //손승우 수정

    // 직접 지정한 모션 태그 버퍼 저장
    bool BufferRequestedMove(const FGameplayTag& RequestedMoveTag); //손승우 추가

    // 버퍼 입력 소비
    EMHComboInputType ConsumeBufferedInput();

    // 직접 지정한 모션 태그 버퍼 소비
    FGameplayTag ConsumeBufferedRequestedMove(); //손승우 추가

    // 버퍼 입력 존재 여부
    bool HasBufferedInput() const;

    // 직접 지정한 모션 태그 버퍼 존재 여부
    bool HasBufferedRequestedMove() const; //손승우 추가

    // 체인 윈도우 안에서 승인된 입력 여부
    bool HasAcceptedBufferedInput() const; //손승우 추가

    // 콤보 입력 윈도우 설정
    void SetChainWindowOpen(bool bOpen);

    // 콤보 입력 윈도우 여부
    bool IsChainWindowOpen() const { return bChainWindowOpen; }

    // 현재 모션 태그 반환
    const FGameplayTag& GetCurrentMoveTag() const { return CurrentMoveTag; }

    // 콤보 활성 여부
    bool IsComboActive() const { return bComboActive; }

    // 현재 모션 기반 다음 노드 선택
    const FMHLongSwordComboNode* SelectNextNode(EMHComboInputType InputType) const;

    // 선택된 노드를 현재 모션으로 확정
    void CommitMove(const FMHLongSwordComboNode& Node);

    // 콤보 종료
    void ResetCombo();

protected:
    // 현재 노드 가져오기
    const FMHLongSwordComboNode* GetCurrentNode() const;

private:
    UPROPERTY(Transient)
    TObjectPtr<UMHLongSwordComboGraph> ComboGraph; // 콤보 그래프

    UPROPERTY(Transient)
    FGameplayTag CurrentMoveTag; // 현재 모션 태그

    UPROPERTY(Transient)
    bool bComboActive = false; // 콤보 활성

    UPROPERTY(Transient)
    bool bChainWindowOpen = false; //손승우 수정

    UPROPERTY(Transient)
    bool bBufferedInputAccepted = false; //손승우 추가

    UPROPERTY(Transient)
    EMHComboInputType BufferedInput = EMHComboInputType::None; // 입력 버퍼

    UPROPERTY(Transient)
    FGameplayTag BufferedRequestedMoveTag; //손승우 추가
};

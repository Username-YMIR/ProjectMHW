#include "Items/Instance/MHGreatSwordInstance.h"

#include "Weapons/GreatSword/MHGreatSwordActionComponent.h"
#include "Weapons/GreatSword/MHGreatSwordChargeStateComponent.h"
#include "Weapons/GreatSword/MHGreatSwordComboGraph.h"

DEFINE_LOG_CATEGORY(LogMHGreatSwordInstance);

AMHGreatSwordInstance::AMHGreatSwordInstance()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
    WeaponType = EMHWeaponType::GreatSword;

    ActionComponent = CreateDefaultSubobject<UMHGreatSwordActionComponent>(TEXT("ActionComponent"));
    ChargeStateComponent = CreateDefaultSubobject<UMHGreatSwordChargeStateComponent>(TEXT("ChargeStateComponent"));
}

void AMHGreatSwordInstance::ApplyItemData()
{
    Super::ApplyItemData();

    const UMHGreatSwordItemData* GreatSwordData = GetGreatSwordData();
    if (!GreatSwordData)
    {
        UE_LOG(LogMHGreatSwordInstance, Warning, TEXT("%s : 대검 아이템 데이터를 찾지 못했습니다."), *GetName());
        return;
    }
}

void AMHGreatSwordInstance::BeginPlay()
{
    Super::BeginPlay();

    if (!ActionComponent)
    {
        UE_LOG(LogMHGreatSwordInstance, Warning, TEXT("%s : ActionComponent가 없어 대검 초기화를 중단합니다."), *GetName());
        return;
    }

    UMHGreatSwordComboGraph* ComboGraph = GetComboGraph();
    if (!ComboGraph)
    {
        RuntimeComboGraph = NewObject<UMHGreatSwordComboGraph>(this, TEXT("RuntimeComboGraph"));
        if (RuntimeComboGraph)
        {
            RuntimeComboGraph->PopulateDefaults_GreatSword();
            ComboGraph = RuntimeComboGraph;
            UE_LOG(LogMHGreatSwordInstance, Verbose, TEXT("%s : 런타임 기본 대검 콤보 그래프를 생성했습니다."), *GetName());
        }
    }

    ActionComponent->SetChargeStateComponent(ChargeStateComponent);
    ActionComponent->SetComboGraph(ComboGraph);

    UE_LOG(LogMHGreatSwordInstance, Verbose, TEXT("%s : 대검 액션 컴포넌트 연결을 마쳤습니다. Graph=%s"), *GetName(), ComboGraph ? *ComboGraph->GetName() : TEXT("None"));
}

UMHGreatSwordComboGraph* AMHGreatSwordInstance::GetComboGraph() const
{
    if (RuntimeComboGraph)
    {
        return RuntimeComboGraph;
    }

    return ComboGraphAsset.IsNull() ? nullptr : ComboGraphAsset.LoadSynchronous();
}

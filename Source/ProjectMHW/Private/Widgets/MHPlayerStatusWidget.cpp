#include "Widgets/MHPlayerStatusWidget.h"

#include "Character/Player/MHPlayerCharacter.h"
#include "Widgets/MHLayeredProgressBarWidget.h"
#include "Widgets/MHProgressBarWidget.h"

UMHPlayerStatusWidget::UMHPlayerStatusWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMHPlayerStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToPlayerCharacter();
	SyncInitialValues();
}

void UMHPlayerStatusWidget::NativeDestruct()
{
	UnbindFromPlayerCharacter();

	Super::NativeDestruct();
}

void UMHPlayerStatusWidget::HandleWidgetInitialized()
{
	Super::HandleWidgetInitialized();

	BindToPlayerCharacter();
}

void UMHPlayerStatusWidget::BindToPlayerCharacter()
{
	if (CachedPlayerCharacter.IsValid())
	{
		return;
	}

	AMHPlayerCharacter* PlayerCharacter = Cast<AMHPlayerCharacter>(GetMHPawn());
	if (!PlayerCharacter)
	{
		return;
	}

	CachedPlayerCharacter = PlayerCharacter;

	// 아래 Delegate 이름은 PlayerCharacter 설계 예시에 맞춘 가정이다.
	// 실제 프로젝트에 맞게 이름/시그니처만 조정하면 된다.
	PlayerCharacter->OnHealthChanged.AddDynamic(this, &ThisClass::HandleHealthChanged);
	PlayerCharacter->OnSpiritGaugeChanged.AddDynamic(this, &ThisClass::HandleSpiritGaugeChanged);
	PlayerCharacter->OnStaminaChanged.AddDynamic(this, &ThisClass::HandleStaminaChanged);
	PlayerCharacter->OnSharpnessChanged.AddDynamic(this, &ThisClass::HandleSharpnessChanged);
}

void UMHPlayerStatusWidget::UnbindFromPlayerCharacter()
{
	if (!CachedPlayerCharacter.IsValid())
	{
		return;
	}

	CachedPlayerCharacter->OnHealthChanged.RemoveDynamic(this, &ThisClass::HandleHealthChanged);
	CachedPlayerCharacter->OnSpiritGaugeChanged.RemoveDynamic(this, &ThisClass::HandleSpiritGaugeChanged);
	CachedPlayerCharacter->OnStaminaChanged.RemoveDynamic(this, &ThisClass::HandleStaminaChanged);
	CachedPlayerCharacter->OnSharpnessChanged.RemoveDynamic(this, &ThisClass::HandleSharpnessChanged);

	CachedPlayerCharacter.Reset();
}

void UMHPlayerStatusWidget::SyncInitialValues()
{
	AMHPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get();
	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<AMHPlayerCharacter>(GetMHPawn());
	}

	if (!PlayerCharacter)
	{
		return;
	}

	SetHealthValues(
		PlayerCharacter->GetCurrentHealthValue(),
		PlayerCharacter->GetMaxHealthValue()
	);

	SetSpiritGaugeValues(
		PlayerCharacter->GetCurrentSpiritGaugeValue(),
		PlayerCharacter->GetMaxSpiritGaugeValue()
	);

	SetStaminaValues(
		PlayerCharacter->GetCurrentStaminaValue(),
		PlayerCharacter->GetMaxStaminaValue()
	);

	// Sharpness는 실제 프로젝트 구조에 맞는 getter로 교체 필요
	// 예시 값
	SetSharpnessValues(100.f, 100.f);
}

void UMHPlayerStatusWidget::SetCurrentHealth(float InCurrentHealth)
{
	const float CurrentHealth = InCurrentHealth;

	if (HPBar)
	{
		HPBar->SetFrontCurrentValue(InCurrentHealth);
	}
}

void UMHPlayerStatusWidget::SetMaxHealth(float InMaxHealth)
{
	const float MaxHealth = FMath::Max(0.f, InMaxHealth);

	if (HPBar)
	{
		HPBar->SetFrontMaxValue(MaxHealth);
	}
}

void UMHPlayerStatusWidget::SetHealthValues(float InCurrentHealth, float InMaxHealth)
{
	const float CurrentHealth = InCurrentHealth;
	const float MaxHealth = FMath::Max(0.f, InMaxHealth);

	if (HPBar)
	{
		HPBar->SetFrontValues(CurrentHealth, MaxHealth);
		HPBar->SetBackValues(CurrentHealth, MaxHealth);
	}
}

void UMHPlayerStatusWidget::SetCurrentSpiritGauge(float InCurrentSpiritGauge)
{
	const float CurrentSpiritGauge = InCurrentSpiritGauge;

	if (SpiritGaugeBar)
	{
		SpiritGaugeBar->SetFrontCurrentValue(CurrentSpiritGauge);
	}
}

void UMHPlayerStatusWidget::SetMaxSpiritGauge(float InMaxSpiritGauge)
{
	const float MaxSpiritGauge = FMath::Max(0.f, InMaxSpiritGauge);

	if (SpiritGaugeBar)
	{
		SpiritGaugeBar->SetFrontMaxValue(MaxSpiritGauge);
	}
}

void UMHPlayerStatusWidget::SetSpiritGaugeValues(float InCurrentSpiritGauge, float InMaxSpiritGauge)
{
	const float CurrentSpiritGauge = InCurrentSpiritGauge;
	const float MaxSpiritGauge = FMath::Max(0.f, InMaxSpiritGauge);

	if (SpiritGaugeBar)
	{
		SpiritGaugeBar->SetFrontValues(CurrentSpiritGauge, MaxSpiritGauge);
		SpiritGaugeBar->SetBackValues(CurrentSpiritGauge, MaxSpiritGauge);
	}
}

void UMHPlayerStatusWidget::SetCurrentStamina(float InCurrentStamina)
{
	const float CurrentStamina = InCurrentStamina;

	if (StaminaBar)
	{
		StaminaBar->SetCurrentValue(CurrentStamina);
	}
}

void UMHPlayerStatusWidget::SetMaxStamina(float InMaxStamina)
{
	const float MaxStamina = FMath::Max(0.f, InMaxStamina);

	if (StaminaBar)
	{
		StaminaBar->SetMaxValue(MaxStamina);
	}
}

void UMHPlayerStatusWidget::SetStaminaValues(float InCurrentStamina, float InMaxStamina)
{
	const float CurrentStamina = InCurrentStamina;
	const float MaxStamina = FMath::Max(0.f, InMaxStamina);

	if (StaminaBar)
	{
		StaminaBar->SetValues(CurrentStamina, MaxStamina);
	}
}

void UMHPlayerStatusWidget::SetCurrentSharpness(float InCurrentSharpness)
{
	const float CurrentSharpness = InCurrentSharpness;

	if (SharpnessBar)
	{
		SharpnessBar->SetCurrentValue(CurrentSharpness);
	}
}

void UMHPlayerStatusWidget::SetMaxSharpness(float InMaxSharpness)
{
	const float MaxSharpness = FMath::Max(0.f, InMaxSharpness);

	if (SharpnessBar)
	{
		SharpnessBar->SetMaxValue(MaxSharpness);
	}
}

void UMHPlayerStatusWidget::SetSharpnessValues(float InCurrentSharpness, float InMaxSharpness)
{
	const float CurrentSharpness = InCurrentSharpness;
	const float MaxSharpness = FMath::Max(0.f, InMaxSharpness);

	if (SharpnessBar)
	{
		SharpnessBar->SetValues(CurrentSharpness, MaxSharpness);
	}
}

void UMHPlayerStatusWidget::HandleHealthChanged(float InCurrentHealth, float InMaxHealth)
{
	SetHealthValues(InCurrentHealth, InMaxHealth);
}

void UMHPlayerStatusWidget::HandleSpiritGaugeChanged(float InCurrentSpiritGauge, float InMaxSpiritGauge)
{
	SetSpiritGaugeValues(InCurrentSpiritGauge, InMaxSpiritGauge);
}

void UMHPlayerStatusWidget::HandleStaminaChanged(float InCurrentStamina, float InMaxStamina)
{
	SetStaminaValues(InCurrentStamina, InMaxStamina);
}

void UMHPlayerStatusWidget::HandleSharpnessChanged(float InCurrentSharpness, float InMaxSharpness)
{
	SetSharpnessValues(InCurrentSharpness, InMaxSharpness);
}

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
	PlayerCharacter->OnHealableHealthChanged.AddDynamic(this, &ThisClass::HandleHealableHealthChanged);
	PlayerCharacter->OnSpiritGaugeChanged.AddDynamic(this, &ThisClass::HandleSpiritGaugeChanged);
	PlayerCharacter->OnSpiritLevelChanged.AddDynamic(this, &ThisClass::HandleSpiritLevelChanged);
	PlayerCharacter->OnSpiritLevelTimerChanged.AddDynamic(this, &ThisClass::HandleSpiritLevelTimerChanged);
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
	CachedPlayerCharacter->OnHealableHealthChanged.RemoveDynamic(this, &ThisClass::HandleHealableHealthChanged);
	CachedPlayerCharacter->OnSpiritGaugeChanged.RemoveDynamic(this, &ThisClass::HandleSpiritGaugeChanged);
	CachedPlayerCharacter->OnSpiritLevelChanged.RemoveDynamic(this, &ThisClass::HandleSpiritLevelChanged);
	CachedPlayerCharacter->OnSpiritLevelTimerChanged.RemoveDynamic(this, &ThisClass::HandleSpiritLevelTimerChanged);
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
	HandleSpiritLevelChanged(
		PlayerCharacter->GetCurrentSpiritLevelValue(),
		PlayerCharacter->GetMaxSpiritLevelValue()
	);
	SetSpiritGaugeTimerValues(
		PlayerCharacter->GetSpiritLevelRemainingTime(),
		PlayerCharacter->GetSpiritLevelDuration()
	);

	SetStaminaValues(
		PlayerCharacter->GetCurrentStaminaValue(),
		PlayerCharacter->GetMaxStaminaValue()
	);

	SetSharpnessValues(
		PlayerCharacter->GetCurrentSharpnessValue(),
		PlayerCharacter->GetMaxSharpnessValue()
	);

	HandleHealableHealthChanged(
		PlayerCharacter->GetCurrentHealableHealthValue(),
		PlayerCharacter->GetMaxHealthValue()
	);
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
	}
}

void UMHPlayerStatusWidget::SetSpiritGaugeTimerValues(float InRemainingTime, float InDuration)
{
	if (SpiritGaugeBar)
	{
		SpiritGaugeBar->SetBackValues(InRemainingTime, FMath::Max(0.f, InDuration));
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
		UpdateSharpnessBarVisual();
	}
}

void UMHPlayerStatusWidget::SetMaxSharpness(float InMaxSharpness)
{
	const float MaxSharpness = FMath::Max(0.f, InMaxSharpness);

	if (SharpnessBar)
	{
		SharpnessBar->SetMaxValue(MaxSharpness);
		UpdateSharpnessBarVisual();
	}
}

void UMHPlayerStatusWidget::SetSharpnessValues(float InCurrentSharpness, float InMaxSharpness)
{
	const float CurrentSharpness = InCurrentSharpness;
	const float MaxSharpness = FMath::Max(0.f, InMaxSharpness);

	if (SharpnessBar)
	{
		SharpnessBar->SetValues(CurrentSharpness, MaxSharpness);
		UpdateSharpnessBarVisual();
	}
}

void UMHPlayerStatusWidget::UpdateSharpnessBarVisual()
{
	if (!SharpnessBar)
	{
		return;
	}

	const AMHPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get();
	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<AMHPlayerCharacter>(GetMHPawn());
	}

	if (!PlayerCharacter
		|| !PlayerCharacter->GetEquippedWeapon()
		|| PlayerCharacter->GetMaxSharpnessValue() <= 0.0f)
	{
		SharpnessBar->SetValues(0.0f, 0.0f);
		SharpnessBar->ResetFillColor();
		return;
	}

	SharpnessBar->SetValues(
		PlayerCharacter->GetCurrentSharpnessSegmentValue(),
		PlayerCharacter->GetCurrentSharpnessSegmentMax());
	SharpnessBar->SetSharpnessFillColor(PlayerCharacter->GetCurrentSharpnessColor());
}

void UMHPlayerStatusWidget::HandleHealthChanged(float InCurrentHealth, float InMaxHealth)
{
	SetHealthValues(InCurrentHealth, InMaxHealth);
}

void UMHPlayerStatusWidget::HandleHealableHealthChanged(float InHealableHealth, float InMaxHealth)
{
	if (HPBar)
	{
		const float FrontHealth = CachedPlayerCharacter.IsValid()
			? CachedPlayerCharacter->GetCurrentHealthValue()
			: 0.f;

		HPBar->SetFrontValues(FrontHealth, InMaxHealth);
		HPBar->SetBackValues(FrontHealth + InHealableHealth, InMaxHealth);
	}
}

void UMHPlayerStatusWidget::HandleSpiritGaugeChanged(float InCurrentSpiritGauge, float InMaxSpiritGauge)
{
	SetSpiritGaugeValues(InCurrentSpiritGauge, InMaxSpiritGauge);
}

void UMHPlayerStatusWidget::HandleSpiritLevelChanged(int32 InCurrentSpiritLevel, int32 InMaxSpiritLevel)
{
	(void)InMaxSpiritLevel;

	CachedSpiritLevel = FMath::Clamp(InCurrentSpiritLevel, 0, 3);
	UpdateSpiritGaugeVisual();
}

void UMHPlayerStatusWidget::HandleSpiritLevelTimerChanged(float InRemainingTime, float InDuration)
{
	SetSpiritGaugeTimerValues(InRemainingTime, InDuration);
}

void UMHPlayerStatusWidget::HandleStaminaChanged(float InCurrentStamina, float InMaxStamina)
{
	SetStaminaValues(InCurrentStamina, InMaxStamina);
}

void UMHPlayerStatusWidget::HandleSharpnessChanged(float InCurrentSharpness, float InMaxSharpness)
{
	SetSharpnessValues(InCurrentSharpness, InMaxSharpness);
}

void UMHPlayerStatusWidget::UpdateSpiritGaugeVisual()
{
	ApplySpiritGaugeLevelStyle(CachedSpiritLevel);
}

void UMHPlayerStatusWidget::ApplySpiritGaugeLevelStyle(const int32 InSpiritLevel)
{
	if (!SpiritGaugeBar)
	{
		return;
	}

	SpiritGaugeBar->SetFrontFillColor(GetSpiritLevelFrontColor(InSpiritLevel));
	SpiritGaugeBar->SetBackFillColor(GetSpiritLevelBackColor(InSpiritLevel));
}

FLinearColor UMHPlayerStatusWidget::GetSpiritLevelFrontColor(const int32 InSpiritLevel) const
{
	switch (FMath::Clamp(InSpiritLevel, 0, 3))
	{
	case 1:
		return SpiritGaugeLevel1FrontColor;
	case 2:
		return SpiritGaugeLevel2FrontColor;
	case 3:
		return SpiritGaugeLevel3FrontColor;
	default:
		return SpiritGaugeLevel0FrontColor;
	}
}

FLinearColor UMHPlayerStatusWidget::GetSpiritLevelBackColor(const int32 InSpiritLevel) const
{
	switch (FMath::Clamp(InSpiritLevel, 0, 3))
	{
	case 1:
		return SpiritGaugeLevel1BackColor;
	case 2:
		return SpiritGaugeLevel2BackColor;
	case 3:
		return SpiritGaugeLevel3BackColor;
	default:
		return SpiritGaugeLevel0BackColor;
	}
}

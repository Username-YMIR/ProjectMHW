#include "Camera/mh_hit_enemy_camera_shake.h"

DEFINE_LOG_CATEGORY(LogMHHitEnemyCameraShake);

UMHHitEnemyCameraShake::UMHHitEnemyCameraShake()
{
    OscillationDuration = 0.08f;
    OscillationBlendInTime = 0.0f;
    OscillationBlendOutTime = 0.04f;

    RotOscillation.Pitch.Amplitude = 1.5f;
    RotOscillation.Pitch.Frequency = 30.0f;
    RotOscillation.Yaw.Amplitude = 1.0f;
    RotOscillation.Yaw.Frequency = 24.0f;
    RotOscillation.Roll.Amplitude = 0.5f;
    RotOscillation.Roll.Frequency = 18.0f;

    LocOscillation.X.Amplitude = 0.0f;
    LocOscillation.Y.Amplitude = 0.0f;
    LocOscillation.Z.Amplitude = 0.0f;
}

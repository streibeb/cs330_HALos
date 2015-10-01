#define HAL_CPU_SCHEDULING_POLICY_CRITERIA_H

struct cpuSchedulingPolicyCriteria
{
    string type;
    int quantumLengthMultiplier;
    int contextSwitchesUntilMoveDown;
    int contextSwitchesUntilMoveUp;
};

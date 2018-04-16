#pragma once
#include <stdbool.h>

//Transform 0~510 To -1024~1024
#define MR2X30A_MAX 1024 
#define MR2X30A_MIN -1024
#define Arduino_Wheel_MR2x30A_SetVel_Transform(arduino, left, right) \
		Arduino_Wheel_MR2x30A_SetVel(arduino, (left - 255) * 4, (right - 255) * 4)

typedef void *Arduino;

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

	//Creat function
	bool Arduino_Uno_Creat(Arduino *Ptr_arduino);
	bool Arduino_Mega_2560_Creat(Arduino *Ptr_arduino);

	//Release function
	bool Arduino_Release(Arduino arduino);

	//Macro function (Main Control function)
	bool Arduino_Wheel_MR2x30A_Enable(Arduino arduino);
	bool Arduino_Wheel_MR2x30A_Disabl(Arduino arduino);
	bool Arduino_Wheel_MR2x30A_Brake(Arduino arduino);
	bool Arduino_Wheel_MR2x30A_SetVel(Arduino arduino, int left, int right);

	bool Arduino_Infrared_Ray_Get(Arduino arduino, int pin, bool *value);
	bool Arduino_Infrared_Ray_Event_Enable(Arduino arduino, int pin);
	bool Arduino_Infrared_Ray_Event_Disable(Arduino arduino, int pin);
	bool Arduino_Infrared_Ray_Event_Wait(Arduino arduino);
	bool Arduino_Infrared_Ray_Event_Result(Arduino arduino, int *pin, bool *value);

	bool Arduino_Robotic_Arm_Enable(Arduino arduino, int pin);
	bool Arduino_Robotic_Arm_Disable(Arduino arduino, int pin);

#ifdef __cplusplus
}
#endif // __cplusplus

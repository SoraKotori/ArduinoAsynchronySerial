#include "ArduinoAsynchronySerial.h"
#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

int main(void)
{
	Arduino arduino;
	if (false == Arduino_Uno_Creat(&arduino))
	{
		printf("Arduino_Uno_Creat");
		system("PAUSE");
		return 0;
	}

#define MR2x30A
#ifdef MR2x30A
	if (false == Arduino_Wheel_MR2x30A_Enable(arduino))
	{
		printf("Arduino_Wheel_MR2x30A_Enable");
		system("PAUSE");
		return 0;
	}
	for (size_t i = 0; i < 5; i++)
	{
		Arduino_Wheel_MR2x30A_SetVel_Transform(arduino, 300, 400);
		Sleep(1000);
		Arduino_Wheel_MR2x30A_SetVel_Transform(arduino, 510, 300);
		Sleep(1000);
		Arduino_Wheel_MR2x30A_Brake(arduino);
		Sleep(100);
		Arduino_Wheel_MR2x30A_SetVel_Transform(arduino, 0, 100);
		Sleep(1000);
		Arduino_Wheel_MR2x30A_SetVel_Transform(arduino, 200, 100);
		Sleep(1000);
	}
	
	if (false == Arduino_Wheel_MR2x30A_SetVel(arduino, 330, 330))
	{
		printf("Arduino_Wheel_MR2x30A_SetVel");
		system("PAUSE");
		return 0;
	}
	Sleep(1000);
	if (false == Arduino_Wheel_MR2x30A_Brake(arduino))
	{
		printf("Arduino_Wheel_MR2x30A_SetVel");
		system("PAUSE");
		return 0;
	}
	if (false == Arduino_Wheel_MR2x30A_Disabl(arduino))
	{
		printf("Arduino_Wheel_MR2x30A_Disabl");
		system("PAUSE");
		return 0;
	}

#endif

#ifdef Infrared_Ray
	if (false == Arduino_Infrared_Ray_Event_Enable(arduino, 2))
	{
		printf("Arduino_Infrared_Ray_Event_Enable");
		system("PAUSE");
		return 0;
	}
	if (false == Arduino_Infrared_Ray_Event_Enable(arduino, 3))
	{
		printf("Arduino_Infrared_Ray_Event_Enable");
		system("PAUSE");
		return 0;
	}

	for (size_t i = 0; i < 100; i++)
	{
		if (false == Arduino_Infrared_Ray_Event_Wait(arduino))
		{
			printf("Arduino_Infrared_Ray_Event_Wait");
			system("PAUSE");
			return 0;
		}

		int Pin;
		bool value;
		if (false == Arduino_Infrared_Ray_Event_Result(arduino, &Pin, &value))
		{
			printf("Arduino_Infrared_Ray_Event_Result");
			system("PAUSE");
			return 0;
		}
		printf("Pin: %d, value: %d\n", Pin, value);
	}

	if (false == Arduino_Infrared_Ray_Event_Disable(arduino, 2))
	{
		printf("Arduino_Infrared_Ray_Event_Disable");
		system("PAUSE");
		return 0;
}
	if (false == Arduino_Infrared_Ray_Event_Disable(arduino, 3))
	{
		printf("Arduino_Infrared_Ray_Event_Disable");
		system("PAUSE");
		return 0;
	}
#endif

	if (false == Arduino_Release(arduino))
	{
		printf("Arduino_Release");
		system("PAUSE");
		return 0;
	}
	system("PAUSE");
	return 0;
}
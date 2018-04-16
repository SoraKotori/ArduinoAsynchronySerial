#include <Arduino.h>
#include <Servo.h>
#include <utility/twi.h>
#include <Wire.h>

#define PIN_NUMBER 14
Servo servo[PIN_NUMBER];

void setup()
{
	Serial.begin(115200UL, SERIAL_8E1);
}

void loop()
{

}

typedef enum
{
	_LOW,
	_CHANGE,
	_RISING,
	_FALLING,
	_HIGH
}Triggered;

typedef enum
{
	_INPUT,
	_OUTPUT,
	_INPUT_PULLUP
}PinConfigure;

enum Decode
{
	Nothing,

	PinMode,
	DigitalRead,

	AnalogWrite,

	Interrupt,
	AttachInterrupt,
	DetachInterrupt,

	Servo_attach,
	Servo_writeMicroseconds,
	Servo_detach,

	Wire_Begin,
	Wire_End,
	Wire_BeginTransmission,
	Wire_EndTransmission,
	Wire_Write,
	Twi_WriteTo
};

void serialEvent()
{
	switch (Serial_readwait())
	{
	case PinMode:
		switch (Serial_readwait())
		{
		case _INPUT:
			pinMode(Serial_readwait(), INPUT);
			break;

		case _OUTPUT:
			pinMode(Serial_readwait(), OUTPUT);
			break;

		case _INPUT_PULLUP:
			pinMode(Serial_readwait(), INPUT_PULLUP);
			break;
		}
		break;

	case DigitalRead:
		digitalRead(Serial_readwait());
		break;

	case AnalogWrite:
		analogWrite(Serial_readwait(), Serial_readwait());
		break;

	case AttachInterrupt:
		Interrupt_number(Serial_readwait());
		break;

	case DetachInterrupt:
		detachInterrupt(digitalPinToInterrupt(Serial_readwait()));
		break;

	case Servo_attach:
		servo_attach(Serial_readwait());
		break;

	case Servo_writeMicroseconds:
		servo[Serial_readwait()].writeMicroseconds(Serial_readwait());
		break;

	case Servo_detach:
		servo[Serial_readwait()].detach();
		break;

	case Wire_Begin:
		Wire.begin();
		break;

	case Wire_End:
		Wire.end();
		break;

	case Wire_BeginTransmission:
		Wire.beginTransmission(Serial_readwait());
		break;

	case Wire_EndTransmission:
		Wire.endTransmission();
		break;

	case Wire_Write:
		for (int length = Serial_readwait(); length > 0; length--)
		{
			Wire.write(Serial_readwait());
		}
		break;
		//pulseIn
			//case Twi_WriteTo:
				//twi_writeTo();
			//	break;
	}
}

int Serial_readwait()
{
	while (true)
	{
		int receive = Serial.read();
		if (-1 != receive)
		{
			return receive;
		}
	}
}

void servo_attach(int pin)
{
	servo[pin].attach(pin);
}

#define Interrupt_Pin(number)					\
void Interrupt_Pin_##number(void)				\
{												\
	Serial.write((uint8_t)Interrupt);			\
	Serial.write((uint8_t)number);				\
	Serial.write((uint8_t)digitalRead(number));	\
}

#define Switch_AttachInterrupt_Pin(number)													\
case number:																				\
	switch (Serial_readwait())																\
	{																						\
		case _LOW:																			\
			attachInterrupt(digitalPinToInterrupt(pin), Interrupt_Pin_##number, LOW);		\
			break;																			\
		case _CHANGE:																		\
			attachInterrupt(digitalPinToInterrupt(pin), Interrupt_Pin_##number, CHANGE);	\
			break;																			\
		case _RISING:																		\
			attachInterrupt(digitalPinToInterrupt(pin), Interrupt_Pin_##number, RISING);	\
			break;																			\
		case _FALLING:																		\
			attachInterrupt(digitalPinToInterrupt(pin), Interrupt_Pin_##number, FALLING);	\
			break;																			\
		case _HIGH:																			\
			attachInterrupt(digitalPinToInterrupt(pin), Interrupt_Pin_##number, HIGH);		\
			break;																			\
	}																						\
	break;

Interrupt_Pin(0)
Interrupt_Pin(1)
Interrupt_Pin(2)
Interrupt_Pin(3)
Interrupt_Pin(4)
Interrupt_Pin(5)
Interrupt_Pin(6)
Interrupt_Pin(7)
Interrupt_Pin(8)
Interrupt_Pin(9)
Interrupt_Pin(10)
Interrupt_Pin(11)
Interrupt_Pin(12)
Interrupt_Pin(13)

void Interrupt_number(int pin)
{
	switch (pin)
	{
		Switch_AttachInterrupt_Pin(0)
			Switch_AttachInterrupt_Pin(1)
			Switch_AttachInterrupt_Pin(2)
			Switch_AttachInterrupt_Pin(3)
			Switch_AttachInterrupt_Pin(4)
			Switch_AttachInterrupt_Pin(5)
			Switch_AttachInterrupt_Pin(6)
			Switch_AttachInterrupt_Pin(7)
			Switch_AttachInterrupt_Pin(8)
			Switch_AttachInterrupt_Pin(9)
			Switch_AttachInterrupt_Pin(10)
			Switch_AttachInterrupt_Pin(11)
			Switch_AttachInterrupt_Pin(12)
			Switch_AttachInterrupt_Pin(13)
	}
}

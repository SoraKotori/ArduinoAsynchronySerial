#ifdef _MSC_VER
#pragma comment(lib, "SetupAPI")
#pragma warning(disable : 4996)
#endif // _MSC_VER

#include <windows.h>
#include <SetupAPI.h>
#include <tchar.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#define IO_Device_Module
#ifdef IO_Device_Module

#ifdef NDEBUG

void IO_Device_FormatLastError_Release(void) { return; }
#define IO_Device_FormatLastError IO_Device_FormatLastError_Release

#else

#define IO_Device_FormatLastError IO_Device_FormatLastError_Debug

#endif

#define ConversionDevice(address) (*(PIO_Device*)&address); \
		if (NULL == *(PIO_Device*)&address) { assert(!(NULL == *(PIO_Device*)&address)); return false; }

typedef struct
{
	HANDLE hFile;
	LPOVERLAPPED lpOverlappedRead;
	LPOVERLAPPED lpOverlappedWrite;
}IO_Device, *PIO_Device;

bool IO_Device_Creat_FileName(PIO_Device *ppIO_Device, LPCTSTR lpFileName, bool Asynchronous, LPCTSTR lpDef);
bool IO_Device_Creat_Hardware_ID(PIO_Device *ppIO_Device, LPCTSTR Hardware_ID, bool Asynchronous, LPCTSTR lpDef);
bool IO_Device_Release(PIO_Device pIO_Device);

bool IO_Device_Read(PIO_Device pIO_device, LPVOID lpBuffer, DWORD nNumberOfBytesToRead);
bool IO_Device_Read_Wait(PIO_Device pIO_device);
bool IO_Device_Read_Result(PIO_Device pIO_device, LPDWORD lpNumberOfBytesTransferred);

bool IO_Device_Write(PIO_Device pIO_device, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite);
bool IO_Device_Write_Wait(PIO_Device pIO_device);
bool IO_Device_Write_Result(PIO_Device pIO_device, LPDWORD lpNumberOfBytesTransferred);

bool IO_Device_Receiver_Wait(PIO_Device pIO_device);
bool IO_Device_Receiver_Number(PIO_Device pIO_device, LPDWORD lpReceived_Number);

bool IO_Device_Transmitter_Wait(PIO_Device pIO_device);
bool IO_Device_Transmitter_Number(PIO_Device pIO_device, LPDWORD lpTransmitted_Number);

bool IO_Device_FormatLastError_Debug(void)
{
	LPTSTR lpBuffer;
	if (0UL == FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(LPTSTR)&lpBuffer,
		0UL,
		NULL
	))
	{
		return false;
	}

	//#define Console
#ifdef Console
	if (0 > _ftprintf(stderr, lpBuffer)) { return false; }
#else
	OutputDebugString(lpBuffer);
#endif

	if (FALSE == HeapFree(GetProcessHeap(), 0UL, lpBuffer))
	{
		return false;
	}
	return true;
}

bool IO_Device_Creat_hFile(HANDLE *phFile, LPCTSTR lpFileName, bool Asynchronous, LPCTSTR lpDef)
{
	HANDLE hFile = CreateFile
	(
		lpFileName,
		GENERIC_READ | GENERIC_WRITE,
		0UL,
		NULL,
		OPEN_EXISTING,
		true == Asynchronous ? FILE_FLAG_OVERLAPPED : 0UL,
		NULL
	);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		IO_Device_FormatLastError();
		return false;
	}

	//#define uncertainty
#ifndef uncertainty
	DCB dcb;
	if (FALSE == GetCommState(hFile, &dcb))
	{
		IO_Device_FormatLastError();
		return false;
	}
	if (FALSE == BuildCommDCB(lpDef, &dcb))
	{
		IO_Device_FormatLastError();
		return false;
	}
	if (FALSE == SetCommState(hFile, &dcb))
	{
		IO_Device_FormatLastError();
		return false;
	}
#else
	DCB dcb;
	if (FALSE == GetCommState(hFile, &dcb))
	{
		IO_Device_FormatLastError();
		return false;
	}
	COMMTIMEOUTS CommTimeouts;
	if (FALSE == GetCommTimeouts(hFile, &CommTimeouts))
	{
		IO_Device_FormatLastError();
		return false;
	}

	if (FALSE == BuildCommDCBAndTimeouts(lpDef, &dcb, &CommTimeouts))
	{
		IO_Device_FormatLastError();
		return false;
	}

	if (FALSE == SetCommState(hFile, &dcb))
	{
		IO_Device_FormatLastError();
		return false;
	}
	if (FALSE == SetCommTimeouts(hFile, &CommTimeouts))
	{
		IO_Device_FormatLastError();
		return false;
	}
	if (FALSE == SetupComm(hFile, 0UL, 0UL))
	{
		IO_Device_FormatLastError();
		return false;
	}
	if (FALSE == PurgeComm(hFile, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR))
	{
		IO_Device_FormatLastError();
		return false;
	}
#endif // !uncertainty
	*phFile = hFile;
	return true;
}

bool IO_Device_Release_hFile(HANDLE hFile)
{
	if (FALSE == CloseHandle(hFile))
	{
		IO_Device_FormatLastError();
		return false;
	}
	return true;
}

bool IO_Device_Creat_Overlapped(LPOVERLAPPED *lppOverlapped)
{
	LPOVERLAPPED lpOverlapped = (LPOVERLAPPED)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(OVERLAPPED));
	if (NULL == lpOverlapped)
	{
		assert(!(NULL == lpOverlapped));
		return false;
	}

	lpOverlapped->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == lpOverlapped->hEvent)
	{
		IO_Device_FormatLastError();
		return false;
	}
	*lppOverlapped = lpOverlapped;
	return true;
}

bool IO_Device_Release_Overlapped(LPOVERLAPPED lpOverlapped)
{
	if (FALSE == CloseHandle(lpOverlapped->hEvent))
	{
		IO_Device_FormatLastError();
		return false;
	}
	if (FALSE == HeapFree(GetProcessHeap(), 0UL, lpOverlapped))
	{
		IO_Device_FormatLastError();
		return false;
	}
	return true;
}

bool IO_Device_Creat_FileName(PIO_Device *ppIO_Device, LPCTSTR lpFileName, bool Asynchronous, LPCTSTR lpDef)
{
	PIO_Device pIO_Device = (PIO_Device)HeapAlloc(GetProcessHeap(), 0UL, sizeof(IO_Device));
	if (NULL == pIO_Device)
	{
		assert(!(NULL == pIO_Device));
		return false;
	}

	if (false == IO_Device_Creat_hFile(&pIO_Device->hFile, lpFileName, Asynchronous, lpDef))
	{
		IO_Device_FormatLastError();
		return false;
	}
	*ppIO_Device = pIO_Device;
	if (false == Asynchronous)
	{
		pIO_Device->lpOverlappedRead = NULL;
		pIO_Device->lpOverlappedWrite = NULL;
		return true;
	}
	if (false == IO_Device_Creat_Overlapped(&pIO_Device->lpOverlappedRead))
	{
		IO_Device_FormatLastError();
		return false;
	}
	if (false == IO_Device_Creat_Overlapped(&pIO_Device->lpOverlappedWrite))
	{
		IO_Device_FormatLastError();
		return false;
	}
	return true;
}

bool IO_Device_Get_DevicePath(LPCTSTR Hardware_ID, PSP_DEVICE_INTERFACE_DETAIL_DATA *pDeviceInterfaceDetailData)
{
#ifndef _MSC_VER
	GUID GUID_DEVINTERFACE_COMPORT = { 0x86E0D1E0L, 0x8089, 0x11D0, { 0x9C, 0xE4, 0x08, 0x00, 0x3E, 0x30, 0x1F, 0x73 } };
#endif
	HDEVINFO DeviceInfoSet = SetupDiGetClassDevs
	(
		&GUID_DEVINTERFACE_COMPORT,
		NULL,
		NULL,
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
	);
	if (INVALID_HANDLE_VALUE == DeviceInfoSet)
	{
		IO_Device_FormatLastError();
		return false;
	}

	SP_DEVINFO_DATA _DeviceInfoData, *DeviceInfoData = &_DeviceInfoData;
	DeviceInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
	SP_DEVICE_INTERFACE_DATA _DeviceInterfaceData, *DeviceInterfaceData = &_DeviceInterfaceData;
	DeviceInterfaceData->cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	for (DWORD DeviceIndex = 0UL;
		TRUE == SetupDiEnumDeviceInfo
		(
			DeviceInfoSet,
			DeviceIndex,
			DeviceInfoData
		);
		DeviceIndex++)
	{
		DWORD RequiredSize;
		if (FALSE == SetupDiGetDeviceRegistryProperty
		(
			DeviceInfoSet,
			DeviceInfoData,
			SPDRP_HARDWAREID,
			NULL,
			NULL,
			0UL,
			&RequiredSize
		) && ERROR_INSUFFICIENT_BUFFER != GetLastError())
		{
			IO_Device_FormatLastError();
			return false;
		}

		PBYTE PropertyBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), 0UL, RequiredSize);
		if (NULL == PropertyBuffer)
		{
			assert(!(NULL == PropertyBuffer));
			return false;
		}

		if (FALSE == SetupDiGetDeviceRegistryProperty
		(
			DeviceInfoSet,
			DeviceInfoData,
			SPDRP_HARDWAREID,
			NULL,
			PropertyBuffer,
			RequiredSize,
			NULL
		))
		{
			IO_Device_FormatLastError();
			return false;
		}
		bool result = 0 == _tcscmp(Hardware_ID, (LPCTSTR)PropertyBuffer);

		if (FALSE == HeapFree(GetProcessHeap(), 0UL, PropertyBuffer))
		{
			IO_Device_FormatLastError();
			return false;
		}

		if (true == result)
		{
			if (FALSE == SetupDiEnumDeviceInterfaces
			(
				DeviceInfoSet,
				DeviceInfoData,
				&GUID_DEVINTERFACE_COMPORT,
				0UL,
				DeviceInterfaceData
			))
			{
				IO_Device_FormatLastError();
				return false;
			}

			DWORD DeviceInterfaceDetailDataSize;
			if (FALSE == SetupDiGetDeviceInterfaceDetail
			(
				DeviceInfoSet,
				DeviceInterfaceData,
				NULL,
				0UL,
				&DeviceInterfaceDetailDataSize,
				NULL
			) && ERROR_INSUFFICIENT_BUFFER != GetLastError())
			{
				IO_Device_FormatLastError();
				return false;
			}

			PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)
				HeapAlloc(GetProcessHeap(), 0UL, DeviceInterfaceDetailDataSize);
			if (NULL == DeviceInterfaceDetailData)
			{
				assert(!(NULL == DeviceInterfaceDetailData));
				return false;
			}
			DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			if (FALSE == SetupDiGetDeviceInterfaceDetail
			(
				DeviceInfoSet,
				DeviceInterfaceData,
				DeviceInterfaceDetailData,
				DeviceInterfaceDetailDataSize,
				NULL,
				NULL
			))
			{
				IO_Device_FormatLastError();
				return false;
			}
			*pDeviceInterfaceDetailData = DeviceInterfaceDetailData;
			if (FALSE == SetupDiDestroyDeviceInfoList(DeviceInfoSet))
			{
				IO_Device_FormatLastError();
				return false;
			}
			return true;
		}
	}
	IO_Device_FormatLastError();
	return false;
}

bool IO_Device_Creat_Hardware_ID(PIO_Device *ppIO_Device, LPCTSTR Hardware_ID, bool Asynchronous, LPCTSTR lpDef)
{
	PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;
	if (false == IO_Device_Get_DevicePath(Hardware_ID, &DeviceInterfaceDetailData))
	{
		IO_Device_FormatLastError();
		return false;
	}

	if (false == IO_Device_Creat_FileName(ppIO_Device, DeviceInterfaceDetailData->DevicePath, Asynchronous, lpDef))
	{
		IO_Device_FormatLastError();
		return false;
	}

	if (FALSE == HeapFree(GetProcessHeap(), 0UL, DeviceInterfaceDetailData))
	{
		IO_Device_FormatLastError();
		return false;
	}
	return true;
}

bool IO_Device_Release(PIO_Device pIO_Device)
{
	if (false == IO_Device_Release_hFile(pIO_Device->hFile))
	{
		IO_Device_FormatLastError();
		return false;
	}
	if (NULL == pIO_Device->lpOverlappedRead && NULL == pIO_Device->lpOverlappedWrite)
	{
		if (FALSE == HeapFree(GetProcessHeap(), 0UL, pIO_Device))
		{
			IO_Device_FormatLastError();
			return false;
		}
		return true;
	}
	if (false == IO_Device_Release_Overlapped(pIO_Device->lpOverlappedRead))
	{
		IO_Device_FormatLastError();
		return false;
	}
	if (false == IO_Device_Release_Overlapped(pIO_Device->lpOverlappedWrite))
	{
		IO_Device_FormatLastError();
		return false;
	}
	if (FALSE == HeapFree(GetProcessHeap(), 0UL, pIO_Device))
	{
		IO_Device_FormatLastError();
		return false;
	}
	return true;
}

bool IO_Device_Read(PIO_Device pIO_device, LPVOID lpBuffer, DWORD nNumberOfBytesToRead)
{
	DWORD NumberOfBytesRead = 0UL;
	if (FALSE == ReadFile
	(
		pIO_device->hFile,
		lpBuffer,
		nNumberOfBytesToRead,
		NULL == pIO_device->lpOverlappedRead ? &NumberOfBytesRead : NULL,
		pIO_device->lpOverlappedRead
	))
	{
		if (ERROR_IO_PENDING != GetLastError())
		{
			IO_Device_FormatLastError();
		}
		return false;
	}
	if (NULL == pIO_device->lpOverlappedRead && nNumberOfBytesToRead != NumberOfBytesRead)
	{
		IO_Device_FormatLastError();
		return false;
	}
	return true;
}

bool IO_Device_Read_Wait(PIO_Device pIO_device)
{
	if (WAIT_OBJECT_0 != WaitForSingleObject(pIO_device->lpOverlappedRead, INFINITE))
	{
		IO_Device_FormatLastError();
		return false;
	}
	return true;
}

bool IO_Device_Read_Result(PIO_Device pIO_device, LPDWORD lpNumberOfBytesTransferred)
{
	if (FALSE == GetOverlappedResult
	(
		pIO_device->hFile,
		pIO_device->lpOverlappedRead,
		lpNumberOfBytesTransferred,
		FALSE
	))
	{
		if (ERROR_IO_INCOMPLETE != GetLastError())
		{
			IO_Device_FormatLastError();
		}
		return false;
	}
	return true;
}

bool IO_Device_Write(PIO_Device pIO_device, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite)
{
	DWORD NumberOfBytesWritten = 0UL;
	if (FALSE == WriteFile
	(
		pIO_device->hFile,
		lpBuffer,
		nNumberOfBytesToWrite,
		NULL == pIO_device->lpOverlappedWrite ? &NumberOfBytesWritten : NULL,
		pIO_device->lpOverlappedWrite
	))
	{
		if (ERROR_IO_PENDING != GetLastError())
		{
			IO_Device_FormatLastError();
		}
		return false;
	}
	if (NULL == pIO_device->lpOverlappedWrite && nNumberOfBytesToWrite != NumberOfBytesWritten)
	{
		IO_Device_FormatLastError();
		return false;
	}
	return true;
}

bool IO_Device_Write_Wait(PIO_Device pIO_device)
{
	if (WAIT_OBJECT_0 != WaitForSingleObject(pIO_device->lpOverlappedWrite, INFINITE))
	{
		IO_Device_FormatLastError();
		return false;
	}
	return true;
}

bool IO_Device_Write_Result(PIO_Device pIO_device, LPDWORD lpNumberOfBytesTransferred)
{
	if (FALSE == GetOverlappedResult
	(
		pIO_device->hFile,
		pIO_device->lpOverlappedWrite,
		lpNumberOfBytesTransferred,
		FALSE
	))
	{
		if (ERROR_IO_INCOMPLETE != GetLastError())
		{
			IO_Device_FormatLastError();
		}
		return false;
	}
	return true;
}

bool IO_Device_Receiver_Wait(PIO_Device pIO_device)
{
	if (FALSE == SetCommMask(pIO_device->hFile, EV_RXCHAR))
	{
		return false;
	}

	DWORD EvtMask = 0;
	if (FALSE == WaitCommEvent(pIO_device->hFile, &EvtMask, pIO_device->lpOverlappedRead))
	{
		if (NULL == pIO_device->lpOverlappedRead)
		{
			return false;
		}
		else
		{
			if (ERROR_IO_PENDING != GetLastError())
			{
				IO_Device_FormatLastError();
				return false;
			}
			if (WAIT_OBJECT_0 != WaitForSingleObject(pIO_device->lpOverlappedRead, INFINITE))
			{
				IO_Device_FormatLastError();
				return false;
			}
		}
	}
	if (0 == (EV_RXCHAR & EvtMask))
	{
		return false;
	}
	return true;
}

bool IO_Device_Receiver_Number(PIO_Device pIO_device, LPDWORD lpReceived_Number)
{
	COMSTAT Stat;
	if (FALSE == ClearCommError(pIO_device->hFile, NULL, &Stat))
	{
		return false;
	}
	*lpReceived_Number = Stat.cbInQue;
	return true;
}

bool IO_Device_Transmitter_Wait(PIO_Device pIO_device)
{
	if (FALSE == SetCommMask(pIO_device->hFile, EV_TXEMPTY))
	{
		return false;
	}

	DWORD EvtMask = 0;
	if (FALSE == WaitCommEvent(pIO_device->hFile, &EvtMask, pIO_device->lpOverlappedWrite))
	{
		if (NULL == pIO_device->lpOverlappedWrite)
		{
			return false;
		}
		else
		{
			if (ERROR_IO_PENDING != GetLastError())
			{
				IO_Device_FormatLastError();
				return false;
			}
			if (WAIT_OBJECT_0 != WaitForSingleObject(pIO_device->lpOverlappedWrite, INFINITE))
			{
				IO_Device_FormatLastError();
				return false;
			}
		}
	}
	if (0 == (EV_TXEMPTY & EvtMask))
	{
		return false;
	}
	return true;
}

bool IO_Device_Transmitter_Number(PIO_Device pIO_device, LPDWORD lpTransmitted_Number)
{
	COMSTAT Stat;
	if (FALSE == ClearCommError(pIO_device->hFile, NULL, &Stat))
	{
		return false;
	}
	*lpTransmitted_Number = Stat.cbOutQue;
	return true;
}
#endif

#define Arduino_Module
#ifdef Arduino_Module

typedef void *Arduino;

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

bool Arduino_Creat(Arduino *Ptr_arduino, LPCTSTR Hardware_ID, bool Asynchronous, LPCTSTR config);
bool Arduino_Release_(Arduino arduino);

bool Arduino_Read(Arduino arduino, void *buffer, size_t length);
bool Arduino_Write(Arduino arduino, void *buffer, size_t length);

bool Arduino_pinMode(Arduino arduino, uint8_t pin, PinConfigure mode);
bool Arduino_digitalRead(Arduino arduino, uint8_t pin);

bool Arduino_analogWrite(Arduino arduino, uint8_t pin, uint8_t value);

bool Arduino_attachInterrupt(Arduino arduino, uint8_t pin, Triggered mode);
bool Arduino_detachInterrupt(Arduino arduino, uint8_t pin);

bool Arduino_Servo_attach(Arduino arduino, uint8_t pin);
bool Arduino_Servo_writeMicroseconds(Arduino arduino, uint8_t pin, uint16_t microseconds);
bool Arduino_Servo_detach(Arduino arduino, uint8_t pin);

bool Arduino_Wire_begin(Arduino arduino);
bool Arduino_Wire_end(Arduino arduino);
bool Arduino_Wire_beginTransmission(Arduino arduino, uint8_t address);
bool Arduino_Wire_endTransmission(Arduino arduino);
bool Arduino_Wire_write(Arduino arduino, uint8_t *data, uint8_t length);

bool Arduino_Creat(Arduino *Ptr_arduino, LPCTSTR Hardware_ID, bool Asynchronous, LPCTSTR config)
{
	PIO_Device *ppIO_device = (PIO_Device*)ConversionDevice(Ptr_arduino);
	return IO_Device_Creat_Hardware_ID(ppIO_device, Hardware_ID, Asynchronous, config);
}

bool Arduino_Release_(Arduino arduino)
{
	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Release(pIO_device);
}

bool Arduino_Read(Arduino arduino, void *buffer, size_t length)
{
	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Read(pIO_device, buffer, length);
}

bool Arduino_Write(Arduino arduino, void *buffer, size_t length)
{
	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Write(pIO_device, buffer, length);
}

bool Arduino_pinMode(Arduino arduino, uint8_t pin, PinConfigure mode)
{
	struct { uint8_t Decode; uint8_t mode; uint8_t pin; }
	PinMode_pin_mode = { PinMode, mode, pin };

	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Write(pIO_device, &PinMode_pin_mode, sizeof(PinMode_pin_mode));
}

bool Arduino_digitalRead(Arduino arduino, uint8_t pin)
{
	struct { uint8_t Decode; uint8_t pin; }
	Arduino_pin = { DigitalRead, pin };

	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Write(pIO_device, &Arduino_pin, sizeof(Arduino_pin));
}

bool Arduino_analogWrite(Arduino arduino, uint8_t pin, uint8_t value)
{
	struct { uint8_t Decode; uint8_t pin; uint8_t value; }
	Arduino_pin = { AnalogWrite, pin, value };

	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Write(pIO_device, &Arduino_pin, sizeof(Arduino_pin));
}

bool Arduino_attachInterrupt(Arduino arduino, uint8_t pin, Triggered mode)
{
	struct { uint8_t Decode; uint8_t pin; uint8_t mode; }
	Interrupt = { AttachInterrupt, pin, mode };

	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Write(pIO_device, &Interrupt, sizeof(Interrupt));
}

bool Arduino_detachInterrupt(Arduino arduino, uint8_t pin)
{
	struct { uint8_t Decode; uint8_t pin; }
	Interrupt = { DetachInterrupt, pin };

	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Write(pIO_device, &Interrupt, sizeof(Interrupt));
}

bool Arduino_Servo_attach(Arduino arduino, uint8_t pin)
{
	struct { uint8_t Decode; uint8_t pin; }
	Servo = { Servo_attach, pin };

	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Write(pIO_device, &Servo, sizeof(Servo));
}

bool Arduino_Servo_writeMicroseconds(Arduino arduino, uint8_t pin, uint16_t microseconds)
{
	struct { uint8_t Decode; uint8_t pin; uint16_t microseconds; }
	Servo = { Servo_writeMicroseconds, pin, (uint16_t)microseconds };

	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Write(pIO_device, &Servo, sizeof(Servo));
}

bool Arduino_Servo_detach(Arduino arduino, uint8_t pin)
{
	struct { uint8_t Decode; uint8_t pin; }
	Servo = { Servo_detach, pin };

	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Write(pIO_device, &Servo, sizeof(Servo));
}

bool Arduino_Wire_begin(Arduino arduino)
{
	struct { uint8_t Decode; }
	Arduino_Wire = { Wire_Begin };

	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Write(pIO_device, &Arduino_Wire, sizeof(Arduino_Wire));
}

bool Arduino_Wire_end(Arduino arduino)
{
	struct { uint8_t Decode; }
	Arduino_Wire = { Wire_End };

	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Write(pIO_device, &Arduino_Wire, sizeof(Arduino_Wire));
}

bool Arduino_Wire_beginTransmission(Arduino arduino, uint8_t address)
{
	struct { uint8_t Decode; uint8_t address; }
	Arduino_Wire = { Wire_BeginTransmission, address };

	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Write(pIO_device, &Arduino_Wire, sizeof(Arduino_Wire));
}

bool Arduino_Wire_endTransmission(Arduino arduino)
{
	struct { uint8_t Decode; }
	Arduino_Wire = { Wire_EndTransmission };

	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Write(pIO_device, &Arduino_Wire, sizeof(Arduino_Wire));
}

bool Arduino_Wire_write(Arduino arduino, uint8_t *data, uint8_t length)
{
	struct { uint8_t Decode; uint8_t length; }
	Wire_Write_length = { Wire_Write, length };

	PIO_Device pIO_device = ConversionDevice(arduino);
	if (false == IO_Device_Write(pIO_device, &Wire_Write_length, sizeof(Wire_Write_length)))
	{
		return false;
	}
	return IO_Device_Write(pIO_device, data, length);
}

#endif

#define ICAL_Arduino
#ifdef ICAL_Arduino

//#define PL2303_Hardware_ID _T("USB\\VID_067B&PID_2303&REV_0300")
#define Arduino_Uno_Hardware_ID _T("USB\\VID_2341&PID_0043&REV_0001")
#define Arduino_Mega_2560_Hardware_ID _T("USB\\VID_2341&PID_0042&REV_0001")
#define Arduino_Asynchronous false
#define Arduino_config _T("Baud=115200 Parity=E Data=8 Stop=1")
#define MID_MR2x30A 0

bool Arduino_Uno_Creat(Arduino *Ptr_arduino)
{
	return Arduino_Creat(Ptr_arduino, Arduino_Uno_Hardware_ID, Arduino_Asynchronous, Arduino_config);
}

bool Arduino_Mega_2560_Creat(Arduino *Ptr_arduino)
{
	return Arduino_Creat(Ptr_arduino, Arduino_Mega_2560_Hardware_ID, Arduino_Asynchronous, Arduino_config);
}

bool Arduino_Wheel_MR2x30A_Brake(Arduino arduino);
bool Arduino_Release(Arduino arduino)
{
	//if (false == Arduino_Wheel_MR2x30A_Brake(arduino))
	//{
	//	return false;
	//}
	return Arduino_Release_(arduino);
}

bool Arduino_Wheel_MR2x30A_Enable(Arduino arduino)
{
	return Arduino_Wire_begin(arduino);
}

bool Arduino_Wheel_MR2x30A_Disabl(Arduino arduino)
{
	return Arduino_Wire_end(arduino);
}

bool Arduino_Wheel_MR2x30A_Brake(Arduino arduino)
{
#define CID_BrakeDual 101U
	struct
	{
		uint8_t CID;
		uint8_t CheckSum1;
		uint8_t Dummy;
	}BrakeDual =
	{
		CID_BrakeDual,
		UINT8_MAX - (MID_MR2x30A * 2) - CID_BrakeDual
	};

	if (false == Arduino_Wire_beginTransmission(arduino, MID_MR2x30A))
	{
		return false;
	}
	if (false == Arduino_Wire_write(arduino, (uint8_t*)&BrakeDual, sizeof(BrakeDual)))
	{
		return false;
	}
	if (false == Arduino_Wire_endTransmission(arduino))
	{
		return false;
	}
	return true;
}

bool Arduino_Wheel_MR2x30A_SetVel(Arduino arduino, int left, int right)
{
#pragma pack(push)
#pragma pack(1)
#define CID_SetVelAB 118U
	struct
	{
		uint8_t CID;
		uint8_t CheckSum1;
		int16_t left;
		int16_t right;
		uint8_t CheckSum2;
		uint8_t Dummy;
	}SetVelAB =
	{
		CID_SetVelAB,
		UINT8_MAX - (MID_MR2x30A * 2) - CID_SetVelAB,
		(int16_t)left,
		(int16_t)right,
		UINT8_MAX - (((uint8_t*)&left)[0] + ((uint8_t*)&left)[1] + ((uint8_t*)&right)[0] + ((uint8_t*)&right)[1])
	};
#pragma pack(pop)

	if (false == Arduino_Wire_beginTransmission(arduino, MID_MR2x30A))
	{
		return false;
	}
	if (false == Arduino_Wire_write(arduino, (uint8_t*)&SetVelAB, sizeof(SetVelAB)))
	{
		return false;
	}
	if (false == Arduino_Wire_endTransmission(arduino))
	{
		return false;
	}
	return true;
}

bool Arduino_Infrared_Ray_Get(Arduino arduino, int pin, bool *value)
{
	if (false == Arduino_digitalRead(arduino, (uint8_t)pin))
	{
		return false;
	}

	struct { uint8_t Decode; uint8_t pin; uint8_t value; } Infrared_Ray_pin;
	if (false == Arduino_Read(arduino, &Infrared_Ray_pin, sizeof(Infrared_Ray_pin)))
	{
		return false;
	}
	if (DigitalRead != Infrared_Ray_pin.Decode || pin != Infrared_Ray_pin.pin)
	{
		return false;
	}

	*value = Infrared_Ray_pin.value;
	return true;
}

bool Arduino_Infrared_Ray_Event_Enable(Arduino arduino, int pin)
{
	return Arduino_attachInterrupt(arduino, (uint8_t)pin, _CHANGE);
}

bool Arduino_Infrared_Ray_Event_Disable(Arduino arduino, int pin)
{
	return Arduino_detachInterrupt(arduino, (uint8_t)pin);
}

bool Arduino_Infrared_Ray_Event_Wait(Arduino arduino)
{
	PIO_Device pIO_device = ConversionDevice(arduino);
	return IO_Device_Receiver_Wait(pIO_device);
}

bool Arduino_Infrared_Ray_Event_Result(Arduino arduino, int *pin, bool *value)
{
	DWORD Received_Number;
	PIO_Device pIO_device = ConversionDevice(arduino);
	if (false == IO_Device_Receiver_Number(pIO_device, &Received_Number) || 0 == Received_Number)
	{
		return false;
	}

	struct { uint8_t Decode; uint8_t pin; uint8_t value; }Interrupt_pin_value;
	if (false == Arduino_Read(arduino, &Interrupt_pin_value, sizeof(Interrupt_pin_value)))
	{
		return false;
	}
	if (Interrupt != Interrupt_pin_value.Decode)
	{
		return false;
	}

	*pin = Interrupt_pin_value.pin;
	*value = Interrupt_pin_value.value;
	return true;
}

bool Arduino_Robotic_Arm_Enable(Arduino arduino, int pin)
{
	return Arduino_Servo_attach(arduino, (uint8_t)pin);
}

bool Arduino_Robotic_Arm_Disable(Arduino arduino, int pin)
{
	return Arduino_Servo_detach(arduino, (uint8_t)pin);
}

bool Arduino_Robotic_Arm_SA_1283SG_Degree(Arduino arduino, int pin, int Degree)
{
	uint16_t microsecond = Degree;
	return Arduino_Servo_writeMicroseconds(arduino, (uint8_t)pin, microsecond);
}

bool Arduino_Sonar_HC_SR04_Enable(Arduino arduino, int pin)
{
	return Arduino_Servo_attach(arduino, (uint8_t)pin);
}

bool Arduino_Sonar_Disable(Arduino arduino, int pin)
{
	return Arduino_Servo_detach(arduino, (uint8_t)pin);
}

#endif // ICAL_Arduino

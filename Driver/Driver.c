#include "ntddk.h"

//define io control (IOCTL) functions to use it in DispatchDecCTL function
//#define DEVICE_SEND CTL_CODE(device type, number, transefering type, access type)
#define DEVICE_SEND CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_WRITE_DATA)
#define DEVICE_RECV CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_DATA)



UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\mydevice123");
UNICODE_STRING SyLinkName = RTL_CONSTANT_STRING(L"\\??\\mySyLink123");
PDEVICE_OBJECT DeviceObject = NULL;

NTSTATUS DriverDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;

	// Get irp stack location
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(Irp);

	//retrive major fuction type of the irp depends on IO Stack location (irpsp)
	switch (irpsp->MajorFunction)
	{
	case IRP_MJ_CREATE:
		KdPrint(("create request \n"));
		status = STATUS_SUCCESS;
		break;
	case IRP_MJ_CLOSE:
		KdPrint(("close request \n"));
		status = STATUS_SUCCESS;
		break;
	case IRP_MJ_READ:
		KdPrint(("read request \n"));
		status = STATUS_SUCCESS;
		break;
    case IRP_MJ_WRITE:
	    KdPrint(("write request \n"));
		status = STATUS_SUCCESS;
	    break;

	default:
		status = STATUS_INVALID_PARAMETER;
		break;
	}

	//
	// Finish irp
	//
	Irp->IoStatus.Information = 0; // how many bytes we read or write
	Irp->IoStatus.Status = status; // success status to indicate that we successfully compelete this request
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS IoDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	// get irp stack location
	//pointer to current stack location
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;

	// retrive system buffer because ioctl using method buffer
	PVOID Buffer = Irp->AssociatedIrp.SystemBuffer;

	// get the buffer length from irp stack location
	ULONG InBufLength = irpsp->Parameters.DeviceIoControl.InputBufferLength;
	ULONG OutBufLength = irpsp->Parameters.DeviceIoControl.OutputBufferLength;

	//define return lenth
	ULONG returnLength = 0;

	WCHAR* data = L"data sent from driver";

	// switch between read and recive pre-defined CTL_CODE
	switch (irpsp->Parameters.DeviceIoControl.IoControlCode)
	{
	case DEVICE_SEND:
		returnLength = (wcsnlen(Buffer, 511)) * 2;
		break;
	case DEVICE_RECV:
		wcsncpy(Buffer, data, 511); // copy some data to buffer
		returnLength = (wcsnlen(Buffer, 511)) * 2; // get buffer length
		break;
	default:
		status = STATUS_INVALID_PARAMETER;
		break;
	}

	//
	// Finish irp
	//
	Irp->IoStatus.Information = returnLength; // how many bytes we read or write
	Irp->IoStatus.Status = status; // success status to indicate that we successfully compelete this request
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

// unload driver fuction
VOID Unload(PDRIVER_OBJECT DriverObject)
{
	IoDeleteSymbolicLink(&SyLinkName);
	IoDeleteDevice(DeviceObject);

	KdPrint(("Device unloaded \n"));
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    // NTSTATUS variable to record success or failure
    NTSTATUS status = STATUS_SUCCESS;

	DriverObject->DriverUnload = Unload;


	// First , create the device object
	status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN,FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("device creation failed \n"));
	}

	//create symboliclink 
	status = IoCreateSymbolicLink(&SyLinkName, &DeviceName);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("Symboliclink creation failed \n"));
		IoDeleteDevice(DeviceObject);


		return  status;
	}

	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DriverObject->MajorFunction[i] = DriverDispatch;
	}
    
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoDeviceControl;

	KdPrint(("WDM driver :: hello world \n"));

	return status;
}
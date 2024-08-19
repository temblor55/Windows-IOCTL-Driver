#include "ntddk.h"

UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\mydevice123");
UNICODE_STRING SyLinkName = RTL_CONSTANT_STRING(L"\\??\\mySyLink123");
PDEVICE_OBJECT DeviceObject = NULL;

NTSTATUS DispatchPassThru(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;

	// Get irp stack location
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(Irp);

	//retrive major fuction type of the irp depends on IO Stack location (irpsp)
	switch (irpsp->MajorFunction)
	{
	case IRP_MJ_CREATE:
		KdPrint(("create request \n"));
		break;
	case IRP_MJ_CLOSE:
		KdPrint(("close request \n"));
		break;

	case IRP_MJ_READ:
		KdPrint(("read request \n"));
		break;

	default:
		break;
	}



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

		DriverObject->MajorFunction[i] = DispatchPassThru;


	}
    
	KdPrint(("WDM driver :: hello world \n"));


	return status;
}


//NTSTATUS
//DriverEntry(
//    _In_ PDRIVER_OBJECT     DriverObject,
//    _In_ PUNICODE_STRING    RegistryPath
//)
//{
//    // NTSTATUS variable to record success or failure
//    NTSTATUS status = STATUS_SUCCESS;
//
//    // Allocate the driver configuration object
//    WDF_DRIVER_CONFIG config;
//
//    // Print "Hello World" for DriverEntry
//    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KmdfHelloWorld: DriverEntry\n"));
//
//    // Initialize the driver configuration object to register the
//    // entry point for the EvtDeviceAdd callback, KmdfHelloWorldEvtDeviceAdd
//    WDF_DRIVER_CONFIG_INIT(&config,
//        KmdfHelloWorldEvtDeviceAdd
//    );
//
//    // Finally, create the driver object
//    status = WdfDriverCreate(DriverObject,
//        RegistryPath,
//        WDF_NO_OBJECT_ATTRIBUTES,
//        &config,
//        WDF_NO_HANDLE
//    );
//    return status;
//}
#include "emunKernelModule.h"
#include "exapi.h"
#include "CommFunction.h"
#include "struct.h"


extern PLIST_ENTRY PsLoadedModuleList;
extern POBJECT_TYPE *IoDriverObjectType;
extern POBJECT_TYPE *IoDeviceObjectType;

POBJECT_TYPE IoDirectoryObjectType = NULL;




BOOLEAN IsUnicodeStringValid(PUNICODE_STRING unString)
{
	BOOLEAN bRet = FALSE;


	__try
	{
		if (unString->Length > 0 &&
			unString->Buffer		&&
			MmIsAddressValid(unString->Buffer) &&
			MmIsAddressValid(&unString->Buffer[unString->Length / sizeof(WCHAR) - 1]))
		{
			bRet = TRUE;
		}

	}
	__except (1)
	{
		bRet = FALSE;
	}

	return bRet;
}

NTSTATUS EnumDriverByLdrDataTableEntry(PALL_DRIVERS pDriversInfo, ULONG nCnt)
{
	PLDR_DATA_TABLE_ENTRY entry = NULL, firstEntry = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	KIRQL OldIrql;
	ULONG i = 0;
	
	if (!MmIsAddressValid(PsLoadedModuleList) && !pDriversInfo) {

		return STATUS_UNSUCCESSFUL;
	}
	entry = firstEntry = (PLDR_DATA_TABLE_ENTRY)PsLoadedModuleList->Flink;


	OldIrql = KeRaiseIrqlToDpcLevel(); //irq ��DPC ȱҳ�쳣�޷�����  *(ULONG_PTR*)MmHighestUserAddress �ò���
	__try 
	{
		

		do
		{
			if ((ULONG_PTR)entry->DllBase > SYSTEM_ADDRESS_START /**(ULONG_PTR*)MmHighestUserAddress sehҲ�Ȳ��� */ && entry->SizeOfImage > 0)
			{
				ULONG nCurCnt = pDriversInfo->nCnt;
				if (nCnt > nCurCnt)
				{
					pDriversInfo->Drivers[nCurCnt].DriverType = enumHaveOrder;
					pDriversInfo->Drivers[nCurCnt].nLodeOrder = i++;
					pDriversInfo->Drivers[nCurCnt].nBase = (ULONG_PTR)entry->DllBase;
					pDriversInfo->Drivers[nCurCnt].nSize = entry->SizeOfImage;
					

					if (IsUnicodeStringValid(&(entry->FullDllName)))
					{

						RtlCopyMemory(pDriversInfo->Drivers[nCurCnt].szDriverPath, entry->FullDllName.Buffer, entry->FullDllName.MaximumLength > MAX_PATH ? MAX_PATH : entry->FullDllName.MaximumLength);
					}
					else if (IsUnicodeStringValid(&(entry->BaseDllName)))
					{
						RtlCopyMemory(pDriversInfo->Drivers[nCurCnt].szDriverPath, entry->BaseDllName.Buffer, entry->BaseDllName.MaximumLength > MAX_PATH ? MAX_PATH : entry->BaseDllName.MaximumLength);
					}
				}

				pDriversInfo->nCnt++;
			}

			entry = (PLDR_DATA_TABLE_ENTRY)entry->InLoadOrderLinks.Flink;

		} while (entry && entry != firstEntry);

		status = STATUS_SUCCESS;
	}
	__except (1)
	{
		status = GetExceptionCode();
	}

	KeLowerIrql(OldIrql);

	return status;
}



NTSTATUS EnumDriversByWalkerDirectoryObject(PALL_DRIVERS pDriversInfo, ULONG nCnt)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	OBJECT_ATTRIBUTES objectAttributes;
	UNICODE_STRING unDirectory;
	HANDLE hDirectory;
	PVOID pDirectoryObject = NULL;
	WCHAR szDirectory[] = { L'\\', L'\0' };


	RtlInitUnicodeString(&unDirectory, szDirectory);
	InitializeObjectAttributes(&objectAttributes, &unDirectory, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	__try
	{
		/*
		// һ��Ŀ¼���� ����37��DirectoryEnTry ����(dt _OBJECT_DIRECTORY_ENTRY) 
		_OBJECT_DIRECTORY_ENTRY ���Object ָ��ľ��� ������ DriverObject(dt _DRIVER_OBJECT)
		*/
		status = ZwOpenDirectoryObject(&hDirectory, 0, &objectAttributes);
		if (NT_SUCCESS(status))
		{
		
			status = ObReferenceObjectByHandle(hDirectory, 0x10000000, 0, KernelMode, &pDirectoryObject, NULL);
			if (NT_SUCCESS(status))
			{
				IoDirectoryObjectType = (POBJECT_TYPE)ObGetObjectType(pDirectoryObject);//��ȡ ObpDirectoryObjectType
		
				WalkerDirectoryObject(pDriversInfo, pDirectoryObject, nCnt);

			}

			status = ZwClose(hDirectory);
			ObReferenceObject(pDirectoryObject);
		}


	}
	__except (1)
	{

		status = GetExceptionCode();
	}


	return status;
}



BOOLEAN IsAddressValid(PVOID pAddress, ULONG nLen)
{
	BOOLEAN bRet = FALSE;

	__try
	{
		if (nLen == 0)
		{
			bRet = MmIsAddressValid(pAddress);
		}
		else if (nLen > 0)
		{
			ULONG i = 0;
			for (i = 0; i < nLen; i++)
			{
				bRet = MmIsAddressValid((PVOID)((PCHAR)pAddress + i));
				if (bRet == FALSE)
				{
					break;
				}
			}
		}
	}
	__except (1)
	{
		bRet = FALSE;
	}

	return bRet;
}

//
// �ж������Ƿ��Ѿ������������� ����Ϊ DriverObject ��ֵ 
//
BOOLEAN AddDriverObjectToDriverInList(PALL_DRIVERS pDriversInfo, PDRIVER_OBJECT pDriverObject, ULONG nCnt)
{
	BOOLEAN bIn = TRUE, bFind = FALSE;


	if (!pDriversInfo ||
		!pDriverObject ||
		!MmIsAddressValid(pDriverObject))
	{
		return TRUE;
	}

	__try
	{
		if (IsAddressValid(pDriverObject, sizeof(DRIVER_OBJECT)))
		{
			PLDR_DATA_TABLE_ENTRY entry = (PLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;

			if (entry &&
				MmIsAddressValid(entry) &&
				MmIsAddressValid(entry->DllBase) &&
				(ULONG_PTR)entry->DllBase > SYSTEM_ADDRESS_START)
			{
				ULONG i = 0;
				ULONG nCntTemp = nCnt > pDriversInfo->nCnt ? pDriversInfo->nCnt : nCnt;

				for (i = 0; i < nCntTemp; i++)
				{
					if (pDriversInfo->Drivers[i].nBase == (ULONG_PTR)entry->DllBase)
					{
						if (pDriversInfo->Drivers[i].nDriverObject == 0)
						{
							pDriversInfo->Drivers[i].nDriverObject = (ULONG_PTR)pDriverObject;
						}

						bFind = TRUE;
						break;
					}
				}

				if (!bFind)
				{
					bIn = FALSE;
				}
			}
		}
	}
	__except (1)
	{
		bIn = TRUE;
	}

	return bIn;
}



void InsertDriver(PALL_DRIVERS pDriversInfo, PDRIVER_OBJECT pDriverObject, ULONG nCnt)
{

	if (!pDriversInfo || !pDriverObject || !MmIsAddressValid(pDriverObject))
	{
		return;
	}
	else
	{
		__try {
			PLDR_DATA_TABLE_ENTRY entry = (PLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;

			if (entry &&
				MmIsAddressValid(entry) &&
				MmIsAddressValid(entry->DllBase) &&
				(ULONG_PTR)entry->DllBase > SYSTEM_ADDRESS_START)
			{
				ULONG nCurCnt = pDriversInfo->nCnt;
				if (nCnt > nCurCnt)
				{
					pDriversInfo->Drivers[nCurCnt].DriverType = enumHide;
					pDriversInfo->Drivers[nCurCnt].nLodeOrder = 0x10000;
					pDriversInfo->Drivers[nCurCnt].nBase = (ULONG_PTR)entry->DllBase;
					pDriversInfo->Drivers[nCurCnt].nSize = entry->SizeOfImage;
					pDriversInfo->Drivers[nCurCnt].nDriverObject = (ULONG_PTR)pDriverObject;

					if (IsUnicodeStringValid(&(entry->FullDllName)))
					{
						RtlCopyMemory(pDriversInfo->Drivers[nCurCnt].szDriverPath, entry->FullDllName.Buffer, entry->FullDllName.MaximumLength > MAX_PATH ? MAX_PATH : entry->FullDllName.MaximumLength);
					}
					else if (IsUnicodeStringValid(&(entry->BaseDllName)))
					{

						RtlCopyMemory(pDriversInfo->Drivers[nCurCnt].szDriverPath, entry->BaseDllName.Buffer, entry->BaseDllName.MaximumLength > MAX_PATH ? MAX_PATH : entry->BaseDllName.MaximumLength);
					}
				}

				pDriversInfo->nCnt++;
			}
		}
		__except (1)
		{

		}
	}
}



// �ݹ����Ŀ¼���� ���Ҽ��� DriverObject �� �ڴ���
VOID WalkerDirectoryObject(PALL_DRIVERS pDriversInfo, PVOID pDirectoryObject, ULONG nCnt)
{
	if (!pDirectoryObject){return;}
	ULONG i = 0;
	POBJECT_DIRECTORY pOd = (POBJECT_DIRECTORY)pDirectoryObject; //�����windbg����� !obejct pDirectoryObject ������������Ϣ

	KIRQL OldIrql = KeRaiseIrqlToDpcLevel();


	__try
	{

		for (i = 0; i < NUMBER_HASH_BUCKETS; i++)
		{
			/*
                  // һ��Ŀ¼���� ����37��DirectoryEnTry ����(dt _OBJECT_DIRECTORY_ENTRY)
               _OBJECT_DIRECTORY_ENTRY ���Object ָ��ľ��� ������ DriverObject(dt _DRIVER_OBJECT)
             */
			POBJECT_DIRECTORY_ENTRY pode = pOd->HashBuckets[i]; 
			for (; (ULONG_PTR)pode > SYSTEM_ADDRESS_START && MmIsAddressValid(pode); pode = pode->ChainLink)
			{
				if (MmIsAddressValid(pode->Object))
				{
					POBJECT_TYPE pType = (POBJECT_TYPE)ObGetObjectType(pode->Object);

					//PMY_OBJECT_TYPE pp = (PMY_OBJECT_TYPE)pType;

					//
					// �����Ŀ¼����ô�����ݹ����  
					//
					if (pType == IoDirectoryObjectType) //_object_type->name "Directory"   һ��Ŀ¼���� ����37��DirectoryEnTry ����(dt _OBJECT_DIRECTORY_ENTRY)
					{
			
						WalkerDirectoryObject(pDriversInfo, pode->Object, nCnt);
					}

					//
					// �������������
					//
					else if (pType == *IoDriverObjectType) //_object_type->name "Driver" ���� PDRIVER_OBJECT
					{
						PDEVICE_OBJECT DeviceObject = NULL;

						if (!AddDriverObjectToDriverInList(pDriversInfo, (PDRIVER_OBJECT)pode->Object, nCnt))
						{
							InsertDriver(pDriversInfo, (PDRIVER_OBJECT)pode->Object, nCnt);
						}

						//
						// �����豸ջ
						//
						for (DeviceObject = ((PDRIVER_OBJECT)pode->Object)->DeviceObject;
							DeviceObject && MmIsAddressValid(DeviceObject);
							DeviceObject = DeviceObject->AttachedDevice)
						{
							if (!AddDriverObjectToDriverInList(pDriversInfo, DeviceObject->DriverObject, nCnt))
							{
								InsertDriver(pDriversInfo, DeviceObject->DriverObject, nCnt);
							}
						}
					}

					//
					// ������豸����
					//
					else if (pType == *IoDeviceObjectType)//_object_type->name "Device" ���� PDEVICE_OBJECT
					{
						PDEVICE_OBJECT DeviceObject = NULL;
						
						if (!AddDriverObjectToDriverInList(pDriversInfo, ((PDEVICE_OBJECT)pode->Object)->DriverObject, nCnt))
						{
							InsertDriver(pDriversInfo, ((PDEVICE_OBJECT)pode->Object)->DriverObject, nCnt);
						}

						//
						// �����豸ջ
						//
						for (DeviceObject = ((PDEVICE_OBJECT)pode->Object)->AttachedDevice;
							DeviceObject && MmIsAddressValid(DeviceObject);
							DeviceObject = DeviceObject->AttachedDevice)
						{
							if (!AddDriverObjectToDriverInList(pDriversInfo, DeviceObject->DriverObject, nCnt))
							{
								InsertDriver(pDriversInfo, DeviceObject->DriverObject, nCnt);
							}
						}
					}
					/*
					else if (pType == 0xffff9404a329ebc0)
					{
						typedef struct _SYMBOLIC_LINK {
							LARGE_INTEGER CreationTime;
							UNICODE_STRING Link;
						}SYMBOLIC_LINK, *PSYMBOLIC_LINK;

						PSYMBOLIC_LINK p = pode->Object;
					}
					*/

				}
			}
		}
	}
	__except (1)
	{

	}

	KeLowerIrql(OldIrql);
}









VOID HaveDriverUnloadThread(IN PVOID lpParam)
{
	PDRIVER_OBJECT pDriverObject = (PDRIVER_OBJECT)lpParam;
	if (pDriverObject)
	{
		PDRIVER_UNLOAD DriverUnload = pDriverObject->DriverUnload;

		if (DriverUnload)
		{
			DriverUnload(pDriverObject);

			pDriverObject->FastIoDispatch = NULL;
			memset(pDriverObject->MajorFunction, 0, sizeof(pDriverObject->MajorFunction));
			pDriverObject->DriverUnload = NULL;

			ObMakeTemporaryObject(pDriverObject);
			ObfDereferenceObject(pDriverObject);
		}
	}

	PsTerminateSystemThread(STATUS_SUCCESS);
}



VOID NotHaveDriverUnloadThread(IN PVOID lpParam)
{
	PDRIVER_OBJECT pDriverObject = (PDRIVER_OBJECT)lpParam;
	PDEVICE_OBJECT DeviceObject = NULL;

	if (pDriverObject)
	{
		pDriverObject->FastIoDispatch = NULL;
		memset(pDriverObject->MajorFunction, 0, sizeof(pDriverObject->MajorFunction));
		pDriverObject->DriverUnload = NULL;

		DeviceObject = pDriverObject->DeviceObject;

		/*
		ָ����һ���ɣ�ͬһ�������򴴽����豸���󣩣�����еĻ������ڵ���IoCreateDevice ���� IoCreateDeviceSecure�ɹ���I/O���������Զ������豸�б�
		*/
		while (DeviceObject && MmIsAddressValid(DeviceObject))
		{
			IoDeleteDevice(DeviceObject);
			DeviceObject = DeviceObject->NextDevice;
		}

		ObMakeTemporaryObject(pDriverObject);
		ObfDereferenceObject(pDriverObject);
	}

	PsTerminateSystemThread(STATUS_SUCCESS);
}



NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (!IsRealDriverObject(pDriverObject)) return status;


	if (MmIsAddressValid(pDriverObject) &&
		MmIsAddressValid((PVOID)((ULONG_PTR)pDriverObject + sizeof(DRIVER_OBJECT) - 1)))
	{
		BOOLEAN bDriverUnload = FALSE;
		HANDLE hSystemThread = NULL;

		if (pDriverObject->DriverUnload &&
			(ULONG_PTR)pDriverObject->DriverUnload > SYSTEM_ADDRESS_START &&
			MmIsAddressValid(pDriverObject->DriverUnload))
		{
			bDriverUnload = TRUE;
		}

		if (bDriverUnload)
		{
			status = PsCreateSystemThread(&hSystemThread, 0, NULL, NULL, NULL, HaveDriverUnloadThread, pDriverObject);
		}
		else
		{
			status = PsCreateSystemThread(&hSystemThread, 0, NULL, NULL, NULL, NotHaveDriverUnloadThread, pDriverObject);
		}

		if (NT_SUCCESS(status))
		{
			PETHREAD pThread = NULL;
			

			status = ObReferenceObjectByHandle(hSystemThread, 0, NULL, KernelMode, &pThread, NULL);
			if (NT_SUCCESS(status))
			{
				LARGE_INTEGER timeout = { 0 };
				timeout.QuadPart = -10 * 1000 * 1000 * 3;
				status = KeWaitForSingleObject(pThread, Executive, KernelMode, TRUE, &timeout); // �ȴ�3��
				ObfDereferenceObject(pThread);
			}

	
			ZwClose(hSystemThread);
		}
	}

	return status;
}


BOOLEAN IsRealDriverObject(PDRIVER_OBJECT DriverObject)
{
	BOOLEAN bRet = FALSE;

	__try
	{
		if (DriverObject->Type == 4 &&
			DriverObject->Size == sizeof(DRIVER_OBJECT) &&
			ObGetObjectType(DriverObject) == *IoDriverObjectType &&
			MmIsAddressValid(DriverObject->DriverSection) &&
			(ULONG_PTR)DriverObject->DriverSection > SYSTEM_ADDRESS_START &&
			!(DriverObject->DriverSize & 0x1F) &&
			DriverObject->DriverSize < SYSTEM_ADDRESS_START &&
			!((ULONG_PTR)(DriverObject->DriverStart) & 0xFFF) &&
			(ULONG_PTR)DriverObject->DriverStart > SYSTEM_ADDRESS_START
			)
		{
			PDEVICE_OBJECT DeviceObject = DriverObject->DeviceObject;
			if (DeviceObject)
			{
				if (MmIsAddressValid(DeviceObject) &&
					ObGetObjectType(DeviceObject) == *IoDeviceObjectType &&
					DeviceObject->Type == 3 &&
					DeviceObject->Size >= sizeof(DEVICE_OBJECT))
				{
					bRet = TRUE;
				}
			}
			else
			{
				bRet = TRUE;
			}
		}
	}
	__except (1)
	{
		bRet = FALSE;
	}

	return bRet;
}


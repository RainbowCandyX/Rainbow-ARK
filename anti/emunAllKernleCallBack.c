#include "emunAllKernleCallBack.h"

extern ULONG g_ObjectCallbackListOffset;
ULONG64 SearchPspLoadImageNotifyRoutine()
{
	ULONG64 pTemp = 0;
	ULONG64 pCheckArea = NULL;
	ULONG64 i = 0;
	UNICODE_STRING szPsRemoveLoadImageNotifyRoutine = RTL_CONSTANT_STRING(L"PsRemoveLoadImageNotifyRoutine");
	pCheckArea = (ULONG64)MmGetSystemRoutineAddress(&szPsRemoveLoadImageNotifyRoutine);
	if (pCheckArea && MmIsAddressValid((PVOID)pCheckArea) && MmIsAddressValid((PVOID)(pCheckArea + 0xff)))
	{
		for (i = pCheckArea; i < pCheckArea + 0xff; i++)
		{
			__try
			{

				if ((*(PUCHAR)i == 0x48) && (*(PUCHAR)(i + 1) == 0x8d) && (*(PUCHAR)(i + 2) == 0x0d))
				{
					LONG OffsetAddr = 0;
					RtlCopyMemory(&OffsetAddr, (PUCHAR)(i + 3), 4);
					pTemp = OffsetAddr + 7 + i;
					return pTemp;
				}
			}
			__except (1)
			{
				pTemp = NULL;
				break;
			}
		}
	}

	return pTemp;
}


ULONG64 SearchPspCreateThreadNotifyRoutine()
{

	ULONG64 pTemp = 0;
	ULONG64 pCheckArea = NULL;
	ULONG64 i = 0;
	UNICODE_STRING szPsRemoveLoadImageNotifyRoutine = RTL_CONSTANT_STRING(L"PsRemoveCreateThreadNotifyRoutine");
	pCheckArea = (ULONG64)MmGetSystemRoutineAddress(&szPsRemoveLoadImageNotifyRoutine);
	if (pCheckArea && MmIsAddressValid((PVOID)pCheckArea) && MmIsAddressValid((PVOID)(pCheckArea + 0xff)))
	{
		for (i = pCheckArea; i < pCheckArea + 0xff; i++)
		{
			__try
			{

				if ((*(PUCHAR)i == 0x48) && (*(PUCHAR)(i + 1) == 0x8d) && (*(PUCHAR)(i + 2) == 0x0d))
				{
					LONG OffsetAddr = 0;
					RtlCopyMemory(&OffsetAddr, (PUCHAR)(i + 3), 4);
					pTemp = OffsetAddr + 7 + i;
					return pTemp;
				}
			}
			__except (1)
			{
				pTemp = NULL;
				break;
			}
		}
	}

	return pTemp;
}

//CmUnRegisterCallback
ULONG64 SearchCallbackListHead()
{
	ULONG64 pTemp = 0;
	ULONG64 pCheckArea = NULL;
	ULONG64 i = 0;
	UNICODE_STRING szPsRemoveLoadImageNotifyRoutine = RTL_CONSTANT_STRING(L"CmUnRegisterCallback");
	pCheckArea = (ULONG64)MmGetSystemRoutineAddress(&szPsRemoveLoadImageNotifyRoutine);
	if (pCheckArea && MmIsAddressValid((PVOID)pCheckArea) && MmIsAddressValid((PVOID)(pCheckArea + 0xff)))
	{
		for (i = pCheckArea; i < pCheckArea + 0xff; i++)
		{
			__try
			{

				if ((*(PUCHAR)i == 0x48) && (*(PUCHAR)(i + 1) == 0x8d) && (*(PUCHAR)(i + 2) == 0x0d))
				{
					LONG OffsetAddr = 0;
					RtlCopyMemory(&OffsetAddr, (PUCHAR)(i + 3), 4);
					pTemp = OffsetAddr + 7 + i;
					return pTemp;
				}
			}
			__except (1)
			{
				pTemp = NULL;
				break;
			}
		}
	}

	return pTemp;
}



BOOLEAN EnumObCallbacks(POB_CALLBACK p, ULONG nSize)
{
	ULONG i = 0;
	PLIST_ENTRY CurrEntry = NULL;
	POB_CALLBACK pObCallback = NULL;
	ULONG64 ObProcessCallbackListHead = *(ULONG64*)PsProcessType + g_ObjectCallbackListOffset;
	ULONG64 ObThreadCallbackListHead = *(ULONG64*)PsThreadType + g_ObjectCallbackListOffset;
	//
	if (!MmIsAddressValid((PVOID)ObProcessCallbackListHead) && !MmIsAddressValid((PVOID)ObThreadCallbackListHead)
		&& !MmIsAddressValid((*(PULONG64)(ObProcessCallbackListHead)) && !MmIsAddressValid((PVOID)(*(PULONG64)(ObThreadCallbackListHead)))))
	{
		return NULL;
	}

	CurrEntry = ((PLIST_ENTRY)ObProcessCallbackListHead)->Flink;	//list_head���������������ݣ�����


	do
	{

		pObCallback = (POB_CALLBACK)CurrEntry;
		if (pObCallback->ObHandle != 0)
		{
			if (i > nSize)break;

			if (MmIsAddressValid(pObCallback->ObjTypeAddr)) 
			{
				p[i].ObHandle = pObCallback->ObHandle;
				p[i].PreCall = pObCallback->PreCall;
				p[i].PostCall = pObCallback->PostCall;
				p[i].ObjTypeAddr = pObCallback->ObjTypeAddr;
				p[i].Unknown = pObCallback->Unknown;
				p[i].bType = TRUE;
				i++;
			}
		}
		CurrEntry = CurrEntry->Flink;
	} while (CurrEntry != (PLIST_ENTRY)ObProcessCallbackListHead);

	CurrEntry = ((PLIST_ENTRY)ObThreadCallbackListHead)->Flink;	//list_head���������������ݣ�����
	do
	{
		pObCallback = (POB_CALLBACK)CurrEntry;
		if (pObCallback->ObHandle != 0)
		{
			if (i > nSize)break;
			if (MmIsAddressValid(pObCallback->ObjTypeAddr)) 
			{
				p[i].ObHandle = pObCallback->ObHandle;
				p[i].PreCall = pObCallback->PreCall;
				p[i].PostCall = pObCallback->PostCall;
				p[i].ObjTypeAddr = pObCallback->ObjTypeAddr;
				p[i].Unknown = pObCallback->Unknown;
				p[i].bType = FALSE;
				i++;
			}

		}
		CurrEntry = CurrEntry->Flink;
	} while (CurrEntry != (PLIST_ENTRY)ObThreadCallbackListHead);

	return TRUE;
}


ULONG64 FindPspSetCreateProcessNotifyRoutine(ULONG64 pCheckArea)
{
	ULONG64 i = 0;
	if (MmIsAddressValid(pCheckArea) && MmIsAddressValid(pCheckArea + 0xff))
	{
		for (i = pCheckArea; i < pCheckArea + 0xff; i++)
		{
			if (*(PUCHAR)i == 0x4c && *(PUCHAR)(i + 1) == 0x8d && *(PUCHAR)(i + 2) == 0x2d)	//lea     r13,[nt!PspCreateProcessNotifyRoutine 
			{
				LONG OffsetAddr = 0;
				memcpy(&OffsetAddr, (PUCHAR)(i + 3), 4);
				return OffsetAddr + 7 + i;
			}
		}
	}
	return NULL;
}


ULONG64 FindPspCreateProcessNotifyRoutine()
{
	LONG	OffsetAddr = 0;
	ULONG64	i = 0, pCheckArea = 0;
	UNICODE_STRING	unstrFunc;
	PVOID pTemp = NULL;
	//���PsSetCreateProcessNotifyRoutine�ĵ�ַ
	RtlInitUnicodeString(&unstrFunc, L"PsSetCreateProcessNotifyRoutine");
	pCheckArea = (ULONG64)MmGetSystemRoutineAddress(&unstrFunc);
	if (pCheckArea)
	{

		//���PspCreateProcessNotifyRoutine�ĵ�ַ
		if (MmIsAddressValid(pCheckArea) && MmIsAddressValid(pCheckArea+0xff))
		{
			for (i = pCheckArea; i < pCheckArea + 0xff; i++)
			{
				if (*(PUCHAR)i == 0xe8)	//lea r14,xxxx
				{
					LONG OffsetAddr = 0;
					memcpy(&OffsetAddr, (PUCHAR)(i + 1), 4);
					
					pTemp = OffsetAddr + 5 + i;

					return FindPspSetCreateProcessNotifyRoutine(pTemp);
				}
			}
		}
	}
	return 0;
}


void EnumCreateProcessNotify(PVOID* pBuffer,ULONG nSize)
{
	int i = 0;
	BOOLEAN b;
	ULONG64 NotifyAddr = 0, MagicPtr = 0;
	ULONG64 PspCreateProcessNotifyRoutine = FindPspCreateProcessNotifyRoutine();
	if (!PspCreateProcessNotifyRoutine)
		return;
	for (i = 0; i < 64; i++)
	{
		MagicPtr = PspCreateProcessNotifyRoutine + i * 8;
		NotifyAddr = *(PULONG64)(MagicPtr);
		if (MmIsAddressValid((PVOID)NotifyAddr) && NotifyAddr != 0)
		{
			
			NotifyAddr = *(PULONG64)(NotifyAddr & 0xfffffffffffffff8);

			if (MmIsAddressValid(NotifyAddr)) {
				pBuffer[i] = NotifyAddr;
			}
		}

		if (i > nSize)break;
	}
}



void EnumCreateThreadNotify(PVOID* pBuffer, ULONG nSize)
{
	int i = 0;
	BOOLEAN b;
	ULONG64 NotifyAddr = 0, MagicPtr = 0;
	ULONG64 PspCreateThreadNotifyRoutine = SearchPspCreateThreadNotifyRoutine();
	if (!PspCreateThreadNotifyRoutine)
		return;
	for (i = 0; i < 64; i++)
	{
		MagicPtr = PspCreateThreadNotifyRoutine + i * 8;
		NotifyAddr = *(PULONG64)(MagicPtr);
		if (MmIsAddressValid((PVOID)NotifyAddr) && NotifyAddr != 0)
		{

			NotifyAddr = *(PULONG64)(NotifyAddr & 0xfffffffffffffff8);

			if (MmIsAddressValid(NotifyAddr)) {
				pBuffer[i] = NotifyAddr;
			}
		}

		if (i > nSize)break;
	}
}


void EnumLoadImageNotify(PVOID* pBuffer, ULONG nSize)
{
	int i = 0;
	BOOLEAN b;
	ULONG64 NotifyAddr = 0, MagicPtr = 0;
	ULONG64 PspLoadImageNotifyRoutine = SearchPspLoadImageNotifyRoutine();
	if (!PspLoadImageNotifyRoutine)
		return;

	for (i = 0; i < 64; i++)
	{
		MagicPtr = PspLoadImageNotifyRoutine + i * 8;
		NotifyAddr = *(PULONG64)(MagicPtr);
		if ( MmIsAddressValid((PVOID)NotifyAddr) && NotifyAddr != 0)
		{

			NotifyAddr = *(PULONG64)(NotifyAddr & 0xfffffffffffffff8);

			if (MmIsAddressValid(NotifyAddr)) {
				pBuffer[i] = NotifyAddr;
			}
		}

		if (i > nSize)break;
	}
}


ULONG CountCmpCallbackAfterXP(PVOID* pBuffer,ULONG nSize)
{
	ULONG sum = 0;
	ULONG64 dwNotifyItemAddr = 0;;
	ULONG64* pNotifyFun = NULL;
	ULONG64* baseNotifyAddr = NULL;
	ULONG64 dwNotifyFun = 0;
	LARGE_INTEGER cmpCookie = {0};
	PLIST_ENTRY notifyList = NULL;
	PCM_NOTIFY_ENTRY notify = NULL;
	dwNotifyItemAddr = SearchCallbackListHead();
	notifyList = (LIST_ENTRY *)dwNotifyItemAddr;

	if (!MmIsAddressValid(notifyList)) return NULL;

	do
	{
		notify = (CM_NOTIFY_ENTRY *)notifyList;
		if (MmIsAddressValid(notify))
		{
			if (MmIsAddressValid((PVOID)(notify->Function)) && notify->Function >
				0x8000000000000000)
			{
			
				if (sum > 0) {
					if (MmIsAddressValid((PVOID)(notify->Function))) {
						pBuffer[sum] = (PVOID)(notify->Function);
					}
					if (nSize < sum)break;
				}

				sum++;
			}
		}
			notifyList = notifyList->Flink;
	} while (notifyList != ((LIST_ENTRY*)(dwNotifyItemAddr)));
	return sum;
}




#pragma once
#include <vector>
#include <algorithm>
#include "Function.h"
#include "Common.h"
using namespace std;
class CListDrivers
{
public:
	CListDrivers();
	~CListDrivers();
	BOOL ListDrivers(vector<DRIVER_INFO> &vectorDrivers);
	BOOL UnLoadDriver(ULONG_PTR DriverObject);
private:
	void FixDriverPath(PDRIVER_INFO pDriverInfo);
};

/*
LoadedModuleList������DriverEntry����û�����ģ�ֱ�ӱ��������Ϲ淶�����������������û�Ƶ������ж�������ģ�
�ַ����������㿴�������ΪʲôVMP������LoadedModuleList����ʼ��������ҪZwQueryһ�µ�ԭ��
*/
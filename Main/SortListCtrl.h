#pragma once
#include <afxcmn.h>

#define WM_CUSTOMER_MY					WM_USER + 1000

// ������ص���Ϣ
#define WM_RESIZE_ALL_PROC_WND			WM_CUSTOMER_MY + 1		// ���·ֲ������б�����ĸ�������
#define WM_CLICK_LIST_HEADER_START		WM_CUSTOMER_MY + 2		// ��ʼ����б�ͷ
#define WM_CLICK_LIST_HEADER_END		WM_CUSTOMER_MY + 3		// ��������б�ͷ


#ifndef _SORTLISTCTRL_INCLUDE__
#define _SORTLISTCTRL_INCLUDE__

/////////////////////////////////////////////////////////////////////////////////////////
//һ�������ͷʱ����������б���
class CSortListCtrl : public CListCtrl
{
	// Construction
public:
	CSortListCtrl();
	void EnableSort(BOOL bEnable = TRUE) { m_bEnableSort = bEnable; };
	// Attributes
public:
	struct Info
	{
		CSortListCtrl* pListCtrl;
		int nSubItem;
		BOOL bAsc;		//�Ƿ�������
		BOOL bIsNumber;	//�����Ƿ�������
	};

	// Summary: �ı���ͷͼ�������״̬���������µ�����״̬��bClearΪ��ʱ�����ͷͼ�ꡣ
	BOOL ChangeHeardCtrlState(CHeaderCtrl* pHeardCtrl, int nItem, BOOL bClear);

	static int CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	// Operations
public:

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CSortListCtrl)
		//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CSortListCtrl();

	// Generated message map functions
protected:
	CBitmap* MakeColorBoxImage(BOOL bUp);
	CImageList m_ImageList;
	BOOL m_bInit;
	BOOL m_bEnableSort;
	CBitmap *m_pBmp[2];

	//{{AFX_MSG(CSortListCtrl)
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
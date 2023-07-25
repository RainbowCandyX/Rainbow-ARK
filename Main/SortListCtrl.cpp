#include "SortListCtrl.h"

#include "sortlistctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL IsNumber(LPCTSTR szText)
{
	CString sText;
	int nLen, nPotCount = 0;

	if (szText == NULL)
	{
		return FALSE;
	}

	sText = szText;
	sText.TrimRight();
	nLen = sText.GetLength();
	if (nLen <= 0)
	{
		return FALSE;
	}

	if ((sText[0] == _T('-')) || (sText[0] == _T('+')))
	{
		sText = sText.Mid(1);
		nLen--;
	}

	for (int i = 0; i < nLen; i++)
	{
		if ((sText[i] >= _T('0')) && (sText[i] <= _T('9')))
		{
			continue;
		}

		if (sText[i] == _T('.'))
		{
			if (++nPotCount <= 1)
			{
				continue;
			}
		}

		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSortListCtrl

CSortListCtrl::CSortListCtrl()
{
	m_bInit = FALSE;
	m_bEnableSort = TRUE;
	m_pBmp[0] = m_pBmp[1] = NULL;
}

CSortListCtrl::~CSortListCtrl()
{
	if (m_pBmp[0])
		delete m_pBmp[0];

	if (m_pBmp[1])
		delete m_pBmp[1];
}

BEGIN_MESSAGE_MAP(CSortListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CSortListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	//}}AFX_MSG_MAP

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSortListCtrl message handlers
//������ͷʱ��������
void CSortListCtrl::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if (m_bEnableSort == FALSE)
	{
		return;
	}

	CWnd *pWnd = GetParent();
	pWnd->SendMessage(WM_CLICK_LIST_HEADER_START);

	//������ͷ��ʾ�������ͼ��
	if (m_bInit == FALSE)
	{
		m_bInit = TRUE;

		m_ImageList.Create(8, 8, ILC_COLOR24 | ILC_MASK, 2, 0);
		m_ImageList.Add(m_pBmp[0] = MakeColorBoxImage(TRUE), RGB(0, 0, 255));
		m_ImageList.Add(m_pBmp[1] = MakeColorBoxImage(FALSE), RGB(0, 0, 255));
		GetHeaderCtrl()->SetImageList(&m_ImageList);
	}

	CHeaderCtrl *pHeardCtrl = GetHeaderCtrl();

	int i;

	//��շǵ�ǰ�е�����ͼ��
	for (i = pHeardCtrl->GetItemCount() - 1; i >= 0; i--)
	{
		if (i != pNMListView->iSubItem)
			ChangeHeardCtrlState(pHeardCtrl, i, TRUE);
	}

	Info SortInfo;
	SortInfo.pListCtrl = this;
	SortInfo.nSubItem = pNMListView->iSubItem;

	//�õ���ǰ�е�������
	SortInfo.bAsc = ChangeHeardCtrlState(pHeardCtrl, pNMListView->iSubItem, FALSE);

	//����������itemData
	int nItemCount;
	CString sVal;
	DWORD_PTR dwData;
	CArray<DWORD_PTR> itemDatas;

	nItemCount = GetItemCount();
	SortInfo.bIsNumber = TRUE;
	for (i = 0; i < nItemCount; i++)
	{
		if (SortInfo.bIsNumber)
		{
			sVal = GetItemText(i, pNMListView->iSubItem);
			if ((sVal.IsEmpty() == FALSE) && (IsNumber(sVal) == FALSE))
			{
				SortInfo.bIsNumber = FALSE;
			}
		}

		dwData = GetItemData(i);
		itemDatas.Add(dwData);
		SetItemData(i, i);
	}

	//���ֻ��һ��,ListCtrl��������ȷ�����������һ��ʱ�У�����ListCtrl��һ��BUG��
	BOOL bOnlyOneColumn = FALSE;
	if (pHeardCtrl->GetItemCount() == 1)
	{
		InsertColumn(1, _T(""));
		bOnlyOneColumn = TRUE;
	}

	//����
	SortItems(MyCompareProc, (DWORD_PTR)&SortInfo);

	if (bOnlyOneColumn)
	{
		DeleteColumn(1);//ȥ����ʱ��
	}

	//�ָ�itemData
	for (i = 0; i < nItemCount; i++)
	{
		dwData = GetItemData(i);
		SetItemData(i, itemDatas[dwData]);
	}

	pWnd->SendMessage(WM_CLICK_LIST_HEADER_END);
}

int CALLBACK CSortListCtrl::MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	Info *pSortInfo;
	pSortInfo = (Info*)lParamSort;

	//lParam1��lParam2ʵ������itemData����ΪOnColumnclick�����ù�������ֱ�ӵ���������ʹ�á�
	//�����������������գ���Ϊ�б�������������������ܾͱ仯�ˣ���ʵ��ʹ����δ�������⣬����MS
	//�����������û�н����У������������Ž����ˡ�
	CString strItem1 = pSortInfo->pListCtrl->GetItemText((int)lParam1, pSortInfo->nSubItem);
	CString strItem2 = pSortInfo->pListCtrl->GetItemText((int)lParam2, pSortInfo->nSubItem);

	if (pSortInfo->bIsNumber)
	{
		//����
		double dV = _tstof(strItem1) - _tstof(strItem2);
		if (dV == 0)
			return 0;

		if (pSortInfo->bAsc)
			return (dV < 0 ? -1 : 1);
		else
			return (dV < 0 ? 1 : -1);
	}
	else
	{
		if (pSortInfo->bAsc)
			return strItem1.CompareNoCase(strItem2);
		else
			return strItem2.CompareNoCase(strItem1);
	}
}


BOOL CSortListCtrl::ChangeHeardCtrlState(CHeaderCtrl* pHeardCtrl, int nItem, BOOL bClear)
{
	HDITEM hdi;
	hdi.mask = HDI_IMAGE | HDI_LPARAM | HDI_FORMAT;

	pHeardCtrl->GetItem(nItem, &hdi);
	BOOL bASC = (hdi.lParam == 0 ? 1 : 0);

	if (!bClear)
		hdi.lParam = bASC;

	if (bClear)
	{
		hdi.fmt &= ~HDF_BITMAP_ON_RIGHT;
		hdi.fmt &= ~HDF_IMAGE;
		hdi.mask &= ~HDI_IMAGE;
	}
	else
	{
		hdi.iImage = (bASC ? 0 : 1);
		hdi.fmt |= HDF_BITMAP_ON_RIGHT | HDF_IMAGE;
	}

	pHeardCtrl->SetItem(nItem, &hdi);

	return bASC;
}

//���ɱ�ͷ��"���ϼ�ͷ"��"���¼�ͷ"����λͼ
CBitmap* CSortListCtrl::MakeColorBoxImage(BOOL bUp)
{
	CBitmap* pBitmap = new CBitmap;

	CClientDC cdc(this);
	CDC dc;
	dc.Attach(cdc.m_hDC);


	pBitmap->CreateCompatibleBitmap(&dc, 8, 8);

	CDC tempDC;
	tempDC.CreateCompatibleDC(&dc);

	CBitmap* pOldBitmap;
	pOldBitmap = tempDC.SelectObject(pBitmap);

	CPen penLight, penShadow, *pOldPen;

	tempDC.FillSolidRect(0, 0, 8, 8, RGB(0, 0, 255));

	if (bUp == TRUE)
	{
		static POINT s_PtShadow[9] = { {3,0},{3,1},{2,1},{2,2},{2,3},{1,3},{1,4},{1,5},{0,5} };
		static POINT s_PtLight[9] = { {4,0},{4,1},{5,1},{5,2},{5,3},{6,3},{6,4},{6,5},{7,5} };

		penLight.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));

		pOldPen = tempDC.SelectObject(&penLight);
		tempDC.MoveTo(0, 6);
		tempDC.LineTo(7, 6);
		int i;
		for (i = 0; i < 9; i++)
		{
			tempDC.SetPixel(s_PtLight[i], RGB(255, 255, 255));
		}

		for (i = 0; i < 9; i++)
		{
			tempDC.SetPixel(s_PtShadow[i], RGB(100, 100, 100));
		}

		tempDC.SelectObject(pOldPen);
	}
	else if (bUp == FALSE)
	{
		static POINT s_PtShadow[9] = { {0,1},{1,1},{1,2},{1,3},{2,3},{2,4},{2,5},{3,5},{3,6} };
		static POINT s_PtLight[9] = { {7,1},{6,1},{6,2},{6,3},{5,3},{5,4},{5,5},{3,5},{4,6} };

		penShadow.CreatePen(PS_SOLID, 1, RGB(100, 100, 100));

		pOldPen = tempDC.SelectObject(&penShadow);
		tempDC.MoveTo(7, 0);
		tempDC.LineTo(0, 0);

		int i;
		for (i = 0; i < 9; i++)
		{
			tempDC.SetPixel(s_PtLight[i], RGB(255, 255, 255));
		}

		for (i = 0; i < 9; i++)
		{
			tempDC.SetPixel(s_PtShadow[i], RGB(100, 100, 100));
		}

		tempDC.SelectObject(pOldPen);
	}

	tempDC.SelectObject(pOldBitmap);
	dc.Detach();

	return pBitmap;
}
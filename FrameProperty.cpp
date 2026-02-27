// FrameProperty.cpp: 구현 파일
//

#include "pch.h"
#include "AniMaker.h"
#include "afxdialogex.h"
#include "FrameProperty.h"


// CFrameProperty 대화 상자

IMPLEMENT_DYNAMIC(CFrameProperty, CDialogEx)

CFrameProperty::CFrameProperty(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FRAME_PROPERTY, pParent)
{

}

CFrameProperty::~CFrameProperty()
{
}

void CFrameProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DELAY, m_edit_delay);
}


BEGIN_MESSAGE_MAP(CFrameProperty, CDialogEx)
	ON_BN_CLICKED(IDOK, &CFrameProperty::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFrameProperty::OnBnClickedCancel)
END_MESSAGE_MAP()


// CFrameProperty 메시지 처리기

BOOL CFrameProperty::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_edit_delay.set_text(m_frame_delay);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CFrameProperty::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString text = m_edit_delay.get_text();
	
	m_frame_delay = _ttoi(text);
	if (m_frame_delay <= 0)
	{
		m_edit_delay.SetSel(0, -1);
		m_edit_delay.SetFocus();
		return;
	}

	CDialogEx::OnOK();
}

void CFrameProperty::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnCancel();
}

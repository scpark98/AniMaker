// PreviewDlg.cpp: 구현 파일
//

#include "pch.h"
#include "AniMaker.h"
#include "afxdialogex.h"
#include "PreviewDlg.h"

#include "Common/Functions.h"

// CPreviewDlg 대화 상자

IMPLEMENT_DYNAMIC(CPreviewDlg, CDialogEx)

CPreviewDlg::CPreviewDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PREVIEW, pParent)
{

}

CPreviewDlg::~CPreviewDlg()
{
}

void CPreviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPreviewDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CPreviewDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CPreviewDlg::OnBnClickedCancel)
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CPreviewDlg 메시지 처리기

BOOL CPreviewDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_resize.Create(this);

	m_imgDlg.create(this);

	RestoreWindowPosition(&theApp, this, _T("PreviewDlg"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CPreviewDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnOK();
}

void CPreviewDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnCancel();
}

void CPreviewDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CDialogEx::OnWindowPosChanged(lpwndpos);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	SaveWindowPosition(&theApp, this, _T("PreviewDlg"));
}

void CPreviewDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if (m_imgDlg.m_hWnd == NULL)
		return;

	CRect rc;
	GetClientRect(rc);

	m_imgDlg.MoveWindow(rc);
}

BOOL CPreviewDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	return FALSE;
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CPreviewDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CDialogEx::OnPaint()을(를) 호출하지 마십시오.
}

void CPreviewDlg::set_image(CSCD2Image* pImg)
{
	m_img = pImg;
	m_imgDlg.set_image(pImg);
}
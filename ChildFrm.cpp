
// ChildFrm.cpp: CChildFrame 클래스의 구현
//

#include "pch.h"
#include "framework.h"
#include "AniMaker.h"

#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

// CChildFrame 생성/소멸

CChildFrame::CChildFrame() noexcept
{
	// TODO: 여기에 멤버 초기화 코드를 추가합니다.
}

CChildFrame::~CChildFrame()
{
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서 Window 클래스 또는 스타일을 수정합니다.
	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	// 저장된 ChildFrame 크기 복원
	CWinApp* pApp = AfxGetApp();
	int left = pApp->GetProfileInt(_T("ChildFrame"), _T("left"), CW_USEDEFAULT);

	if (left != CW_USEDEFAULT)
	{
		cs.x = pApp->GetProfileInt(_T("ChildFrame"), _T("left"), 0);
		cs.y = pApp->GetProfileInt(_T("ChildFrame"), _T("top"), 0);
		cs.cx = pApp->GetProfileInt(_T("ChildFrame"), _T("right"), 0) - cs.x;
		cs.cy = pApp->GetProfileInt(_T("ChildFrame"), _T("bottom"), 0) - cs.y;
	}

	return TRUE;
}

// CChildFrame 진단

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}
#endif //_DEBUG

// CChildFrame 메시지 처리기

void CChildFrame::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	GetWindowPlacement(&wp);

	CWinApp* pApp = AfxGetApp();
	pApp->WriteProfileInt(_T("ChildFrame"), _T("showCmd"), wp.showCmd);
	pApp->WriteProfileInt(_T("ChildFrame"), _T("left"), wp.rcNormalPosition.left);
	pApp->WriteProfileInt(_T("ChildFrame"), _T("top"), wp.rcNormalPosition.top);
	pApp->WriteProfileInt(_T("ChildFrame"), _T("right"), wp.rcNormalPosition.right);
	pApp->WriteProfileInt(_T("ChildFrame"), _T("bottom"), wp.rcNormalPosition.bottom);

	CMDIChildWnd::OnClose();
}

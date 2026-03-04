
// MainFrm.cpp: CMainFrame 클래스의 구현
//

#include "pch.h"
#include "framework.h"
#include "AniMaker.h"
#include "AniMakerView.h"

#include "MainFrm.h"

#include <Uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_WM_DROPFILES()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_CHECK_CHILD_FRAMES, &CMainFrame::OnCheckChildFrames)
	ON_WM_CONTEXTMENU()
	ON_WM_SIZE()
END_MESSAGE_MAP()

//ID_SEPARATOR의 갯수는 .h에 정의된 enum status_id의 갯수와 일치해야 한다.
static UINT indicators[] =
{
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
};

// CMainFrame 생성/소멸

CMainFrame::CMainFrame() noexcept
{
	// TODO: 여기에 멤버 초기화 코드를 추가합니다.
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// ★ 비주얼 스타일 설정 (원하는 테마 선택)
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	// 다른 옵션: CMFCVisualManagerVS2008, CMFCVisualManagerWindows, etc.

	// ★ CMFCToolBar 생성
	if (!m_wndToolBar.CreateEx(this,
		TBSTYLE_FLAT,
		WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("도구 모음을 만들지 못했습니다.\n");
		return -1;
	}

	// ★ CMFCStatusBar 생성
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("상태 표시줄을 만들지 못했습니다.\n");
		return -1;
	}

	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
	m_wndStatusBar.SetPaneInfo(status_default, ID_SEPARATOR, SBPS_STRETCH, 240);
	m_wndStatusBar.SetPaneInfo(status_progress, ID_SEPARATOR, SBPS_NORMAL, 100);
	m_wndStatusBar.SetPaneInfo(status_image_info, ID_SEPARATOR, SBPS_NORMAL, 140);
	m_wndStatusBar.SetPaneInfo(status_duration_info, ID_SEPARATOR, SBPS_NORMAL, 100);
	m_wndStatusBar.SetPaneInfo(status_zoom_info, ID_SEPARATOR, SBPS_NORMAL, 45);
	m_wndStatusBar.SetPaneInfo(status_selected_info, ID_SEPARATOR, SBPS_NORMAL, 45);

	CRect rc;
	m_wndStatusBar.GetItemRect(status_progress, &rc);
	m_progress.Create(WS_CHILD | WS_VISIBLE, rc, &m_wndStatusBar, 1);
	m_progress.SetRange32(0, 100);
	/*
	m_progress.ModifyStyleEx(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE, 0, SWP_FRAMECHANGED);
	HWND h = m_progress.GetSafeHwnd();
	LONG ex = ::GetWindowLong(h, GWL_EXSTYLE);
	ex &= ~(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	::SetWindowLong(h, GWL_EXSTYLE, ex);
	::SetWindowPos(h, nullptr, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	::SetWindowTheme(m_progress.GetSafeHwnd(), L"Explorer", nullptr);
	*/

	// ★ 도킹 활성화 (CMFCToolBar 방식)
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndToolBar);  // DockControlBar → DockPane

	m_wndToolBar.SetSizes(CSize(32, 32), CSize(16, 16));
	// 툴바 커스터마이징 비활성화 (사용자가 버튼을 변경하지 못하게)
	CMFCToolBar::SetCustomizeMode(FALSE);

	// 툴바를 고정하여 이동 불가하게 설정 (선택사항)
	//m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() &
	//	~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	DragAcceptFiles();

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return TRUE;
}

// CMainFrame 진단

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame 메시지 처리기
void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
	UINT nFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, nullptr, 0);

	for (UINT i = 0; i < nFiles; ++i)
	{
		TCHAR szFileName[MAX_PATH];
		DragQueryFile(hDropInfo, i, szFileName, MAX_PATH);

		// DocTemplate을 통해 새 Doc/ChildFrame/View 생성
		POSITION pos = AfxGetApp()->GetFirstDocTemplatePosition();
		CDocTemplate* pTemplate = AfxGetApp()->GetNextDocTemplate(pos);
		if (!pTemplate)
			continue;

		CDocument* pDoc = pTemplate->OpenDocumentFile(nullptr); // 빈 문서 생성
		if (!pDoc)
			continue;

		// 새로 생성된 View 찾기
		POSITION viewPos = pDoc->GetFirstViewPosition();
		CView* pView = pDoc->GetNextView(viewPos);
		CAniMakerView* pAniView = dynamic_cast<CAniMakerView*>(pView);
		if (pAniView)
		{
			pAniView->load(szFileName);
		}
	}

	DragFinish(hDropInfo);
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
			case VK_ESCAPE:
			{
				OnClose();
				return TRUE;
			}
		}
	}

	return CMDIFrameWndEx::PreTranslateMessage(pMsg);
}

void CMainFrame::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	GetWindowPlacement(&wp);

	CWinApp* pApp = AfxGetApp();
	pApp->WriteProfileInt(_T("MainFrame"), _T("flags"), wp.flags);
	pApp->WriteProfileInt(_T("MainFrame"), _T("showCmd"), wp.showCmd);
	pApp->WriteProfileInt(_T("MainFrame"), _T("left"), wp.rcNormalPosition.left);
	pApp->WriteProfileInt(_T("MainFrame"), _T("top"), wp.rcNormalPosition.top);
	pApp->WriteProfileInt(_T("MainFrame"), _T("right"), wp.rcNormalPosition.right);
	pApp->WriteProfileInt(_T("MainFrame"), _T("bottom"), wp.rcNormalPosition.bottom);

	// ChildFrame 상태 저장 (메인 종료 시 OnClose가 호출되지 않으므로)
	CMDIChildWnd* pChild = MDIGetActive();
	if (pChild)
	{
		WINDOWPLACEMENT wpChild = { sizeof(WINDOWPLACEMENT) };
		pChild->GetWindowPlacement(&wpChild);

		pApp->WriteProfileInt(_T("ChildFrame"), _T("showCmd"), wpChild.showCmd);
		pApp->WriteProfileInt(_T("ChildFrame"), _T("left"), wpChild.rcNormalPosition.left);
		pApp->WriteProfileInt(_T("ChildFrame"), _T("top"), wpChild.rcNormalPosition.top);
		pApp->WriteProfileInt(_T("ChildFrame"), _T("right"), wpChild.rcNormalPosition.right);
		pApp->WriteProfileInt(_T("ChildFrame"), _T("bottom"), wpChild.rcNormalPosition.bottom);
	}

	CMDIFrameWndEx::OnClose();
}

void CMainFrame::set_image_info(int total_frames, int width, int height)
{
	CString text;

	if (total_frames > 0 && width > 0 && height > 0)
		text.Format(_T("%d frames, %d x %d"), total_frames, width, height);
	else
		text = _T("No image loaded");

	set_status_text(status_image_info, text);
}

void CMainFrame::set_progress_range(int min, int max)
{
	m_progress.SetRange32(min, max);
}

void CMainFrame::set_progress_pos(int pos)
{
	m_progress.SetPos(pos);
}

void CMainFrame::set_duration_info(std::deque<int>& frame_delays)
{
	int total_ms = 0;

	for (int delay : frame_delays)
		total_ms += delay;

	CString text;

	text.Format(_T("total %.3f ms"), (double)total_ms / 1000.0f);
	set_status_text(status_duration_info, text);
}

void CMainFrame::set_zoom_info(float zoom)
{
	CString text;
	text.Format(_T("%d%%"), (int)(zoom * 100));
	set_status_text(status_zoom_info, text);
}

void CMainFrame::set_status_text(int index, CString text)
{
	m_wndStatusBar.SetPaneText(index, text);
}

LRESULT CMainFrame::OnCheckChildFrames(WPARAM wParam, LPARAM lParam)
{
	CMDIChildWnd* pChild = MDIGetActive();

	if (pChild == NULL)
	{
		// 더 이상 child frame이 없음 → status bar 초기화
		set_image_info(0, 0, 0);
		set_status_text(status_duration_info, _T(""));
		set_status_text(status_zoom_info, _T(""));
		set_status_text(status_selected_info, _T(""));
	}

	return 0;
}

void CMainFrame::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu menu;
	CMenu* pSubMenu = nullptr;

	menu.LoadMenu(IDR_MENU_MAINFRAME_CONTEXT);
	pSubMenu = menu.GetSubMenu(0);

	pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIFrameWndEx::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if (m_progress.GetSafeHwnd() == NULL)
		return;

	CRect rc;
	m_wndStatusBar.GetItemRect(status_progress, &rc);
	rc.DeflateRect(0, 1, 3, 2); // 글자 baseline 맞추려면 y를 조금 더 줄임
	m_progress.MoveWindow(rc);
}

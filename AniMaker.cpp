
// AniMaker.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "AniMaker.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "AniMakerDoc.h"
#include "AniMakerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAniMakerApp

BEGIN_MESSAGE_MAP(CAniMakerApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CAniMakerApp::OnAppAbout)
	// 표준 인쇄 설정 명령입니다.
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
	ON_COMMAND(ID_FILE_NEW, &CAniMakerApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CAniMakerApp::OnFileOpen)
	ON_COMMAND(ID_FILE_CLOSE, &CAniMakerApp::OnFileClose)
	ON_COMMAND(ID_PASTE_AS_NEW_ANIMATION, &CAniMakerApp::OnPasteAsNewAnimation)
	ON_UPDATE_COMMAND_UI(ID_PASTE_AS_NEW_ANIMATION, &CAniMakerApp::OnUpdatePasteAsNewAnimation)
END_MESSAGE_MAP()


// CAniMakerApp 생성

CAniMakerApp::CAniMakerApp() noexcept
{

	// TODO: 아래 애플리케이션 ID 문자열을 고유 ID 문자열로 바꾸십시오(권장).
	// 문자열에 대한 서식: CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("AniMaker.AppID.NoVersion"));

	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}

// 유일한 CAniMakerApp 개체입니다.

CAniMakerApp theApp;


// CAniMakerApp 초기화

BOOL CAniMakerApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// OLE 라이브러리를 초기화합니다.
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();
	EnableTaskbarInteraction(FALSE);

	SetRegistryKey(_T("Legends Software"));
	LoadStdProfileSettings(16);  // MRU를 포함하여 표준 INI 파일 옵션을 로드합니다.

	// ★ CMFCToolBar 커스터마이징 지원 (선택)
	InitContextMenuManager();
	InitKeyboardManager();
	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(
		AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);	//COINIT_MULTITHREADED를 사용하면 프로그램 종료 시 런타임 에러 발생함.


	// 애플리케이션의 문서 템플릿을 등록합니다.  문서 템플릿은
	//  문서, 프레임 창 및 뷰 사이의 연결 역할을 합니다.
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_AniMakerTYPE,
		RUNTIME_CLASS(CAniMakerDoc),
		RUNTIME_CLASS(CChildFrame), // 사용자 지정 MDI 자식 프레임입니다.
		RUNTIME_CLASS(CAniMakerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// 주 MDI 프레임 창을 만듭니다.
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

	// --- MainFrame 위치 복원 ---
	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	wp.rcNormalPosition.left = GetProfileInt(_T("MainFrame"), _T("left"), CW_USEDEFAULT);
	wp.rcNormalPosition.top = GetProfileInt(_T("MainFrame"), _T("top"), CW_USEDEFAULT);
	wp.rcNormalPosition.right = GetProfileInt(_T("MainFrame"), _T("right"), CW_USEDEFAULT);
	wp.rcNormalPosition.bottom = GetProfileInt(_T("MainFrame"), _T("bottom"), CW_USEDEFAULT);
	wp.showCmd = GetProfileInt(_T("MainFrame"), _T("showCmd"), SW_SHOWDEFAULT);
	wp.flags = GetProfileInt(_T("MainFrame"), _T("flags"), 0);

	if (wp.rcNormalPosition.left != CW_USEDEFAULT)
	{
		pMainFrame->SetWindowPlacement(&wp);
	}
	else
	{
		pMainFrame->ShowWindow(m_nCmdShow);
	}
	pMainFrame->UpdateWindow();


	// 표준 셸 명령, DDE, 파일 열기에 대한 명령줄을 구문 분석합니다.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	//프로그램 시작 시 기본 빈 문서가 생성되는 것을 방지하기 위해.
	cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	// 명령줄에 지정된 명령을 디스패치합니다.
	// 응용 프로그램이 /RegServer, /Register, /Unregserver 또는 /Unregister로 시작된 경우 FALSE를 반환합니다.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// 주 창이 초기화되었으므로 이를 표시하고 업데이트합니다.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

int CAniMakerApp::ExitInstance()
{
	//TODO: 추가한 추가 리소스를 처리합니다.
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CAniMakerApp 메시지 처리기


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// 대화 상자를 실행하기 위한 응용 프로그램 명령입니다.
void CAniMakerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CAniMakerApp 메시지 처리기
void CAniMakerApp::OnFileNew()
{
	POSITION pos = GetFirstDocTemplatePosition();
	CDocTemplate* pTemplate = GetNextDocTemplate(pos);
	if (!pTemplate)
		return;

	CDocument* pDoc = pTemplate->OpenDocumentFile(nullptr); // 빈 문서 생성
	if (!pDoc)
		return;

	POSITION viewPos = pDoc->GetFirstViewPosition();
	CView* pView = pDoc->GetNextView(viewPos);
	CAniMakerView* pAniView = dynamic_cast<CAniMakerView*>(pView);
}

void CAniMakerApp::OnFileOpen()
{
	CString recent = GetProfileString(_T("setting"), _T("recent opened file"), _T(""));
	CFileDialog dlg(TRUE, nullptr, recent, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
		_T("Animation Files (*.gif;*.webp)|*.gif;*.webp|All Files (*.*)|*.*||"));

	if (dlg.DoModal() == IDCANCEL)
		return;

	CString path = dlg.GetPathName();
	WriteProfileString(_T("setting"), _T("recent opened file"), path);

	// MainFrm::OnDropFiles와 동일한 패턴: 새 Doc/Frame/View 생성 후 load 호출
	POSITION pos = GetFirstDocTemplatePosition();
	CDocTemplate* pTemplate = GetNextDocTemplate(pos);
	if (!pTemplate)
		return;

	CDocument* pDoc = pTemplate->OpenDocumentFile(nullptr); // 빈 문서 생성
	if (!pDoc)
		return;

	POSITION viewPos = pDoc->GetFirstViewPosition();
	CView* pView = pDoc->GetNextView(viewPos);
	CAniMakerView* pAniView = dynamic_cast<CAniMakerView*>(pView);
	if (pAniView)
	{
		pAniView->load(path);
	}
}

void CAniMakerApp::OnFileClose()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CAniMakerApp::OnPasteAsNewAnimation()
{
	POSITION pos = GetFirstDocTemplatePosition();
	CDocTemplate* pTemplate = GetNextDocTemplate(pos);
	if (!pTemplate)
		return;

	CDocument* pDoc = pTemplate->OpenDocumentFile(nullptr);
	if (!pDoc)
		return;

	POSITION viewPos = pDoc->GetFirstViewPosition();
	CView* pView = pDoc->GetNextView(viewPos);
	CAniMakerView* pAniView = dynamic_cast<CAniMakerView*>(pView);
	if (pAniView)
	{
		if (!pAniView->load_from_clipboard())
		{
			pDoc->OnCloseDocument();
		}
	}
}

void CAniMakerApp::OnUpdatePasteAsNewAnimation(CCmdUI* pCmdUI)
{
	static UINT cfPng = RegisterClipboardFormat(_T("PNG"));
	BOOL hasImage = IsClipboardFormatAvailable(CF_DIBV5) ||
		IsClipboardFormatAvailable(CF_DIB) ||
		IsClipboardFormatAvailable(cfPng);
	pCmdUI->Enable(hasImage);
}


// AniMakerView.cpp: CAniMakerView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "AniMaker.h"
#endif

#include "MainFrm.h"
#include "AniMakerDoc.h"
#include "AniMakerView.h"
#include "FrameProperty.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAniMakerView

IMPLEMENT_DYNCREATE(CAniMakerView, CView)

BEGIN_MESSAGE_MAP(CAniMakerView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_DROPFILES()
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEHWHEEL()
	ON_WM_RBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	ON_COMMAND(ID_MENU_PREVIEW, &CAniMakerView::OnMenuPreview)
	ON_COMMAND(ID_MENU_CUT, &CAniMakerView::OnMenuCut)
	ON_COMMAND(ID_MENU_COPY, &CAniMakerView::OnMenuCopy)
	ON_COMMAND(ID_MENU_PASTE_INTO_SELECTED_FRAME, &CAniMakerView::OnMenuPasteIntoSelectedFrame)
	ON_COMMAND(ID_MENU_PASTE_BEFORE_CURRENT_FRAME, &CAniMakerView::OnMenuPasteBeforeCurrentFrame)
	ON_COMMAND(ID_MENU_PASTE_AFTER_CURRENT_FRAME, &CAniMakerView::OnMenuPasteAfterCurrentFrame)
	ON_COMMAND(ID_MENU_DELETE, &CAniMakerView::OnMenuDelete)
	ON_COMMAND(ID_MENU_DUPLICATE_SELECTED, &CAniMakerView::OnMenuDuplicateSelected)
	ON_COMMAND(ID_MENU_INSERT_FRAME_FROM_FILE, &CAniMakerView::OnMenuInsertFrameFromFile)
	ON_COMMAND(ID_MENU_INSERT_FRAME_EMPTY, &CAniMakerView::OnMenuInsertFrameEmpty)
	ON_COMMAND(ID_MENU_FRAME_PROPERTY, &CAniMakerView::OnMenuFrameProperty)
	ON_COMMAND(ID_MENU_SAVE_FRAME_AS, &CAniMakerView::OnMenuSaveFrameAs)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_MENU_VIEW_ANIMATION, &CAniMakerView::OnMenuViewAnimation)
	ON_COMMAND(ID_MENU_SAVE_AS, &CAniMakerView::OnMenuSaveAs)
	ON_COMMAND(ID_FILE_SAVE, &CAniMakerView::OnFileSave)
	ON_COMMAND(ID_EDIT_CUT, &CAniMakerView::OnMenuCut)
	ON_COMMAND(ID_EDIT_COPY, &CAniMakerView::OnMenuCopy)
	ON_COMMAND(ID_EDIT_PASTE, &CAniMakerView::OnMenuPasteAfterCurrentFrame)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, &CAniMakerView::OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CAniMakerView::OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &CAniMakerView::OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, &CAniMakerView::OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CAniMakerView::OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CAniMakerView::OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &CAniMakerView::OnUpdateEditRedo)
	ON_COMMAND(ID_MENU_ZOOM_50, &CAniMakerView::OnMenuZoom50)
	ON_COMMAND(ID_MENU_ZOOM_100, &CAniMakerView::OnMenuZoom100)
END_MESSAGE_MAP()

// CAniMakerView 생성/소멸

CAniMakerView::CAniMakerView() noexcept
{
	// TODO: 여기에 생성 코드를 추가합니다.

}

CAniMakerView::~CAniMakerView()
{
}

BOOL CAniMakerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.
	cs.style |= WS_HSCROLL | WS_VSCROLL;
	return CView::PreCreateWindow(cs);
}

// CAniMakerView 그리기

void CAniMakerView::OnDraw(CDC* /*pDC*/)
{
	//CRect rc;
	//GetClientRect(rc);

	ID2D1DeviceContext* d2dc = m_d2dc.get_d2dc();
	if (!d2dc)
		return;

	D2D1_SIZE_F sz_dc = m_d2dc.get_size();

	d2dc->BeginDraw();
	d2dc->Clear(get_sys_d2color(COLOR_3DFACE));

	// Transform: world → screen
	// 1) scroll (world 좌표 이동)  2) zoom (확대/축소)
	D2D1::Matrix3x2F mat =
		D2D1::Matrix3x2F::Translation(-m_pt_scroll.x, -m_pt_scroll.y) *
		D2D1::Matrix3x2F::Scale(m_zoom, m_zoom);
	d2dc->SetTransform(mat);

	int nFrames = m_img.get_frame_count();
	if (nFrames > 0 && m_img.is_valid())
	{
		float imgW = m_img.get_width();
		float imgH = m_img.get_height();
		float thumbH = m_sz_thumb;
		float thumbW = (imgH > 0.f) ? (imgW / imgH * thumbH) : thumbH;

		float x = m_thumb_margin;
		float y = m_thumb_margin;
		CString str;

		for (int i = 0; i < nFrames; i++)
		{
			ID2D1Bitmap1* pFrame = m_img.get_frame_img(i);
			if (pFrame)
			{
				D2D1_RECT_F rthumb = D2D1::RectF(x, y, x + thumbW, y + thumbH);

				//alpha가 포함된 이미지라면 격자 패턴을 먼저 그려주고
				if (m_img.get_alpha_pixel_count() > 0)
				{
					auto zigzag = m_d2dc.get_zigzag_brush();
					float zigzag_size = m_d2dc.get_zigzag_size();
					zigzag->SetTransform(D2D1::Matrix3x2F::Scale(zigzag_size / m_zoom, zigzag_size / m_zoom) * D2D1::Matrix3x2F::Translation(rthumb.left, rthumb.top));
					d2dc->FillRectangle(rthumb, zigzag.Get());
				}

				//이미지 표시
				d2dc->DrawBitmap(pFrame, rthumb);

				//이미지 영역에 옅은 회색 테두리 표시
				draw_rect(d2dc, rthumb, Gdiplus::Color::LightGray, Gdiplus::Color::Transparent, 1.0f);

				//frame index와 delay 표시
				str.Format(_T("F:%d D:%d"), i, m_img.get_frame_delay(i));
				D2D1_RECT_F rtext = rthumb;
				rtext.top = rthumb.bottom + 8;
				rtext.bottom = rtext.top + 24;
				draw_text(d2dc, rtext, str, _T("Arial"), 12.0f / m_zoom, FW_NORMAL, Gdiplus::Color::Black, Gdiplus::Color::LightGray, DT_CENTER | DT_TOP);

				//선택항목이면 royalblue border로 표시한다.
				if (std::find(m_selected.begin(), m_selected.end(), i) != m_selected.end())
				{
					//inflate_rect(rthumb, 0, 0);
					draw_rect(d2dc, rthumb, Gdiplus::Color::RoyalBlue, Gdiplus::Color::Transparent, 4.0f / m_zoom);
				}
			}
			x += thumbW + (m_thumb_gap / m_zoom);
		}
	}

	d2dc->SetTransform(D2D1::Matrix3x2F::Identity());

	//드래그 선택 중이면 rubber band 표시
	if (m_bDragSelecting)
	{
		D2D1_RECT_F dragRect = D2D1::RectF(
			(float)min(m_ptDragStart.x, m_ptDragCurrent.x),
			(float)min(m_ptDragStart.y, m_ptDragCurrent.y),
			(float)max(m_ptDragStart.x, m_ptDragCurrent.x),
			(float)max(m_ptDragStart.y, m_ptDragCurrent.y));

		//반투명 채우기 + 테두리 (Windows 탐색기 스타일)
		draw_rect(d2dc, dragRect,
			Gdiplus::Color(180, 65, 105, 225),	//테두리: RoyalBlue
			Gdiplus::Color(40, 65, 105, 225),	//채우기: 반투명 RoyalBlue
			1.0f);
	}

	HRESULT hr = d2dc->EndDraw();

	if (SUCCEEDED(hr))
		hr = m_d2dc.get_swapchain()->Present(0, 0);
}


// CAniMakerView 인쇄

BOOL CAniMakerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CAniMakerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CAniMakerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}


// CAniMakerView 진단

#ifdef _DEBUG
void CAniMakerView::AssertValid() const
{
	CView::AssertValid();
}

void CAniMakerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CAniMakerDoc* CAniMakerView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAniMakerDoc)));
	return (CAniMakerDoc*)m_pDocument;
}
#endif //_DEBUG


// CAniMakerView 메시지 처리기

void CAniMakerView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (!pDoc)
		pDoc = GetDocument();

	recalc_scrollbars();
	Invalidate();

	//preview 창은 각 view마다 생성된다. mainframe의 공통 child가 아닌 각 view마다 생성된다.
	// Preview에 D2D 디바이스 공유 설정 (create 전에 호출)
	m_preview.set_shared_d2dc(&m_d2dc);
	m_preview.Create(IDD_PREVIEW, this);

	m_message.set_text(this, _T(""), 32, Gdiplus::FontStyleBold, 4.0f, 2.4f);
	m_message.set_stroke_color(Gdiplus::Color::Black);
	m_message.set_alpha(192);
	m_message.use_control(false);

	DragAcceptFiles();

	if (pDoc && !pDoc->m_file.IsEmpty())
		load(pDoc->m_file);
}

void CAniMakerView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	if (!pDoc)
		pDoc = GetDocument();

	CRect rc;
	GetClientRect(&rc);

	// 실제 뷰 크기로 초기화 (아직 크기가 0이면 기본값 사용)
	int cx = max(rc.Width(), 1);
	int cy = max(rc.Height(), 1);
	m_d2dc.init(m_hWnd, cx, cy);

	m_zoom = get_profile_value(_T("setting"), _T("zoom"), 1.f);
	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_zoom_info(m_zoom);
}

BOOL CAniMakerView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	//TRACE(_T("message = %u, wParam = %u, lParam = %ld\n"), pMsg->message, pMsg->wParam, pMsg->lParam);
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ADD || pMsg->wParam == VK_OEM_PLUS)
		{
			// '+' 키: 줌 인
			m_zoom += m_zoom_step;
			m_zoom = min(m_zoom, m_zoom_max);
			((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_zoom_info(m_zoom);
			recalc_scrollbars();
			Invalidate(FALSE);
			return TRUE;
		}
		else if (pMsg->wParam == VK_SUBTRACT || pMsg->wParam == VK_OEM_MINUS)
		{
			// '-' 키: 줌 아웃
			m_zoom -= m_zoom_step;
			m_zoom = max(m_zoom, m_zoom_min);
			((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_zoom_info(m_zoom);
			recalc_scrollbars();
			Invalidate(FALSE);
			return TRUE;
		}
		else if (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT)
		{
			int nFrames = m_img.get_frame_count();
			if (nFrames <= 0)
				return TRUE;

			//멀티선택 상태에서 좌우 방향키를 누른다면 맨 처음 혹은 맨 끝 선택항목만 남긴다.
			if (m_selected.size() > 1)
				m_selected.resize(1);

			if (pMsg->wParam == VK_LEFT)
				m_selected[0] = max(0, m_selected[0] - 1);
			else
				m_selected[0] = min(nFrames - 1, m_selected[0] + 1);

			// 선택 프레임이 보이도록 자동 스크롤
			ensure_frame_visible(m_selected[0]);
			Invalidate(FALSE);
			return TRUE;
		}
		else if (pMsg->wParam == VK_HOME)
		{
			//멀티선택 상태에서 좌우 방향키를 누른다면 맨 처음 혹은 맨 끝 선택항목만 남긴다.
			if (m_selected.size() > 1)
				m_selected.resize(1);

			m_selected[0] = 0;

			ensure_frame_visible(m_selected[0]);
			Invalidate(FALSE);
			return TRUE;
		}
		else if (pMsg->wParam == VK_END)
		{
			//멀티선택 상태에서 좌우 방향키를 누른다면 맨 처음 혹은 맨 끝 선택항목만 남긴다.
			if (m_selected.size() > 1)
				m_selected.resize(1);

			int nFrames = m_img.get_frame_count();
			if (nFrames > 0)
				m_selected[0] = nFrames - 1;

			ensure_frame_visible(m_selected[0]);
			Invalidate(FALSE);
			return TRUE;
		}
		else if (pMsg->wParam == 'C')
		{
			if (IsCtrlPressed())
			{
				OnMenuCopy();
				return TRUE;
			}
		}
		else if (pMsg->wParam == 'Y')
		{
			if (IsCtrlPressed())
			{
				OnEditRedo();
				return TRUE;
			}
		}
	}
	else if (pMsg->message == WM_MOUSEHWHEEL)
	{
		TRACE(_T("message = %d, wParam = %d, lParam = %d\n"), pMsg->message, pMsg->wParam, pMsg->lParam);
		OnMouseHWheel((UINT)LOWORD(pMsg->wParam), (short)HIWORD(pMsg->wParam), CPoint(GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam)));
		return TRUE;
	}
	return CView::PreTranslateMessage(pMsg);
}

void CAniMakerView::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
}

void CAniMakerView::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	TCHAR szFileName[MAX_PATH];
	DragQueryFile(hDropInfo, 0, szFileName, MAX_PATH);

	load(szFileName);

	DragFinish(hDropInfo);
}

BOOL CAniMakerView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	return FALSE;

	return CView::OnEraseBkgnd(pDC);
}

void CAniMakerView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	float frame_step = get_frame_step();
	if (frame_step <= 0.f)
		return;

	SCROLLINFO si = { sizeof(SCROLLINFO), SIF_ALL };
	GetScrollInfo(SB_HORZ, &si);

	// 현재 프레임 인덱스 기반으로 이동
	int curFrame = (int)roundf(m_pt_scroll.x / frame_step);

	// 한 페이지에 보이는 프레임 수
	float visibleW = (si.nPage > 0) ? (float)si.nPage : 1.f;
	int pageFrames = max(1, (int)(visibleW / frame_step));

	switch (nSBCode)
	{
		case SB_LINELEFT:    curFrame -= 1;           break;
		case SB_LINERIGHT:   curFrame += 1;           break;
		case SB_PAGELEFT:    curFrame -= pageFrames;  break;
		case SB_PAGERIGHT:   curFrame += pageFrames;  break;
		case SB_THUMBTRACK:  curFrame = (int)roundf((float)si.nTrackPos / frame_step); break;
		default: return;
	}

	curFrame = max(0, curFrame);
	m_pt_scroll.x = curFrame * frame_step;

	recalc_scrollbars();
	Invalidate();

	//CView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CAniMakerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	int index = get_frame_index(point);

	if (index >= 0)
	{
		if (IsCtrlPressed() && IsShiftPressed())
		{
			//Ctrl+Shift+클릭: 앵커~클릭 범위를 기존 선택에 추가
			if (m_anchor_index >= 0)
			{
				int start = min(m_anchor_index, index);
				int end = max(m_anchor_index, index);
				for (int i = start; i <= end; i++)
				{
					if (std::find(m_selected.begin(), m_selected.end(), i) == m_selected.end())
						m_selected.push_back(i);
				}
			}
			//앵커는 유지
		}
		else if (IsShiftPressed())
		{
			//Shift+클릭: 앵커~클릭 범위 선택 (기존 선택 대체)
			if (m_anchor_index >= 0)
			{
				int start = min(m_anchor_index, index);
				int end = max(m_anchor_index, index);
				m_selected.clear();
				for (int i = start; i <= end; i++)
					m_selected.push_back(i);
			}
			else
			{
				m_selected.clear();
				m_selected.push_back(index);
				m_anchor_index = index;
			}
			//앵커는 유지
		}
		else if (IsCtrlPressed())
		{
			//Ctrl+클릭: 개별 토글
			auto it = std::find(m_selected.begin(), m_selected.end(), index);
			if (it != m_selected.end())
				m_selected.erase(it);
			else
				m_selected.push_back(index);
			m_anchor_index = index;
		}
		else
		{
			//일반 클릭: 단일 선택
			m_selected.clear();
			m_selected.push_back(index);
			m_anchor_index = index;
		}
	}
	else
	{
		//빈 영역 클릭 → 드래그 선택 시작
		if (!IsCtrlPressed())
			m_selected.clear();

		m_selected_before_drag = m_selected;
		m_bDragSelecting = true;
		m_ptDragStart = point;
		m_ptDragCurrent = point;
		SetCapture();
	}

	Invalidate();

	CView::OnLButtonDown(nFlags, point);
}

void CAniMakerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (m_bDragSelecting)
	{
		m_bDragSelecting = false;
		ReleaseCapture();
		m_selected_before_drag.clear();
		Invalidate();
	}

	CView::OnLButtonUp(nFlags, point);
}

void CAniMakerView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (m_bDragSelecting)
	{
		m_ptDragCurrent = point;

		//screen → world 좌표로 변환한 드래그 영역
		float x1 = m_ptDragStart.x / m_zoom + m_pt_scroll.x;
		float y1 = m_ptDragStart.y / m_zoom + m_pt_scroll.y;
		float x2 = m_ptDragCurrent.x / m_zoom + m_pt_scroll.x;
		float y2 = m_ptDragCurrent.y / m_zoom + m_pt_scroll.y;

		D2D1_RECT_F dragRect = D2D1::RectF(
			min(x1, x2), min(y1, y2),
			max(x1, x2), max(y1, y2));

		//Ctrl+드래그이면 이전 선택 상태에서 시작
		if (IsCtrlPressed())
			m_selected = m_selected_before_drag;
		else
			m_selected.clear();

		get_frames_in_rect(dragRect, m_selected);

		Invalidate();
	}

	CView::OnMouseMove(nFlags, point);
}

BOOL CAniMakerView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	ScreenToClient(&pt);

	//Wheel에 따라 커서 중심 줌
	float wx = pt.x / m_zoom + m_pt_scroll.x;
	float wy = pt.y / m_zoom + m_pt_scroll.y;

	if (zDelta > 0)
		m_zoom += m_zoom_step;
	else
		m_zoom -= m_zoom_step;
	m_zoom = max(m_zoom_min, min(m_zoom, m_zoom_max));

	// 줌 후 동일 월드 좌표가 커서 아래에 유지되도록 보정
	m_pt_scroll.x = wx - pt.x / m_zoom;
	m_pt_scroll.y = wy - pt.y / m_zoom;

	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_zoom_info(m_zoom);

	recalc_scrollbars();
	Invalidate();

	return TRUE;
	//return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CAniMakerView::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	TRACE(_T("OnMouseHWheel\n"));
	float frame_step = get_frame_step();
	if (frame_step <= 0.f)
		return;

	// 틸트 한 틱 = 1프레임 이동
	if (zDelta > 0)
		m_pt_scroll.x += frame_step;
	else if (zDelta < 0)
		m_pt_scroll.x -= frame_step;

	recalc_scrollbars();
	Invalidate();
	//CView::OnMouseHWheel(nFlags, zDelta, pt);
}

void CAniMakerView::OnRButtonDown(UINT nFlags, CPoint point)
{
	int index = get_frame_index(point);

	//선택된 항목에서 우클릭은 바로 팝업 메뉴를 표시하지만
	//선택되지 않은 항목을 우클릭하면 선택 정보를 모두 초기화하고 우클릭한 프레임을 선택한 후 팝업 메뉴를 표시한다.
	if (index >= 0 && std::find(m_selected.begin(), m_selected.end(), index) == m_selected.end())
	{
		m_selected.clear();
		m_selected.push_back(index);
		Invalidate();
	}

	CView::OnRButtonDown(nFlags, point);
}

void CAniMakerView::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CMenu menu;
	CMenu* pMenu;

	int index = get_frame_index(point);

	//프레임이 아닌 영역에서 우클릭 시 표시하는 메뉴가 다르다.
	if (index < 0)
		menu.LoadMenu(IDR_MENU_CONTEXT);
	else
		menu.LoadMenu(IDR_MENU_FRAME_CONTEXT);

	pMenu = menu.GetSubMenu(0);

	ClientToScreen(&point);





	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

	CView::OnRButtonUp(nFlags, point);
}

BOOL CAniMakerView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	return CView::OnSetCursor(pWnd, nHitTest, message);
}

void CAniMakerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if (!m_d2dc.get_d2dc())
		return;

	m_d2dc.on_size_changed(cx, cy);
	recalc_scrollbars();
}

void CAniMakerView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CView::OnTimer(nIDEvent);
}

void CAniMakerView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	SCROLLINFO si = { sizeof(SCROLLINFO), SIF_ALL };
	GetScrollInfo(SB_VERT, &si);

	int pos = si.nPos;
	switch (nSBCode)
	{
		case SB_LINEUP:      pos -= 20;           break;
		case SB_LINEDOWN:    pos += 20;           break;
		case SB_PAGEUP:      pos -= si.nPage;     break;
		case SB_PAGEDOWN:    pos += si.nPage;     break;
		case SB_THUMBTRACK:  pos = si.nTrackPos;  break;
		default: return;
	}

	m_pt_scroll.y = (float)max(si.nMin, min(pos, si.nMax - (int)si.nPage));
	recalc_scrollbars();
	Invalidate(FALSE);
	//CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CAniMakerView::recalc_scrollbars()
{
	CRect rc;
	GetClientRect(&rc);

	int nFrames = m_img.get_frame_count();
	float contentW = 0.f, contentH = 0.f;
	float frame_step = get_frame_step();

	if (nFrames > 0 && m_img.is_valid())
	{
		float thumbW = frame_step - (m_thumb_gap / m_zoom);
		contentW = m_thumb_margin * 2 + nFrames * thumbW + (nFrames - 1) * (m_thumb_gap / m_zoom);
		contentH = m_thumb_margin * 2 + m_sz_thumb;
	}

	// 뷰포트의 월드 좌표 크기
	float visibleW = rc.Width() / m_zoom;
	float visibleH = rc.Height() / m_zoom;

	// 가로: 프레임 단위 스냅
	if (frame_step > 0.f)
		m_pt_scroll.x = roundf(m_pt_scroll.x / frame_step) * frame_step;

	// 클램프
	m_pt_scroll.x = max(0.f, min(m_pt_scroll.x, max(0.f, contentW - visibleW)));
	m_pt_scroll.y = max(0.f, min(m_pt_scroll.y, max(0.f, contentH - visibleH)));

	SCROLLINFO si = { sizeof(SCROLLINFO), SIF_ALL };

	// 가로 스크롤
	si.nMin = 0;
	si.nMax = (int)contentW;
	si.nPage = (UINT)visibleW;
	si.nPos = (int)m_pt_scroll.x;
	SetScrollInfo(SB_HORZ, &si, TRUE);

	// 세로 스크롤
	si.nMax = (int)contentH;
	si.nPage = (UINT)visibleH;
	si.nPos = (int)m_pt_scroll.y;
	SetScrollInfo(SB_VERT, &si, TRUE);

	//m_zoom이 변경되면 항상 recalc_scrollbars()가 호출되므로 여기에서 저장
	write_profile_value(_T("setting"), _T("zoom"), m_zoom);
}

float CAniMakerView::get_frame_step()
{
	float imgH = m_img.get_height();
	float thumbW = (imgH > 0.f) ? (m_img.get_width() / imgH * m_sz_thumb) : m_sz_thumb;
	return thumbW + (m_thumb_gap / m_zoom);
}

int	CAniMakerView::get_frame_index(CPoint pt)
{
	int nFrames = m_img.get_frame_count();
	if (nFrames <= 0 || !m_img.is_valid())
		return -1;

	// screen → world 좌표 변환
	float wx = pt.x / m_zoom + m_pt_scroll.x;
	float wy = pt.y / m_zoom + m_pt_scroll.y;

	float imgH = m_img.get_height();
	float thumbH = m_sz_thumb;
	float thumbW = (imgH > 0.f) ? (m_img.get_width() / imgH * thumbH) : thumbH;
	float frame_step = thumbW + (m_thumb_gap / m_zoom);

	// Y 범위 체크
	if (wy < m_thumb_margin || wy > m_thumb_margin + thumbH)
		return -1;

	// X로 인덱스 계산
	float relX = wx - m_thumb_margin;
	if (relX < 0.f)
		return -1;

	int index = (int)(relX / frame_step);
	//trace(index);

	// gap 영역 클릭 제외 (썸네일 이미지 위만 유효)
	float withinFrame = relX - index * frame_step;
	if (withinFrame > thumbW)
		return -1;

	if (index < 0 || index >= nFrames)
		return -1;

	return index;
}

void CAniMakerView::get_frames_in_rect(D2D1_RECT_F rectWorld, std::deque<int>& result)
{
	int nFrames = m_img.get_frame_count();
	if (nFrames <= 0 || !m_img.is_valid())
		return;

	float imgH = m_img.get_height();
	float thumbH = m_sz_thumb;
	float thumbW = (imgH > 0.f) ? (m_img.get_width() / imgH * thumbH) : thumbH;
	float frame_step = thumbW + (m_thumb_gap / m_zoom);

	for (int i = 0; i < nFrames; i++)
	{
		float x = m_thumb_margin + i * frame_step;
		float y = m_thumb_margin;

		//프레임 사각형과 드래그 사각형의 교차 판정
		if (x + thumbW > rectWorld.left && x < rectWorld.right &&
			y + thumbH > rectWorld.top && y < rectWorld.bottom)
		{
			if (std::find(result.begin(), result.end(), i) == result.end())
				result.push_back(i);
		}
	}
}

void CAniMakerView::ensure_frame_visible(int index)
{
	int nFrames = m_img.get_frame_count();
	if (index < 0 || index >= nFrames)
		return;

	float frame_step = get_frame_step();
	float thumbW = frame_step - (m_thumb_gap / m_zoom);

	// 해당 프레임의 월드 좌표 범위
	float frameLeft = m_thumb_margin + index * frame_step;
	float frameRight = frameLeft + thumbW;

	CRect rc;
	GetClientRect(&rc);
	float visibleW = rc.Width() / m_zoom;

	// 왼쪽으로 벗어난 경우
	if (frameLeft < m_pt_scroll.x)
		m_pt_scroll.x = frameLeft - m_thumb_margin;

	// 오른쪽으로 벗어난 경우
	if (frameRight > m_pt_scroll.x + visibleW)
		m_pt_scroll.x = frameRight - visibleW + m_thumb_margin;

	recalc_scrollbars();
}

void CAniMakerView::OnMenuPreview()
{
	//현재 view에서 생성된 facotory와 m_preview에서 사용하는 m_imgDlg를 생성할 때 생성된 factory가 다르므로
	//이미지 객체만 전달한다고 해서 그려지지 않는다.
	//m_preview에서 생성한 m_imgDlg 생성 시 만들어진 factory로 설정하여 테스트 해 볼 수는 있으나 정석이 아니며
	//동일한 D2D 팩토리/디바이스 공유 방식으로 구현해야 한다.
	//복잡하므로 우선 m_img를 외부파일로 저장하고 이를 m_preview에서 읽어서 재생하는 방식으로 구현한다.
	//m_img.save_webp(_T("D:\\share.webp"));
	//=>202602 동일한 D2D 팩토리/디바이스 공유 방식으로 구현 완료.
	m_preview.set_image(&m_img);

	if (m_preview.IsIconic())
		m_preview.ShowWindow(SW_RESTORE);

	m_preview.ShowWindow(SW_SHOW);
}

void CAniMakerView::show_message(CString message)
{
	//::PlaySound(MAKEINTRESOURCE(IDR_WAVE_DICK), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);

	//메시지 폰트 크기는 rc height에 비례하는 크기로 자동 조정되어야 한다.
	CRect rc;
	GetClientRect(rc);

	CSCShapeDlgTextSetting* setting = m_message.get_text_setting();
	setting->text = message;
	setting->text_prop.size = rc.Height() / 26.18f;
	Clamp(setting->text_prop.size, 16.0f, 44.0f);

	m_message.set_text(setting);
	m_message.CenterWindow();
	m_message.fade_in(0, 1000, true);

	SetFocus();
}

void CAniMakerView::OnMenuCut()
{
	if (m_selected.size() == 0)
		return;

	OnMenuCopy();
	OnMenuDelete();
}

void CAniMakerView::OnUpdateEditCut(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_selected.size() > 0 && m_img.is_valid());
}

void CAniMakerView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_selected.size() > 0 && m_img.is_valid());
}

void CAniMakerView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_copied_frames.empty() && !m_selected.empty());
}

void CAniMakerView::OnMenuCopy()
{
	if (m_selected.size() == 0)
		return;

	m_copied_frames.clear();
	m_copied_delays.clear();

	UINT w = (UINT)m_img.get_width();
	UINT h = (UINT)m_img.get_height();

	for (auto idx : m_selected)
	{
		CSCD2Image frame_copy;
		frame_copy.create(m_img.get_WICFactory2(), m_img.get_d2dc(), w, h);

		D2D1_POINT_2U pt = { 0, 0 };
		D2D1_RECT_U r = { 0, 0, w, h };
		frame_copy.get_frame_img(0)->CopyFromBitmap(&pt, m_img.get_frame_img(idx), &r);

		m_copied_frames.push_back(std::move(frame_copy.get_frame_img(0)));
		m_copied_delays.push_back(m_img.get_frame_delay(idx));
	}

	//클립보드에도 복사 (외부 붙여넣기 용도)
	m_img.copy_to_clipboard(m_selected[0]);
}

void CAniMakerView::OnMenuPasteIntoSelectedFrame()
{
	if (m_copied_frames.empty() || m_selected.empty())
		return;

	UINT w = (UINT)m_img.get_width();
	UINT h = (UINT)m_img.get_height();
	D2D1_POINT_2U pt = { 0, 0 };
	D2D1_RECT_U r = { 0, 0, w, h };

	// Undo: 수정 전 원본 저장
	UndoAction action;
	action.type = eUndoType::ModifyFrames;

	//복사된 프레임과 선택된 프레임을 1:1 대응하여 덮어쓴다
	int count = min((int)m_copied_frames.size(), (int)m_selected.size());
	for (int i = 0; i < count; i++)
	{
		int idx = m_selected[i];
		action.indices.push_back(idx);
		action.saved_frames.push_back(clone_bitmap(m_img.get_frame_img(idx)));
		action.saved_delays.push_back(m_img.get_frame_delay(idx));

		ID2D1Bitmap1* pDst = m_img.get_frame_img(idx);
		if (pDst && m_copied_frames[i])
		{
			pDst->CopyFromBitmap(&pt, m_copied_frames[i].Get(), &r);
			m_img.set_frame_delay(idx, m_copied_delays[i]);
		}
	}

	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_duration_info(*m_img.get_frame_delay_list());

	Invalidate();

	apply_to_preview();

	push_undo(std::move(action));
	update_ui_after_edit();
}

void CAniMakerView::OnMenuPasteBeforeCurrentFrame()
{
	if (m_copied_frames.empty() || m_selected.empty())
		return;

	int insertPos = m_selected[0];
	UINT w = (UINT)m_img.get_width();
	UINT h = (UINT)m_img.get_height();

	for (size_t i = 0; i < m_copied_frames.size(); i++)
	{
		CSCD2Image frame_img;
		frame_img.create(m_img.get_WICFactory2(), m_img.get_d2dc(), w, h);

		D2D1_POINT_2U pt = { 0, 0 };
		D2D1_RECT_U r = { 0, 0, w, h };
		frame_img.get_frame_img(0)->CopyFromBitmap(&pt, m_copied_frames[i].Get(), &r);

		m_img.get_img_list()->insert(m_img.get_img_list()->begin() + insertPos,
			std::move(frame_img.get_frame_img(0)));
		m_img.get_frame_delay_list()->insert(m_img.get_frame_delay_list()->begin() + insertPos,
			m_copied_delays[i]);
		insertPos++;
	}

	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_image_info(m_img.get_frame_count(), m_img.get_width(), m_img.get_height());
	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_duration_info(*m_img.get_frame_delay_list());

	recalc_scrollbars();

	apply_to_preview();

	Invalidate();
}

void CAniMakerView::OnMenuPasteAfterCurrentFrame()
{
	if (m_copied_frames.empty() || m_selected.empty())
		return;

	int insertPos = m_selected[0] + 1;
	UINT w = (UINT)m_img.get_width();
	UINT h = (UINT)m_img.get_height();

	// Undo 데이터 준비
	UndoAction action;
	action.type = eUndoType::InsertFrames;

	for (size_t i = 0; i < m_copied_frames.size(); i++)
	{
		CSCD2Image frame_img;
		frame_img.create(m_img.get_WICFactory2(), m_img.get_d2dc(), w, h);

		D2D1_POINT_2U pt = { 0, 0 };
		D2D1_RECT_U r = { 0, 0, w, h };
		frame_img.get_frame_img(0)->CopyFromBitmap(&pt, m_copied_frames[i].Get(), &r);

		m_img.get_img_list()->insert(m_img.get_img_list()->begin() + insertPos,
			std::move(frame_img.get_frame_img(0)));
		m_img.get_frame_delay_list()->insert(m_img.get_frame_delay_list()->begin() + insertPos,
			m_copied_delays[i]);

		action.indices.push_back(insertPos);
		insertPos++;
	}

	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_image_info(m_img.get_frame_count(), m_img.get_width(), m_img.get_height());
	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_duration_info(*m_img.get_frame_delay_list());

	recalc_scrollbars();

	apply_to_preview();

	Invalidate();

	push_undo(std::move(action));
	update_ui_after_edit();
}

void CAniMakerView::OnMenuDelete()
{
	if (m_selected.size() == 0)
		return;

	// Undo 데이터 저장 (삭제 전에)
	UndoAction action;
	action.type = eUndoType::DeleteFrames;
	action.indices = m_selected;
	for (int idx : m_selected)
	{
		action.saved_frames.push_back(clone_bitmap(m_img.get_frame_img(idx)));
		action.saved_delays.push_back(m_img.get_frame_delay(idx));
	}
	push_undo(std::move(action));

	// 선택된 프레임들을 뒤에서부터 삭제 (인덱스가 밀리는 것을 방지하기 위해)
	for (int i = m_selected.size() - 1; i >= 0; i--)
	{
		m_img.get_img_list()->erase(m_img.get_img_list()->begin() + m_selected[i]);
		m_img.get_frame_delay_list()->erase(m_img.get_frame_delay_list()->begin() + m_selected[i]);
	}

	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_image_info(m_img.get_frame_count(), m_img.get_width(), m_img.get_height());
	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_duration_info(*m_img.get_frame_delay_list());

	//선택 항목들이 삭제되면 선택 정보 또한 clear 시켜야 한다.
	m_selected.clear();

	update_ui_after_edit();

	recalc_scrollbars();

	Invalidate();

	apply_to_preview();
}

void CAniMakerView::OnMenuDuplicateSelected()
{
	if (m_selected.size() != 1)
	{
		show_message(_T("프레임 복제 기능은 하나의 프레임을 선택했을때만 가능합니다."));
		return;
	}

	CSCD2Image frame_img;

	frame_img.create(m_img.get_WICFactory2(), m_img.get_d2dc(), m_img.get_width(), m_img.get_height());

	D2D1_POINT_2U pt = { 0, 0 };
	D2D1_RECT_U r = { 0, 0, m_img.get_width(), m_img.get_height() };
	frame_img.get_frame_img(0)->CopyFromBitmap(&pt, m_img.get_frame_img(m_selected[0]), &r);
	m_img.get_img_list()->insert(m_img.get_img_list()->begin() + m_selected[0], std::move(frame_img.get_frame_img(0)));

	//gif, webp와 같은 animated image가 아닌 경우는 m_frame_delay가 처음부터 존재하지 않으므로
	//기본값으로 넣어줘야 한다.
	int src_frame_delay = m_img.get_frame_delay(m_selected[0]);
	if (src_frame_delay <= 0)
	{
		src_frame_delay = 30;
		m_img.set_frame_delay(m_selected[0], src_frame_delay);
	}

	m_img.get_frame_delay_list()->insert(m_img.get_frame_delay_list()->begin() + m_selected[0], src_frame_delay);

	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_image_info(m_img.get_frame_count(), m_img.get_width(), m_img.get_height());
	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_duration_info(*m_img.get_frame_delay_list());

	recalc_scrollbars();

	apply_to_preview();

	Invalidate();
}

void CAniMakerView::OnMenuInsertFrameFromFile()
{
	if (m_selected.size() != 1)
	{
		show_message(_T("파일에서 프레임 삽입 기능은 하나의 프레임을 선택했을때만 가능합니다."));
		return;
	}

	CString recent = theApp.GetProfileString(_T("setting"), _T("recent insert frame path"), _T(""));
	CFileDialog dlg(TRUE, NULL, recent,
		OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST,
		_T("Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.webp;*.tif)|*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.webp;*.tif|All Files (*.*)|*.*||"));

	// 멀티선택을 위한 충분한 버퍼
	TCHAR szFiles[MAX_PATH * 100] = { 0 };
	dlg.m_ofn.lpstrFile = szFiles;
	dlg.m_ofn.nMaxFile = MAX_PATH * 100;

	if (dlg.DoModal() == IDCANCEL)
		return;

	int insertPos = m_selected[0] + 1;
	int src_frame_delay = m_img.get_frame_delay(m_selected[0]);
	if (src_frame_delay <= 0)
	{
		src_frame_delay = 30;
		m_img.set_frame_delay(m_selected[0], src_frame_delay);
	}

	UINT targetW = (UINT)m_img.get_width();
	UINT targetH = (UINT)m_img.get_height();
	ID2D1DeviceContext* d2dc = m_img.get_d2dc();

	POSITION pos = dlg.GetStartPosition();

	while (pos)
	{
		CString filePath = dlg.GetNextPathName(pos);
		theApp.WriteProfileString(_T("setting"), _T("recent insert frame path"), filePath);

		// 외부 이미지 로드 (auto_play = false)
		CSCD2Image srcImg;
		srcImg.load(m_img.get_WICFactory2(), d2dc, filePath, false);
		if (!srcImg.is_valid())
			continue;

		// 현재 애니메이션과 동일한 크기의 빈 프레임 생성
		CSCD2Image frame_img;
		frame_img.create(m_img.get_WICFactory2(), d2dc, targetW, targetH);

		ID2D1Bitmap1* pTarget = frame_img.get_frame_img(0);
		ID2D1Bitmap1* pSrc = srcImg.get_frame_img(0);
		if (!pTarget || !pSrc)
			continue;

		// source를 target 크기로 리사이즈하여 그린다
		ComPtr<ID2D1Image> oldTarget;
		d2dc->GetTarget(&oldTarget);
		d2dc->SetTarget(pTarget);
		d2dc->BeginDraw();
		d2dc->Clear(D2D1::ColorF(0, 0, 0, 0));
		D2D1_RECT_F ratio_rect = get_ratio_rect(D2D1::RectF(0, 0, (float)targetW, (float)targetH), srcImg.get_width(), srcImg.get_height());
		d2dc->DrawBitmap(pSrc, ratio_rect);
		d2dc->EndDraw();
		d2dc->SetTarget(oldTarget.Get());

		// 선택된 프레임 뒤에 순차 삽입
		m_img.get_img_list()->insert(m_img.get_img_list()->begin() + insertPos,
			std::move(frame_img.get_frame_img(0)));
		m_img.get_frame_delay_list()->insert(m_img.get_frame_delay_list()->begin() + insertPos,
			src_frame_delay);
		insertPos++;
	}

	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_image_info(m_img.get_frame_count(), m_img.get_width(), m_img.get_height());
	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_duration_info(*m_img.get_frame_delay_list());

	recalc_scrollbars();

	apply_to_preview();

	Invalidate();
}

void CAniMakerView::OnMenuInsertFrameEmpty()
{
	if (m_selected.size() != 1)
	{
		show_message(_T("빈 프레임 추가 기능은 하나의 프레임을 선택했을때만 가능합니다."));
		return;
	}

	CSCD2Image frame_img;

	frame_img.create(m_img.get_WICFactory2(), m_img.get_d2dc(), m_img.get_width(), m_img.get_height());

	//D2D1_POINT_2U pt = { 0, 0 };
	//D2D1_RECT_U r = { 0, 0, m_img.get_width(), m_img.get_height() };
	//frame_img.get_frame_img(0)->CopyFromBitmap(&pt, m_img.get_frame_img(m_selected[0]), &r);
	m_img.get_img_list()->insert(m_img.get_img_list()->begin() + m_selected[0], std::move(frame_img.get_frame_img(0)));
	m_img.get_frame_delay_list()->insert(m_img.get_frame_delay_list()->begin() + m_selected[0], m_img.get_frame_delay(m_selected[0]));

	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_image_info(m_img.get_frame_count(), m_img.get_width(), m_img.get_height());
	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_duration_info(*m_img.get_frame_delay_list());

	recalc_scrollbars();

	apply_to_preview();

	Invalidate();
}

void CAniMakerView::OnMenuFrameProperty()
{
	if (m_selected.size() == 0)
		return;

	CFrameProperty	dlg;
	dlg.m_frame_delay = m_img.get_frame_delay_list()->at(m_selected[0]);

	if (dlg.DoModal() == IDCANCEL)
		return;

	int new_delay = dlg.m_frame_delay;

	for (auto selected : m_selected)
		m_img.set_frame_delay(selected, new_delay);
	
	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_duration_info(*m_img.get_frame_delay_list());

	Invalidate();

	apply_to_preview();
}

void CAniMakerView::OnMenuSaveFrameAs()
{
	if (m_selected.size() > 1)
	{
		int res = AfxMessageBox(_T("멀티 선택 시 프레임 저장 기능은 맨 처음 선택된 프레임만 저장합니다."), MB_OKCANCEL | MB_ICONEXCLAMATION);
		if (res == IDCANCEL)
			return;
	}

	CString recent = theApp.GetProfileString(_T("setting"), _T("recent saved frame file"), _T(""));
	CFileDialog dlg(FALSE, _T("png"), recent, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("PNG Files (*.png)|*.png|All Files (*.*)|*.*||"));

	if (dlg.DoModal() == IDCANCEL)
		return;

	recent = dlg.GetPathName();
	theApp.WriteProfileString(_T("setting"), _T("recent saved frame file"), recent);
	m_img.save(m_img.get_frame_img(m_selected[0]), 1.0f, recent);
}


void CAniMakerView::OnMenuViewAnimation()
{
	OnMenuPreview();
}

void CAniMakerView::OnMenuSaveAs()
{
	CString path = m_img.get_filename();

	if (path.IsEmpty())
		path = get_exe_directory(true) + _T("untitled.webp");

	CString recent = theApp.GetProfileString(_T("setting"), _T("recent saved file"), _T(""));
	CFileDialog dlg(FALSE, _T("*"), recent, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("WebP Files (*.webp)|*.webp|Gif Files (*.gif)|*.gif||"));

	if (dlg.DoModal() == IDCANCEL)
		return;

	recent = dlg.GetPathName();
	theApp.WriteProfileString(_T("setting"), _T("recent saved file"), recent);

	if (dlg.GetFileExt().CompareNoCase(_T("gif")) == 0)
		m_img.save_gif(recent);
	else if (dlg.GetFileExt().CompareNoCase(_T("webp")) == 0)
		m_img.save_webp(recent);
	else
	{
		AfxMessageBox(_T("지원하지 않는 파일 형식입니다."));
	}
}

bool CAniMakerView::load(CString path)
{
	m_img.load(m_d2dc.get_WICFactory(), m_d2dc.get_d2dc(), path);

	if (!pDoc)
		pDoc = GetDocument();

	//zoom 1.0 = 실제 이미지 크기가 되도록 썸네일 크기를 이미지 높이로 설정
	if (m_img.is_valid() && m_img.get_height() > 0)
		m_sz_thumb = (float)m_img.get_height();

	pDoc->SetTitle(m_img.get_filename());
	theApp.AddToRecentFileList(path);

	m_preview.set_title(m_img.get_filename());

	CMainFrame* pMain = (CMainFrame*)(AfxGetApp()->m_pMainWnd);
	pMain->set_image_info(m_img.get_frame_count(), (int)m_img.get_width(), (int)m_img.get_height());
	pMain->set_duration_info(*m_img.get_frame_delay_list());

	recalc_scrollbars();
	Invalidate();

	return true;
}

void CAniMakerView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	if (!bActivate)
		return;

	// 활성화된 View의 이미지 정보를 status bar에 반영
	CMainFrame* pMain = (CMainFrame*)(AfxGetApp()->m_pMainWnd);
	if (!pMain)
		return;

	if (m_img.is_valid())
	{
		pMain->set_image_info(m_img.get_frame_count(), (int)m_img.get_width(), (int)m_img.get_height());
		pMain->set_duration_info(*m_img.get_frame_delay_list());
	}
	else
	{
		pMain->set_image_info(0, 0, 0);
		pMain->set_status_text(status_duration_info, _T(""));
	}

	pMain->set_zoom_info(m_zoom);
}

void CAniMakerView::OnFileSave()
{
	CString path = m_img.get_filename(true);

	// 경로가 없거나 저장 가능한 형식이 아니면 Save As로 전환
	CString ext = get_part(path, fn_ext);

	if (path.IsEmpty() || (ext.CompareNoCase(_T("webp")) != 0 && ext.CompareNoCase(_T("gif")) != 0))
	{
		OnMenuSaveAs();
		return;
	}

	if (ext.CompareNoCase(_T("gif")) == 0)
		m_img.save_gif(path);
	else
		m_img.save_webp(path);
}

// ─── Undo/Redo 유틸리티 ───

ComPtr<ID2D1Bitmap1> CAniMakerView::clone_bitmap(ID2D1Bitmap1* src)
{
	if (!src)
		return nullptr;

	UINT w = (UINT)m_img.get_width();
	UINT h = (UINT)m_img.get_height();

	CSCD2Image temp;
	temp.create(m_img.get_WICFactory2(), m_img.get_d2dc(), w, h);

	D2D1_POINT_2U pt = { 0, 0 };
	D2D1_RECT_U r = { 0, 0, w, h };
	temp.get_frame_img(0)->CopyFromBitmap(&pt, src, &r);

	return ComPtr<ID2D1Bitmap1>(temp.get_frame_img(0));
}

void CAniMakerView::update_ui_after_edit()
{
	CMainFrame* pMain = (CMainFrame*)(AfxGetApp()->m_pMainWnd);
	pMain->set_image_info(m_img.get_frame_count(), m_img.get_width(), m_img.get_height());
	pMain->set_duration_info(*m_img.get_frame_delay_list());

	recalc_scrollbars();

	apply_to_preview();

	Invalidate();
}

void CAniMakerView::push_undo(UndoAction&& action)
{
	action.prev_selected = m_selected;

	m_undo_stack.push_back(std::move(action));

	if ((int)m_undo_stack.size() > MAX_UNDO)
		m_undo_stack.pop_front();

	//새 작업이 들어오면 redo 스택은 무효화
	m_redo_stack.clear();
}

void CAniMakerView::perform_undo()
{
	if (m_undo_stack.empty())
		return;

	UndoAction action = std::move(m_undo_stack.back());
	m_undo_stack.pop_back();

	// Redo용으로 현재 상태를 역방향 액션으로 저장
	UndoAction redo_action;
	redo_action.prev_selected = m_selected;

	switch (action.type)
	{
	case eUndoType::InsertFrames:
	{
		// Undo: 삽입된 프레임들을 제거
		// Redo: 제거된 프레임들을 다시 삽입
		redo_action.type = eUndoType::DeleteFrames;
		redo_action.indices = action.indices;
		for (int idx : action.indices)
		{
			redo_action.saved_frames.push_back(clone_bitmap(m_img.get_frame_img(idx)));
			redo_action.saved_delays.push_back(m_img.get_frame_delay(idx));
		}

		// 뒤에서부터 제거 (인덱스 밀림 방지)
		std::deque<int> sorted_indices = action.indices;
		std::sort(sorted_indices.begin(), sorted_indices.end(), std::greater<int>());
		for (int idx : sorted_indices)
		{
			m_img.get_img_list()->erase(m_img.get_img_list()->begin() + idx);
			m_img.get_frame_delay_list()->erase(m_img.get_frame_delay_list()->begin() + idx);
		}
		break;
	}
	case eUndoType::DeleteFrames:
	{
		// Undo: 삭제된 프레임들을 원래 위치에 복원
		// Redo: 복원된 프레임들을 다시 삭제
		redo_action.type = eUndoType::InsertFrames;
		redo_action.indices = action.indices;

		for (size_t i = 0; i < action.indices.size(); i++)
		{
			int idx = action.indices[i];
			ComPtr<ID2D1Bitmap1> bmp = clone_bitmap(action.saved_frames[i].Get());

			m_img.get_img_list()->insert(m_img.get_img_list()->begin() + idx, std::move(bmp.Get()));
			m_img.get_frame_delay_list()->insert(m_img.get_frame_delay_list()->begin() + idx, action.saved_delays[i]);
		}
		break;
	}
	case eUndoType::ModifyFrames:
	{
		// Undo: 원본 비트맵+딜레이 복원
		// Redo: 현재 비트맵+딜레이 저장
		redo_action.type = eUndoType::ModifyFrames;
		redo_action.indices = action.indices;

		UINT w = (UINT)m_img.get_width();
		UINT h = (UINT)m_img.get_height();
		D2D1_POINT_2U pt = { 0, 0 };
		D2D1_RECT_U r = { 0, 0, w, h };

		for (size_t i = 0; i < action.indices.size(); i++)
		{
			int idx = action.indices[i];
			ID2D1Bitmap1* pCur = m_img.get_frame_img(idx);

			// 현재 상태를 redo에 저장
			redo_action.saved_frames.push_back(clone_bitmap(pCur));
			redo_action.saved_delays.push_back(m_img.get_frame_delay(idx));

			// 원본으로 복원
			if (pCur && action.saved_frames[i])
				pCur->CopyFromBitmap(&pt, action.saved_frames[i].Get(), &r);

			m_img.set_frame_delay(idx, action.saved_delays[i]);
		}
		break;
	}
	}

	m_selected = action.prev_selected;
	m_redo_stack.push_back(std::move(redo_action));
	update_ui_after_edit();
}

void CAniMakerView::perform_redo()
{
	if (m_redo_stack.empty())
		return;

	UndoAction action = std::move(m_redo_stack.back());
	m_redo_stack.pop_back();

	// Undo용 역방향 액션 생성
	UndoAction undo_action;
	undo_action.prev_selected = m_selected;

	switch (action.type)
	{
	case eUndoType::InsertFrames:
		// Redo에서 InsertFrames = 원래 Undo에서 삭제한 것을 다시 삭제
		undo_action.type = eUndoType::DeleteFrames;
		undo_action.indices = action.indices;
		for (int idx : action.indices)
		{
			undo_action.saved_frames.push_back(clone_bitmap(m_img.get_frame_img(idx)));
			undo_action.saved_delays.push_back(m_img.get_frame_delay(idx));
		}
		{
			std::deque<int> sorted = action.indices;
			std::sort(sorted.begin(), sorted.end(), std::greater<int>());
			for (int idx : sorted)
			{
				m_img.get_img_list()->erase(m_img.get_img_list()->begin() + idx);
				m_img.get_frame_delay_list()->erase(m_img.get_frame_delay_list()->begin() + idx);
			}
		}
		break;

	case eUndoType::DeleteFrames:
		// Redo에서 DeleteFrames = 원래 삽입한 것을 다시 삽입
		undo_action.type = eUndoType::InsertFrames;
		undo_action.indices = action.indices;
		for (size_t i = 0; i < action.indices.size(); i++)
		{
			int idx = action.indices[i];
			ComPtr<ID2D1Bitmap1> bmp = clone_bitmap(action.saved_frames[i].Get());
			m_img.get_img_list()->insert(m_img.get_img_list()->begin() + idx, std::move(bmp.Get()));
			m_img.get_frame_delay_list()->insert(m_img.get_frame_delay_list()->begin() + idx, action.saved_delays[i]);
		}
		break;

	case eUndoType::ModifyFrames:
		undo_action.type = eUndoType::ModifyFrames;
		undo_action.indices = action.indices;
		{
			UINT w = (UINT)m_img.get_width();
			UINT h = (UINT)m_img.get_height();
			D2D1_POINT_2U pt = { 0, 0 };
			D2D1_RECT_U r = { 0, 0, w, h };
			for (size_t i = 0; i < action.indices.size(); i++)
			{
				int idx = action.indices[i];
				ID2D1Bitmap1* pCur = m_img.get_frame_img(idx);
				undo_action.saved_frames.push_back(clone_bitmap(pCur));
				undo_action.saved_delays.push_back(m_img.get_frame_delay(idx));
				if (pCur && action.saved_frames[i])
					pCur->CopyFromBitmap(&pt, action.saved_frames[i].Get(), &r);
				m_img.set_frame_delay(idx, action.saved_delays[i]);
			}
		}
		break;
	}

	m_selected = action.prev_selected;
	m_undo_stack.push_back(std::move(undo_action));
	update_ui_after_edit();
}

// ─── 메시지 핸들러 ───

void CAniMakerView::OnEditUndo()
{
	perform_undo();
}

void CAniMakerView::OnEditRedo()
{
	perform_redo();
}

void CAniMakerView::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_undo_stack.empty());
}

void CAniMakerView::OnUpdateEditRedo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_redo_stack.empty());
}

//이미지 정보 또는 frame 정보가 변경되면 즉시 preview창에 적용시킨다.
//단, preview 창이 열려있는 않으면 굳이 매번 적용시킬 필요는 없다.
void CAniMakerView::apply_to_preview()
{
	pDoc->SetModifiedFlag(TRUE);

	if (m_preview.IsWindowVisible())
		m_preview.set_image(&m_img);
}

void CAniMakerView::OnMenuZoom50()
{
	m_zoom = 0.5f;
	recalc_scrollbars();
	Invalidate();
	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_zoom_info(m_zoom);
}

void CAniMakerView::OnMenuZoom100()
{
	m_zoom = 1.0f;
	recalc_scrollbars();
	Invalidate();
	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_zoom_info(m_zoom);
}


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
	CRect rc;

	GetClientRect(rc);

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
				D2D1_RECT_F rc = D2D1::RectF(x, y, x + thumbW, y + thumbH);
				d2dc->DrawBitmap(pFrame, rc);
				draw_rect(d2dc, rc, Gdiplus::Color::LightGray, Gdiplus::Color::Transparent, 1.0f);

				str.Format(_T("F:%d D:%d"), i, m_img.get_frame_delay(i));
				D2D1_RECT_F rtext = rc;
				rtext.top = rc.bottom + 8;
				rtext.bottom = rtext.top + 24;
				draw_text(d2dc, rtext, str, _T("Arial"), 12.0f / m_zoom, FW_NORMAL, Gdiplus::Color::Black, Gdiplus::Color::LightGray, DT_CENTER | DT_TOP);
				if (i == m_index)
				{
					inflate_rect(rc, 0, 0);
					draw_rect(d2dc, rc, Gdiplus::Color::RoyalBlue, Gdiplus::Color::Transparent, 2.0f);
				}
			}
			x += thumbW + m_thumb_gap;
		}
	}

	d2dc->SetTransform(D2D1::Matrix3x2F::Identity());

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

	m_img.load(m_d2dc.get_WICFactory(), m_d2dc.get_d2dc(), _T("D:\\chart.gif"), false);
	recalc_scrollbars();
	Invalidate();

	// Preview에 D2D 디바이스 공유 설정 (create 전에 호출)
	m_preview.set_shared_d2dc(&m_d2dc);
	m_preview.Create(IDD_PREVIEW, this);

	DragAcceptFiles();
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
			recalc_scrollbars();
			Invalidate(FALSE);
			return TRUE;
		}
		else if (pMsg->wParam == VK_SUBTRACT || pMsg->wParam == VK_OEM_MINUS)
		{
			// '-' 키: 줌 아웃
			m_zoom -= m_zoom_step;
			m_zoom = max(m_zoom, m_zoom_min);
			recalc_scrollbars();
			Invalidate(FALSE);
			return TRUE;
		}
		else if (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT)
		{
			int nFrames = m_img.get_frame_count();
			if (nFrames <= 0)
				return TRUE;

			if (pMsg->wParam == VK_LEFT)
				m_index = max(0, m_index - 1);
			else
				m_index = min(nFrames - 1, m_index + 1);

			// 선택 프레임이 보이도록 자동 스크롤
			ensure_frame_visible(m_index);
			Invalidate(FALSE);
			return TRUE;
		}
		else if (pMsg->wParam == VK_HOME)
		{
			m_index = 0;
			ensure_frame_visible(m_index);
			Invalidate(FALSE);
			return TRUE;
		}
		else if (pMsg->wParam == VK_END)
		{
			int nFrames = m_img.get_frame_count();
			if (nFrames > 0)
				m_index = nFrames - 1;
			ensure_frame_visible(m_index);
			Invalidate(FALSE);
			return TRUE;
		}
		else if (pMsg->wParam == 'C')
		{
			m_img.copy_to_clipboard();
			return TRUE;
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

	// 현재 View/Doc에서 파일 처리
	// 예: 이미지 로드, 프레임 추가 등
	TRACE(_T("Dropped: %s\n"), szFileName);

	m_img.load(m_d2dc.get_WICFactory(), m_d2dc.get_d2dc(), szFileName);

	DragFinish(hDropInfo);
	recalc_scrollbars();
	Invalidate();  // 화면 갱신

	//CView::OnDropFiles(hDropInfo);
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
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	m_index = get_frame_index(point);
	trace(m_index);
	Invalidate();

	CView::OnLButtonDown(nFlags, point);
}

void CAniMakerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CView::OnLButtonUp(nFlags, point);
}

void CAniMakerView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

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

	recalc_scrollbars();
	Invalidate();
	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->set_zoom_info(m_zoom);

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

void CAniMakerView::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

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
		float thumbW = frame_step - m_thumb_gap;
		contentW = m_thumb_margin * 2 + nFrames * thumbW + (nFrames - 1) * m_thumb_gap;
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
	return thumbW + m_thumb_gap;
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
	float frame_step = thumbW + m_thumb_gap;

	// Y 범위 체크
	if (wy < m_thumb_margin || wy > m_thumb_margin + thumbH)
		return -1;

	// X로 인덱스 계산
	float relX = wx - m_thumb_margin;
	if (relX < 0.f)
		return -1;

	int index = (int)(relX / frame_step);

	// gap 영역 클릭 제외 (썸네일 이미지 위만 유효)
	float withinFrame = relX - index * frame_step;
	if (withinFrame > thumbW)
		return -1;

	if (index < 0 || index >= nFrames)
		return -1;

	return index;
}

void CAniMakerView::ensure_frame_visible(int index)
{
	int nFrames = m_img.get_frame_count();
	if (index < 0 || index >= nFrames)
		return;

	float frame_step = get_frame_step();
	float thumbW = frame_step - m_thumb_gap;

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
	m_preview.set_image(&m_img);

	if (m_preview.IsIconic())
		m_preview.ShowWindow(SW_RESTORE);

	m_preview.ShowWindow(SW_SHOW);
}

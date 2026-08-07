// AniMakerView.h: CAniMakerView 클래스의 인터페이스
//

#pragma once

#include "Common/directx/CSCD2Context/SCD2Context.h"
#include "Common/directx/CSCD2Image/SCD2Image.h"
#include "Common/CDialog/SCShapeDlg/SCShapeDlg.h"

#include "PreviewDlg.h"

class CAniMakerDoc;

//Undo/Redo 액션 타입
enum class eUndoType
{
	InsertFrames,		// 프레임 삽입됨 → Undo 시 제거
	DeleteFrames,		// 프레임 삭제됨 → Undo 시 복원
	ModifyFrames,		// 프레임 내용 변경됨 → Undo 시 원본 복원
};

struct UndoAction
{
	eUndoType			type;
	std::deque<int>		indices;		// 대상 프레임 인덱스들

	// DeleteFrames, ModifyFrames: 원본 비트맵+딜레이 백업
	std::deque<ComPtr<ID2D1Bitmap1>> saved_frames;
	std::deque<int>		saved_delays;

	// InsertFrames: 삽입된 프레임 수 (Undo 시 제거할 개수)
	int					insert_count = 0;
	int					insert_pos = 0;		// 삽입 시작 위치

	// 이전 선택 상태 복원용
	std::deque<int>		prev_selected;
};

class CAniMakerView : public CView
{
protected: // serialization에서만 만들어집니다.
	CAniMakerView() noexcept;
	DECLARE_DYNCREATE(CAniMakerView)

// 특성입니다.
public:
	CAniMakerDoc* GetDocument() const;

// 작업입니다.
public:
	bool					load(CString path);
	bool					load_from_clipboard();

private:
	CSCD2Context			m_d2dc;
	CSCD2Image				m_img;
	CPreviewDlg				m_preview;

	float					m_zoom = 1.f;
	D2D1_POINT_2F			m_pt_scroll = D2D1::Point2F(0.f, 0.f);

	float					m_sz_thumb = 100.f;
	float					m_thumb_gap = 12.f;
	float					m_thumb_margin = 12.f;
	float					m_zoom_max = 10.f;
	float					m_zoom_min = 0.1f;
	float					m_zoom_step = 0.1f;

	void					recalc_scrollbars();
	bool					m_in_recalc_scrollbars = false;
	float					get_frame_step();

	//가로 스크롤바에서 1프레임을 표현하는 단위. 스크롤바 값 = 프레임 인덱스 * 이 값.
	int						get_hscroll_unit();

	//값이 실제로 바뀐 경우에만 SetScrollInfo를 호출한다.
	void					update_scrollbar(int bar, const SCROLLINFO& si);

	//사용자가 스크롤바 썸을 실제로 끌고 있는 중인지. 이때는 스크롤바에 위치를 되돌려 쓰지 않는다.
	bool					m_thumb_dragging = false;
	void					ensure_frame_visible(int index);

	//zoom을 유효범위로 제한해 적용하고 레지스트리 저장 + 상태바 표시까지 함께 처리한다.
	void					set_zoom(float zoom);

	//OnDraw에서 쓰는 브러시와 text format을 준비한다. font_size가 바뀐 경우에만 text format을 다시 만든다.
	void					prepare_draw_resources(float font_size);

	ComPtr<IDWriteFactory>			m_write_factory;
	ComPtr<IDWriteTextFormat>		m_text_format;
	float							m_text_format_size = 0.f;
	ComPtr<ID2D1SolidColorBrush>	m_br_text;
	ComPtr<ID2D1SolidColorBrush>	m_br_border;
	ComPtr<ID2D1SolidColorBrush>	m_br_selected;

	//선택된 인덱스
	std::deque<int>			m_selected;
	int						m_anchor_index = -1;	//Shift+클릭 범위 선택용 앵커
	int						get_frame_index(CPoint pt);
	void					get_frames_in_rect(D2D1_RECT_F rectWorld, std::deque<int>& result);

	//드래그 선택 (rubber band)
	bool					m_bDragSelecting = false;
	CPoint					m_ptDragStart;
	CPoint					m_ptDragCurrent;
	std::deque<int>			m_selected_before_drag;

	//내부 복사/붙여넣기용 프레임 저장
	std::deque<ComPtr<ID2D1Bitmap1>> m_copied_frames;
	std::deque<int>			m_copied_delays;

	//Undo/Redo
	std::deque<UndoAction>	m_undo_stack;
	std::deque<UndoAction>	m_redo_stack;
	static const int		MAX_UNDO = 50;

	void					push_undo(UndoAction&& action);
	void					perform_undo();
	void					perform_redo();
	ComPtr<ID2D1Bitmap1>	clone_bitmap(ID2D1Bitmap1* src);
	void					update_ui_after_edit();


	CSCShapeDlg				m_message;
	void					show_message(CString message);

	//이미지 정보 또는 frame 정보가 변경되면 즉시 preview창에 적용
	void					apply_to_preview();

// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 구현입니다.
public:
	virtual ~CAniMakerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CAniMakerDoc*			pDoc = NULL;

// 생성된 메시지 맵 함수
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMenuPreview();
	afx_msg void OnMenuCut();
	afx_msg void OnUpdateMenuCut(CCmdUI* pCmdUI);
	afx_msg void OnMenuCopy();
	afx_msg void OnMenuPasteIntoSelectedFrame();
	afx_msg void OnMenuPasteBeforeCurrentFrame();
	afx_msg void OnMenuPasteAfterCurrentFrame();
	afx_msg void OnMenuDelete();
	afx_msg void OnMenuDuplicateSelected();
	afx_msg void OnMenuInsertFrameFromFile();
	afx_msg void OnMenuInsertFrameEmpty();
	afx_msg void OnMenuFrameProperty();
	afx_msg void OnMenuSaveFrameAs();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMenuViewAnimation();
	afx_msg void OnMenuSaveAs();
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	afx_msg void OnFileSave();
	afx_msg void OnEditUndo();
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);

	afx_msg void OnMenuZoom50();
	afx_msg void OnMenuZoom100();
	afx_msg void OnPasteFromClipboard();
	afx_msg void OnPasteAsNewAnimation();
	afx_msg void OnUpdatePasteAsNewAnimation(CCmdUI* pCmdUI);
	afx_msg void OnMenuViewMakeTransparentBack();
	afx_msg void OnMenuSelectAll();
	afx_msg void OnFileSaveAs();
};

#ifndef _DEBUG  // AniMakerView.cpp의 디버그 버전
inline CAniMakerDoc* CAniMakerView::GetDocument() const
   { return reinterpret_cast<CAniMakerDoc*>(m_pDocument); }
#endif


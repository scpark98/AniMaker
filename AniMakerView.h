
// AniMakerView.h: CAniMakerView 클래스의 인터페이스
//

#pragma once

#include "Common/directx/CSCD2Context/SCD2Context.h"
#include "Common/directx/CSCD2Image/SCD2Image.h"

#include "PreviewDlg.h"

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

private:
	CSCD2Context			m_d2dc;
	CSCD2Image				m_img;
	CPreviewDlg				m_preview;

	float					m_zoom = 1.f;
	D2D1_POINT_2F			m_pt_scroll = D2D1::Point2F(0.f, 0.f);

	float					m_sz_thumb = 100.f;
	float					m_thumb_gap = 8.f;
	float					m_thumb_margin = 12.f;
	float					m_zoom_max = 8.f;
	float					m_zoom_min = 0.4f;
	float					m_zoom_step = 0.2f;

	void					recalc_scrollbars();
	float					get_frame_step();
	void					ensure_frame_visible(int index);

	int						m_index = -1;
	int						get_frame_index(CPoint pt);

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
};

#ifndef _DEBUG  // AniMakerView.cpp의 디버그 버전
inline CAniMakerDoc* CAniMakerView::GetDocument() const
   { return reinterpret_cast<CAniMakerDoc*>(m_pDocument); }
#endif


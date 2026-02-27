
// MainFrm.h: CMainFrame 클래스의 인터페이스
//

#pragma once

#include <deque>

//.cpp의 static UINT indicators[]에 정의된 ID_SEPARATOR의 갯수와 enum status_id의 갯수는 일치해야 한다.
enum status_id
{
	status_default = 0,
	status_image_info,		//total frames, image size
	status_duration_info,	//duration
	status_zoom_info,		//zoom percentage
	status_selected_info,	//selected info
};

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame() noexcept;

// 특성입니다.
public:

// 작업입니다.
public:
	void				set_image_info(int total_frames, int width, int height);
	void				set_duration_info(std::deque<int>& frame_delays);
	void				set_zoom_info(float zoom);
	void				set_status_text(int index, CString text);

// 재정의입니다.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 구현입니다.
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 컨트롤 모음이 포함된 멤버입니다.
	CToolBar          m_wndToolBar;
	CStatusBar        m_wndStatusBar;

// 생성된 메시지 맵 함수
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnCheckChildFrames(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
//	afx_msg void OnColorizationColorChanged(DWORD dwColorizationColor, BOOL bOpacity);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
};



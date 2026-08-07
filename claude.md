@../Common/claude.md

# AniMaker 프로젝트 규칙·맥락

## 1. 프로젝트 정체

**애니메이션 이미지(GIF / WebP / APNG) 프레임 편집기.** MFC MDI 데스크톱 앱.

- 목적: animated gif/webp 를 열어 **프레임 단위로** 보고, 자르고, 붙이고, 삽입/삭제/복제하고, 프레임별 delay 를 바꾸고, 다시 gif/webp 로 저장.
- 뷰어가 아니라 **에디터**. 재생(뷰어) 역할은 별도 Preview 창(`CPreviewDlg`)이 담당.
- 회사: `SetRegistryKey(_T("Legends Software"))` → 설정은 `HKCU\Software\Legends Software\AniMaker` 에 저장.
- 렌더링은 전부 **Direct2D + WIC** (`Common/directx` 의 `CSCD2Context` / `CSCD2Image`). GDI/GDI+ 는 색 상수(`Gdiplus::Color`) 용도로만 쓰인다.

## 2. 구조 (MFC MDI Doc/View)

| 클래스 | 파일 | 역할 |
|---|---|---|
| `CAniMakerApp` | AniMaker.cpp | 앱. File New/Open/Close, "Paste as new animation" 을 **앱 레벨**에서 처리 (새 Doc/Frame/View 를 만들고 View 에 load 위임) |
| `CMainFrame` | MainFrm.cpp | MDI 프레임. CMFCToolBar + CMFCStatusBar(6 pane) + 상태바에 얹은 `CProgressCtrl` |
| `CChildFrame` | ChildFrm.cpp | MDI 자식. 닫힐 때 `WM_CHECK_CHILD_FRAMES` 를 MainFrame 에 Post → 마지막 child 가 닫히면 상태바 초기화 |
| `CAniMakerDoc` | AniMakerDoc.cpp | **거의 빈 껍데기**. `m_file` 경로만 보관. Serialize 미구현 |
| `CAniMakerView` | AniMakerView.cpp (~1850줄) | **사실상 전부**. 이미지 데이터(`m_img`), 필름스트립 렌더링, 선택, 편집, Undo/Redo, 저장 |
| `CPreviewDlg` | PreviewDlg.cpp | 모델리스 재생 창. View 마다 1개. 내부에 `CSCD2ImageDlg` |
| `CFrameProperty` | FrameProperty.cpp | frame delay 입력 모달 다이얼로그 |

**중요 — 데이터가 Doc 이 아니라 View 에 있다.** `CSCD2Image m_img` 는 `CAniMakerView` 의 멤버다. Doc 은 파일 경로만 들고 있고 Serialize 는 TODO 로 비어 있다. 저장은 View 의 `OnFileSave` / `OnMenuSaveAs` 가 `m_img.save_gif/save_webp` 를 직접 호출한다. → **Doc 에 데이터가 있다고 가정하고 코드를 짜지 말 것.**

## 3. 화면 모델 — 가로 필름스트립 + world/screen 좌표계

`OnDraw` 는 프레임들을 **가로 한 줄**로 배치한 필름스트립을 그린다. 각 썸네일 아래 `F:<index> D:<delay>` 텍스트, 선택 항목은 RoyalBlue 4px 테두리, alpha 가 있는 이미지는 zigzag(체커보드) 브러시를 깔고 그린다.

변환 행렬:
```
world → screen : Translation(-m_pt_scroll) * Scale(m_zoom)
screen → world : pt / m_zoom + m_pt_scroll
```

**핵심 규칙 — "world 량" vs "screen 량":**
- **world 량** = 줌에 따라 같이 커져야 하는 것 → 그대로 쓴다. 현재 `m_sz_thumb` (썸네일 높이. load 시 이미지 실제 높이로 세팅해서 zoom 100% = 원본 크기가 되게 함) 하나뿐.
- **screen 량** = 줌과 무관하게 화면에서 항상 같은 픽셀로 보여야 하는 것 → **world 좌표로 쓸 때 반드시 `/ m_zoom`**. `m_thumb_gap`, `m_thumb_margin`, 테두리 두께(1.0f, 4.0f), 폰트 크기(12.0f), 텍스트 영역 오프셋(8.0f, 24.0f), zigzag 크기가 전부 여기 해당하며 현재 모두 `/ m_zoom` 처리되어 있다.

새 시각 요소를 추가할 때는 먼저 이 둘 중 어느 쪽인지 정하고, screen 량이면 예외 없이 `/ m_zoom`.

**알려진 구조적 약점:** 동일한 레이아웃 계산(thumbW, margin, frame_step)이 `OnDraw` / `recalc_scrollbars` / `get_frame_step` / `get_frame_index` / `get_frames_in_rect` / `ensure_frame_visible` **6곳에 중복**되어 있다. 레이아웃 상수를 하나 바꾸면 6곳을 동시에 고쳐야 하고, 하나라도 빠지면 "클릭 위치와 그려진 위치가 어긋나는" 버그가 된다. 레이아웃 변경 작업이 들어오면 먼저 `struct layout { float thumb_w, thumb_h, margin, gap, step; } get_layout()` 로 한 곳에 모으는 것을 제안할 것.

스크롤은 가로가 **프레임 단위 스냅**(`roundf(x / frame_step) * frame_step`)이고, 틸트휠(WM_MOUSEHWHEEL)은 1틱 = 1프레임. 휠 줌은 **커서 위치 고정 줌**(줌 전후 커서 아래 world 좌표 유지).

## 4. 선택 모델

`std::deque<int> m_selected` + `m_anchor_index`. 탐색기와 동일한 규약:
- 클릭 = 단일 선택 / Ctrl+클릭 = 토글 / Shift+클릭 = 앵커~클릭 범위 / Ctrl+Shift+클릭 = 범위 추가
- 빈 영역 드래그 = 러버밴드 선택(Ctrl 누르면 기존 선택에 누적). `m_selected_before_drag` 로 시작 상태 보관
- 선택 안 된 프레임에서 우클릭 → 그 프레임만 선택하고 팝업. 이미 선택된 프레임 우클릭 → 선택 유지

`m_selected` 는 **정렬되어 있지 않다** (클릭 순서대로 push_back). 인덱스 삭제/삽입 시 반드시 정렬 후 뒤에서부터 처리해야 한다 (`perform_undo` 는 `std::sort(..., greater)` 로 처리하고 있음).

## 5. Undo/Redo

`UndoAction { eUndoType type, indices, saved_frames, saved_delays, prev_selected }` 를 `m_undo_stack` / `m_redo_stack` (deque, MAX_UNDO=50) 에 쌓는다. 타입은 InsertFrames / DeleteFrames / ModifyFrames 3종이고, undo 시 역방향 액션을 만들어 redo 스택에 넣는 대칭 구조.

**정착된 패턴 (새 편집 기능은 이대로 작성):**
```cpp
UndoAction action;
action.type = eUndoType::ModifyFrames;      // 또는 Insert/Delete
action.indices = m_selected;
for (int idx : m_selected) {
    action.saved_frames.push_back(clone_bitmap(m_img.get_frame_img(idx)));
    action.saved_delays.push_back(m_img.get_frame_delay(idx));
}
push_undo(std::move(action));
// ... 실제 편집 ...
update_ui_after_edit();   // status bar + recalc_scrollbars + apply_to_preview + Invalidate
```
`OnMenuViewMakeTransparentBack` 이 이 패턴의 레퍼런스 구현이다 (코드에도 그렇게 주석되어 있음).

**미완성 상태 (2026-08 기준):** Undo 가 붙어 있는 건 `OnMenuDelete`, `OnMenuPasteAfterCurrentFrame`, `OnMenuPasteIntoSelectedFrame`, `OnMenuViewMakeTransparentBack` 뿐. `OnMenuPasteBeforeCurrentFrame`, `OnMenuDuplicateSelected`, `OnMenuInsertFrameFromFile`, `OnMenuInsertFrameEmpty`, `OnMenuFrameProperty`(delay 변경), `OnPasteFromClipboard` 은 **Undo 미지원**이라 되돌릴 수 없다. 이 함수들을 건드릴 일이 생기면 위 패턴 적용을 함께 제안할 것.

## 6. 클립보드

두 갈래가 공존한다.
- **내부 클립보드**: `m_copied_frames` / `m_copied_delays` (프레임 비트맵 + delay 를 같이 보존). 붙여넣기 3종(선택 프레임에 덮어쓰기 / 앞에 삽입 / 뒤에 삽입)이 이걸 쓴다.
- **시스템 클립보드**: `m_img.copy_to_clipboard(idx)` / `paste_from_clipboard(idx)`, 그리고 `load_from_clipboard()` (PNG 포맷 → CF_DIBV5 → CF_DIB 순으로 크기를 알아낸 뒤 새 애니메이션 문서 생성). "Paste as new animation" 메뉴가 이 경로.

Copy 시 내부 + 시스템 양쪽에 모두 복사된다.

## 7. 저장 / 로드

- 로드: `m_img.load(...)` — gif/webp/png/jpg/bmp/tif 를 CSCD2Image 가 처리. **APNG 는 이 프로젝트에서 비활성**이다 — `SC_USE_APNG` 를 정의하지 않았고 `Common/SCApng.cpp` + libpng/zlib 소스도 빌드에 포함돼 있지 않아 `load_apng` 자체가 컴파일에서 빠진다. APNG 지원이 필요해지면 그 세 가지를 함께 추가해야 한다. 프레임 1장짜리 이미지는 delay 정보가 없으므로 **`DEFAULT_FRAME_DELAY`(300ms)를 강제 주입**한다. 안 그러면 이후 프레임 편집에서 delay 리스트 인덱스 에러가 난다.
- 저장: **WebP / GIF 만** 지원. `OnFileSave` 는 현재 확장자가 webp/gif 가 아니면 자동으로 Save As 로 전환.
- 프레임 1장 저장(`OnMenuSaveFrameAs`)은 PNG.
- 드래그&드롭은 MainFrame(여러 파일 → 각각 새 문서)과 View(첫 파일만 현재 View 에 로드) 양쪽에 있다.

## 8. 설정 저장 위치

`HKCU\Software\Legends Software\AniMaker` 아래:
- `MainFrame` / `ChildFrame`: 창 위치·showCmd (MainFrame 은 `OnClose` 에서 자기 것과 **활성 child 것까지** 같이 저장)
- `PreviewDlg`: `SaveWindowPosition`/`RestoreWindowPosition` (Common/Functions)
- `setting`: `zoom`(recalc_scrollbars 가 호출될 때마다 저장), `recent opened file`, `recent insert frame path`, `recent saved frame file`
- MRU 16개 (`LoadStdProfileSettings(16)`)

## 9. 빌드

- VS 플랫폼 툴셋 **v145**, C++17, MFC 동적 링크, 유니코드, x86/x64 Debug/Release.
- 외부 의존: **libwebp / libwebpmux / libwebpdemux** — include `D:\1.Projects_C++\Common\directx\webp\include`, lib `...\webp\lib` (경로만 vcxproj 4개 구성 전부에 설정돼 있음). **lib 자체는 `SCD2Image.cpp` 의 `#pragma comment(lib, ...)` 로 링크된다** (`SCD2IMAGE_NO_WEBP` 를 정의하면 빠짐). Release 구성의 `AdditionalDependencies=libwebp.lib;libwebpmux.lib` 는 그래서 중복이며, Debug 구성이 비어 있어도 정상 링크된다 — 이걸 "Debug 설정 누락"으로 오판하지 말 것.
- Common 소스를 프로젝트에 **직접 컴파일 포함**한다(라이브러리 참조 아님): SCD2Context, SCD2Image, SCD2ImageDlg, SCShapeDlg, SCEdit, SCStaticEdit, SCColorPicker, SCDropperDlg, SCScrollbar, SCSliderCtrl, SCStatic, SCThumbCtrl, SCParagraph, Functions, colors, ResizeCtrl, SCGdiplusBitmap, ThreadManager, Win32InputBox, MemoryDC.
- `CoInitializeEx(COINIT_APARTMENTTHREADED)` 고정 — 주석에 명시: **COINIT_MULTITHREADED 로 바꾸면 종료 시 런타임 에러**.

## 10. D2D 팩토리·디바이스 공유 (해결 완료된 과거 이슈)

View 의 `m_d2dc` 와 Preview 의 `m_imgDlg` 가 서로 다른 D2D 팩토리에서 만들어지면 이미지 객체만 넘겨도 그려지지 않는다. 한때 "m_img 를 임시 webp 파일로 저장 후 Preview 가 읽어 재생" 하는 우회로를 썼으나, **2026-02 에 `CPreviewDlg::set_shared_d2dc(&m_d2dc)` 로 디바이스를 공유하는 정식 방식으로 해결**되었다 (`m_preview.Create()` **전에** 호출해야 함). 다시 파일 경유 방식으로 되돌리지 말 것.

## 11. 코드 컨벤션 (이 프로젝트 실제 상태)

- MFC 생성 클래스/핸들러는 MFC 관례(PascalCase, `On*`, `m_` 접두사) 유지.
- 직접 추가한 멤버 함수·변수는 **snake_case** (`recalc_scrollbars`, `get_frame_index`, `apply_to_preview`, `m_sz_thumb`, `m_thumb_gap`).
- 주석은 한국어, `//` 뒤 공백 없이 붙여 쓰는 스타일이 다수.
- 소스 인코딩: `.cpp/.h` 는 **UTF-8 BOM**(`EF BB BF`) — Edit/Write 도구로 편집해도 안전한 상태다. 단 편집 전 첫 바이트 재확인 원칙은 유지 (Common/claude.md §2B).
- `resource.h` / `AniMaker.rc` 는 VS 가 관리 — 직접 편집 금지.

## 12. 열려 있는 이슈 / 알려진 결함

`readme.txt` 에 사용자가 적어둔 현재 관심사: "m_zoom 에 무관하게 일관되게 그려지려면 m_thumb_gap 외에 어떤 변수를 어떻게 처리해야 하는가" → 답은 §3 의 world/screen 분류 규칙이며, 코드상 screen 량은 이미 전부 `/ m_zoom` 처리되어 있다. 남은 실제 문제는 §3 끝의 **레이아웃 계산 6중 중복**이다.

**수정 완료 (2026-08-03):**
1. `PreTranslateMessage` 의 VK_LEFT/RIGHT/HOME/END 가 `m_selected.size() > 1` 일 때만 resize 하고 곧바로 `m_selected[0]` 에 접근해, 선택이 빈 상태(이미지 로드 직후)에서 방향키를 누르면 빈 deque 인덱싱 UB 였다. → 좌우는 빈 선택 시 첫 프레임 선택으로 시작, Home/End 는 `clear()+push_back()` 단일 선택으로 재작성 (Home 에는 프레임 수 체크도 없었다).
2. `CMainFrame::set_duration_info` 가 `total_ms / 1000` 을 계산해 놓고 단위를 `ms` 로 출력 → `s` 로 수정.
3. `OnMenuViewMakeTransparentBack` 이 편집 후 `Invalidate()` 만 호출해 Preview 와 상태바에 반영되지 않던 것 → `update_ui_after_edit()` 로 교체.

**수정 완료 (2026-08-07) — 스크롤바:**

이 항목들은 원인이 겉으로 드러나지 않으니 손대기 전에 반드시 읽을 것.

1. **가로 스크롤바의 단위는 픽셀이 아니라 "프레임 × `WHEEL_DELTA`(120)" 다** (`get_hscroll_unit()`). 이상해 보이지만 이유가 둘 있다.
   - 월드 픽셀을 그대로 범위로 쓰면 231프레임짜리에서 `nMax` 가 175600까지 올라가는데, `WM_HSCROLL` 이 실어 보내는 `nPos` 는 16bit 라 그 위치를 표현하지 못한다. 게다가 MFC 가 `(short)HIWORD` 로 넘기므로 실질 상한은 **32767** 이다. 그래서 프레임 단위로 낮췄다.
   - 그런데 **가로 틸트휠이 `WM_MOUSEHWHEEL` 로 오지 않는다.** 마우스 드라이버가 `GetScrollInfo` 로 현재 위치를 한 번 읽은 뒤, 자기 누산기로 `WHEEL_DELTA` 씩 증감시킨 값을 `WM_HSCROLL + SB_THUMBTRACK` 으로 보낸다. 단위가 "1 = 1프레임" 이면 한 틱에 120프레임을 건너뛴다. **1프레임 = 120** 으로 맞춰야 휠 한 틱이 정확히 한 프레임이 된다. 프레임이 273개를 넘으면 32767 상한 때문에 단위가 줄어들고, 그만큼 한 틱 이동량이 커진다.
2. **`SB_THUMBTRACK` 에서 `nTrackPos` 를 쓰면 안 된다.** 위의 휠은 실제 썸 드래그가 아니라서 Windows 가 `nTrackPos` 를 갱신하지 않고 직전 드래그 값에 고정돼 있다. `nPos` 를 쓴다 (단위가 프레임이라 16bit 로 충분).
3. **진짜 썸 드래그 중에는 스크롤바에 위치를 되돌려 쓰지 않는다** (`m_thumb_dragging`). 프레임 격자로 스냅한 값을 밀어넣으면 사용자가 끄는 위치와 싸워 썸이 좌우로 떨린다. 진짜 드래그와 휠이 만든 가짜 `SB_THUMBTRACK` 은 `::GetCapture() == m_hWnd` 로 구분한다.
4. `recalc_scrollbars` 에 **재진입 가드** + **`SIF_DISABLENOSCROLL`**. 스크롤바를 감추면 클라이언트 크기가 바뀌어 `WM_SIZE → OnSize → recalc_scrollbars` 로 되돌아오고, 가로/세로가 서로의 표시 여부를 뒤집으며 진동한다.
5. **프레임 단위 스냅을 `recalc_scrollbars` 에서 뺐다.** 매 recalc 마다 반올림하면 `ensure_frame_visible` 이 맞춰놓은 위치와 커서 고정 줌의 보정값까지 격자로 끌려간다. 스냅은 프레임 단위로 움직이는 쪽(`OnHScroll`, `OnMouseHWheel`)에서만 한다.
6. zoom 레지스트리 저장을 `recalc_scrollbars` 에서 분리 (`set_zoom`). 스크롤 틱마다 레지스트리에 쓰고 있었다.

**수정 완료 (2026-08-07) — `OnDraw` 성능:** 가시 영역 컬링이 없어 231프레임을 매번 전부 그렸고, 프레임 라벨마다 `Common` 의 `draw_text` 가 텍스트 레이아웃을 새로 만들고 **그림자 이펙트를 13패스** 그렸다(`show_shadow` 기본값이 `true`). → 화면에 걸치는 프레임만 그리고(231장 중 8장), 라벨은 캐시한 `IDWriteTextFormat` + `DrawText`, 테두리는 미리 만든 브러시로 직접 그린다. 실측 0~31ms.

**남은 것:**
- §5 의 Undo 미적용 함수 6개.
- **가로휠을 굴리는 동안 스크롤바가 클래식(3D raised) ↔ 테마(가는 라운드) 모양으로 교대**한다. 위 1번의 마우스 드라이버가 우리 프로세스 안에서 스크롤바를 레거시 경로로 다시 그리기 때문이며, **우리 코드로는 막을 수 없다**(세로휠·썸 드래그 등 드라이버가 개입하지 않는 경로에서는 전혀 나타나지 않음을 확인). 없애려면 `WS_HSCROLL` 을 떼고 가로만 `CSCScrollbar`(Common 의 자체 그리기 스크롤바)로 교체해야 한다. 그 경우 §3 의 레이아웃 6중 중복부터 `get_layout()` 으로 통합하는 것이 선행이다. 기능에는 문제가 없어 보류 중.

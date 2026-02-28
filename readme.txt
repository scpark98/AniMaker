[수정될 내용]
- 각 프레임 및 프레임 이미지의 copy/paste
  copy 후 특정 프레임을 선택하고 paste하면 클립보드 이미지로 덮어써지지만
  특정 프레임이 아닌 곳이 선택된 상태에서 paste하면 새 프레임으로 추가된다.

CAniMakerView::OnDraw()에서 그릴 때 m_zoom에 무관한 크기로 그려지도록 font size도 12.0f / m_zoom로 그리고 zigzag brush도 zigzag_size / m_zoom 크기로 그리도록 했다. 하지만 여전히 m_thumb_gap 은 상수라서 확대/축소 시 보여지는 크기가 달라진다. m_thumb_gap와 같이 또 어떤 변수를 어떻게 처리해야만 m_zoom에 관계없이 일관되게 그려지는가?
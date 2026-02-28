[수정될 내용]


CAniMakerView::OnDraw()에서 그릴 때 m_zoom에 무관한 크기로 그려지도록 font size도 12.0f / m_zoom로 그리고 zigzag brush도 zigzag_size / m_zoom 크기로 그리도록 했다. 하지만 여전히 m_thumb_gap 은 상수라서 확대/축소 시 보여지는 크기가 달라진다. m_thumb_gap와 같이 또 어떤 변수를 어떻게 처리해야만 m_zoom에 관계없이 일관되게 그려지는가?
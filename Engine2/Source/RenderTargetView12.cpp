#include "Stdafx.h"

#include "RenderTargetView.h"


using namespace Kodiak;


RenderTargetView::RenderTargetView()
	: m_rtv()
	, m_rtvHandle()
	, m_currentResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET)
{}
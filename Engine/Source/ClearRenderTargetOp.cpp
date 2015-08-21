#include "Stdafx.h"

#include "ClearRenderTargetOp.h"

#include "CommandList.h"
#include "RenderTargetView.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


ClearRenderTargetOperation::ClearRenderTargetOperation(const shared_ptr<RenderTargetView>& rtv, const XMVECTORF32& color)
	: m_rtv(rtv)
	, m_color(color)
{}


void ClearRenderTargetOperation::PopulateCommandList(const shared_ptr<CommandList>& commandList)
{
	commandList->ClearRenderTargetView(m_rtv, m_color);
}
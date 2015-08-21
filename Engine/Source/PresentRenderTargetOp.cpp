#include "Stdafx.h"

#include "PresentRenderTargetOp.h"

#include "CommandList.h"
#include "RenderTargetView.h"


using namespace Kodiak;
using namespace std;


PresentRenderTargetOperation::PresentRenderTargetOperation(const shared_ptr<RenderTargetView>& rtv)
	: m_rtv(rtv)
{}


void PresentRenderTargetOperation::PopulateCommandList(const shared_ptr<CommandList>& commandList)
{
	commandList->Present(m_rtv);
}
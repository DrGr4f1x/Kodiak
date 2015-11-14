// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "BasicApplication.h"

#include "Engine\Source\ColorBuffer.h"
#include "Engine\Source\CommandList.h"
#include "Engine\Source\Format.h"
#include "Engine\Source\Log.h"
//#include "Engine\Source\Model.h"
#include "Engine\Source\Renderer.h"
#include "Engine\Source\RenderPipeline.h"
#include "Engine\Source\StepTimer.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


BasicApplication::BasicApplication(uint32_t width, uint32_t height, const std::wstring& name)
	: Application(width, height, name)
{}


void BasicApplication::OnInit()
{
	LOG_INFO << "BasicApplication initialize";
	m_renderer->SetWindow(m_width, m_height, m_hwnd);
	
	// Create resources
	m_colorTarget = m_renderer->CreateColorBuffer("Main color buffer", m_width, m_height, 1, ColorFormat::R11G11B10_Float, 
		DirectX::Colors::CornflowerBlue);

	// Setup the root rendering pipeline
	auto pipeline = m_renderer->GetRootPipeline();
	
	pipeline->ClearColor(m_colorTarget);
	
	// Create the box model
	/*BoxModelDesc desc;
	desc.colors[0] = XMFLOAT3(0.0f, 0.0f, 0.0f);
	desc.colors[1] = XMFLOAT3(0.0f, 0.0f, 1.0f);
	desc.colors[2] = XMFLOAT3(0.0f, 1.0f, 0.0f);
	desc.colors[3] = XMFLOAT3(0.0f, 1.0f, 1.0f);
	desc.colors[4] = XMFLOAT3(1.0f, 0.0f, 0.0f);
	desc.colors[5] = XMFLOAT3(1.0f, 0.0f, 1.0f);
	desc.colors[6] = XMFLOAT3(1.0f, 1.0f, 0.0f);
	desc.colors[7] = XMFLOAT3(1.0f, 1.0f, 1.0f);
	desc.genColors = true;
	m_boxModel = MakeBoxModel(m_renderer.get(), desc);*/

	pipeline->Present(m_colorTarget);
}


void BasicApplication::OnUpdate(StepTimer* timer)
{}


void BasicApplication::OnDestroy()
{
	m_renderer->Finalize();
	LOG_INFO << "BasicApplication finalize";
}


bool BasicApplication::OnEvent(MSG msg)
{
	return false;
}
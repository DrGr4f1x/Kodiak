// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "Engine\Source\Application.h"


namespace Kodiak
{

// Forward declarations
class ColorBuffer;
class CommandList;
class DepthBuffer;
class Model;
class Scene;


class BasicApplication : public Application
{
public:
	BasicApplication(uint32_t width, uint32_t height, const std::wstring& name);

protected:
	void OnInit() override;
	void OnUpdate(StepTimer* timer) override;
	void OnDestroy() override;
	
	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	std::shared_ptr<ColorBuffer>	m_colorTarget;
	std::shared_ptr<DepthBuffer>	m_depthBuffer;
	std::shared_ptr<Model>			m_boxModel;
	std::shared_ptr<Scene>			m_mainScene;

	bool	m_isTracking{ false };
	int		m_mouseX{ 0 };
	int		m_mouseY{ 0 };
};

} // namespace Kodiak
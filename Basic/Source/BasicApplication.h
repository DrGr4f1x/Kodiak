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
class Camera;
class CameraController;
class ColorBuffer;
class DepthBuffer;
class RootRenderTask;
class Scene;
class StaticModel;


class BasicApplication : public Application
{
public:
	BasicApplication(uint32_t width, uint32_t height, const std::wstring& name);

protected:
	void OnStartup() override;
	void OnInit() override;
	void OnUpdate(StepTimer* timer) override;
	void OnRender() override;
	void OnDestroy() override;

private:
	// Creation/setup helpers
	void CreateResources();
	void CreateMaterials();
	void CreateModel();
	void SetupScene();
	std::shared_ptr<RootRenderTask> SetupFrame();

private:
	std::shared_ptr<ColorBuffer>		m_colorTarget;
	std::shared_ptr<DepthBuffer>		m_depthBuffer;
	std::shared_ptr<StaticModel>		m_boxModel;
	std::shared_ptr<Scene>				m_mainScene;
	std::shared_ptr<Camera>				m_camera;
	std::shared_ptr<CameraController>	m_cameraController;

	float	m_meshRadians[4];

	bool								m_reverseZ{ true };
};

} // namespace Kodiak
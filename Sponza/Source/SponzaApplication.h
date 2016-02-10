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
class ColorBuffer;
class CommandList;
class DepthBuffer;
#if 0
class Material;
#endif
class RenderPass;
class Scene;
class StaticModel;


class SponzaApplication : public Application
{
public:
	SponzaApplication(uint32_t width, uint32_t height, const std::wstring& name);

protected:
	void OnInit() override;
	void OnUpdate(StepTimer* timer) override;
	void OnDestroy() override;

private:
	// Creation/setup helpers
	void CreateResources();
	void CreateMaterials();
	void CreateModel();
	void SetupScene();
	void SetupPipeline();

private:
	std::shared_ptr<ColorBuffer>	m_colorTarget;
	std::shared_ptr<DepthBuffer>	m_depthBuffer;
	std::shared_ptr<Scene>			m_mainScene;
	std::shared_ptr<Camera>			m_camera;
	std::shared_ptr<StaticModel>	m_sponzaModel;

	std::shared_ptr<RenderPass>		m_basePass;
#if 0
	std::shared_ptr<Material>		m_baseMaterial;
#endif
};

} // namespace Kodiak
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
#include "Engine\Source\Color.h"

namespace Kodiak
{

// Forward declarations
class Camera;
class CameraController;
class ColorBuffer;
class CommandList;
class DepthBuffer;
class FXAA;
class ParticleEffectManager;
class PostProcessing;
class SSAO;
class Scene;
class ShadowBuffer;
class ShadowCamera;
class StaticModel;


class SponzaApplication : public Application
{
public:
	SponzaApplication(uint32_t width, uint32_t height, const std::wstring& name);

protected:
	void OnStartup() override;
	void OnInit() override;
	void OnUpdate(StepTimer* timer) override;
	void OnRender() override;
	void OnDestroy() override;

private:
	// Creation/setup helpers
	void CreateResources();
#if 0
	void CreateParticleEffects();
#endif
	void CreateEffects();
	void CreateModel();
	void SetupScene();

private:
	std::shared_ptr<ColorBuffer>		m_colorTarget;
	std::shared_ptr<DepthBuffer>		m_depthBuffer;
	std::shared_ptr<ColorBuffer>		m_linearDepthBuffer;
	std::shared_ptr<ColorBuffer>		m_ssaoFullscreen;
	std::shared_ptr<ColorBuffer>		m_postEffectsBuffer;
	std::shared_ptr<ColorBuffer>		m_lumaBuffer;

	std::shared_ptr<Scene>				m_mainScene;
	std::shared_ptr<Camera>				m_camera;
	std::shared_ptr<CameraController>	m_cameraController;
	std::shared_ptr<ParticleEffectManager> m_particleEffectManager;
	std::shared_ptr<StaticModel>		m_sponzaModel;
	std::shared_ptr<SSAO>				m_ssao;
	std::shared_ptr<PostProcessing>		m_postProcessing;
	std::shared_ptr<FXAA>				m_fxaa;

	std::shared_ptr<ShadowBuffer>		m_shadowBuffer;
	std::shared_ptr<ShadowCamera>		m_shadowCamera;

	bool								m_reverseZ{ true };
	float								m_elapsedTime{ 0.0f };
};

} // namespace Kodiak
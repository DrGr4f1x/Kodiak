// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "ConstantBuffer12.h"
#include "PipelineState12.h"
#include "Shader.h"

#include <ppltasks.h>

namespace Kodiak
{

// Forward declarations
class ConstantBuffer;
#if 0
class Effect;
#endif
class RenderPass;
class RootSignature;
struct ShaderConstantBufferDesc;


class Material
{
	friend class MaterialManager;

public:
	concurrency::task<void> loadTask;

public:
	Material();

	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() const { return m_name; }

#if 0
	void SetEffect(std::shared_ptr<Effect> effect);
#endif

	void SetRenderPass(std::shared_ptr<RenderPass> renderPass);
	std::shared_ptr<RenderPass> GetRenderPass();

private:
	std::string						m_name;

#if 0
	std::shared_ptr<Effect>			m_effect;
#endif
	std::shared_ptr<RenderPass>		m_renderPass;

	bool							m_usesPerViewData{ false };
	bool							m_usesPerObjectData{ false };
};


} // namespace Kodiak
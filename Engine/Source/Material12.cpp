// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Material.h"

#if 0
#include "Effect.h"
#endif
#include "RenderPass.h"
#include "RootSignature12.h"


using namespace Kodiak;
using namespace std;


Material::Material()
{
	loadTask = concurrency::create_task([] {});
}


#if 0
void Material::SetEffect(shared_ptr<Effect> effect)
{
	// TODO: This should happen on the render thread, always

	m_effect = effect;
	
	if (m_effect)
	{
		loadTask = concurrency::create_task([this]
		{
			// Wait on the effect to finish loading
			m_effect->loadTask.wait();

			// Create constant buffers, resources, and parameters
			const auto& effectSig = m_effect->GetSignature();

			m_usesPerViewData = effectSig.perViewDataSize != 0;
			m_usesPerObjectData = effectSig.perObjectDataSize != 0;


		});
	}
	else
	{
		// Null effect, so clear old constant buffers, resources, and parameters
	}
}
#endif


void Material::SetRenderPass(shared_ptr<RenderPass> renderPass)
{
	m_renderPass = renderPass;
}


shared_ptr<RenderPass> Material::GetRenderPass()
{
	return m_renderPass;
}
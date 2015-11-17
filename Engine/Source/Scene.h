// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

namespace Kodiak
{

// Forward declarations
class ConstantBuffer;
class GraphicsCommandList;
class GraphicsPSO;
class Model;


class Scene
{
public:
	Scene(uint32_t width, uint32_t height);

	void AddModel(std::shared_ptr<Model> model);

	void Render(GraphicsCommandList& commandList);

private:
	void Initialize();

private:
	std::vector<std::shared_ptr<Model>> m_models;
	std::shared_ptr<ConstantBuffer>		m_perViewConstantBuffer;
	std::shared_ptr<ConstantBuffer>		m_perObjectConstantBuffer;
	std::shared_ptr<GraphicsPSO>		m_pso;

	struct PerViewConstants
	{
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	} m_perViewConstants;

	struct PerObjectConstants
	{
		DirectX::XMFLOAT4X4 model;
	} m_perObjectConstants;

	uint32_t m_width;
	uint32_t m_height;
};


} // namespace Kodiak

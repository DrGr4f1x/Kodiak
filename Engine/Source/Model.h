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

#if 0
// Forward declarations
class IndexBuffer;
class Renderer;
class VertexBuffer;

enum class PrimitiveTopology;

class MeshPart
{
public:
	MeshPart(
		std::shared_ptr<VertexBuffer> vbuffer,
		std::shared_ptr<IndexBuffer> ibuffer,
		PrimitiveTopology topology,
		uint32_t indexCount,
		uint32_t startIndex,
		int32_t baseVertex);

private:
	std::shared_ptr<VertexBuffer>	m_vertexBuffer;
	std::shared_ptr<IndexBuffer>	m_indexBuffer;
	PrimitiveTopology				m_topology;
	uint32_t						m_indexCount;
	uint32_t						m_startIndex;
	int32_t							m_baseVertexOffset;
};

class Mesh
{
public:
	
	void SetMeshParts(std::vector<MeshPart> meshParts);

private:
	std::vector<MeshPart> m_meshParts;
};

class Model
{
public:
	void SetSingleMesh(Mesh mesh);

private:
	std::vector<Mesh> m_meshes;
};

struct BoxModelDesc
{
	float sizeX{ 1.0f };
	float sizeY{ 1.0f };
	float sizeZ{ 1.0f };
	DirectX::XMFLOAT3 colors[8];
	bool genColors{ false };
	bool genNormals{ false };
	bool facesIn{ false };
};

std::shared_ptr<Model> MakeBoxModel(Renderer* renderer, const BoxModelDesc& desc);
#endif

} // namespace Kodiak
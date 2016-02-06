// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include <ppltasks.h>
#include <unordered_set>

namespace Kodiak
{

// Forward declarations
class BaseIndexBufferData;
class BaseVertexBufferData;
class ConstantBuffer;
class GraphicsCommandList;
class IndexBuffer;
class Scene;
class VertexBuffer;

enum class PrimitiveTopology;


namespace RenderThread
{

struct StaticMeshPartData
{
	std::shared_ptr<VertexBuffer>	vertexBuffer;
	std::shared_ptr<IndexBuffer>	indexBuffer;
	PrimitiveTopology				topology;
	uint32_t						indexCount;
	uint32_t						startIndex;
	int32_t							baseVertexOffset;
};


struct StaticMeshData
{
	std::vector<StaticMeshPartData>		meshParts;
	DirectX::XMFLOAT4X4					matrix;

	std::shared_ptr<ConstantBuffer>		perObjectConstants;
};


struct StaticMeshPerObjectData
{
	DirectX::XMMATRIX	matrix;
};


struct StaticModelData
{
	std::vector<StaticMeshData>		meshes;
	std::vector<Scene*>				scenes;
	DirectX::XMFLOAT4X4				matrix;

	bool							isDirty{ true };

	concurrency::task<void>			prepareTask;

	void UpdateConstants(GraphicsCommandList& commandList);
};


} // namespace RenderThread


struct StaticMeshPart
{
	std::shared_ptr<BaseVertexBufferData>	vertexData;
	std::shared_ptr<BaseIndexBufferData>	indexData;

	PrimitiveTopology						topology;
	uint32_t								indexCount;
	uint32_t								startIndex;
	int32_t									baseVertexOffset;
};


class StaticMesh
{
	friend class Scene;
	friend class StaticModel;
public:
	StaticMesh();

	void AddMeshPart(StaticMeshPart part);
	size_t GetNumMeshParts() const { return m_meshParts.size(); }

	void SetMatrix(const DirectX::XMFLOAT4X4& matrix);

private:
	std::vector<StaticMeshPart>	m_meshParts;
	DirectX::XMFLOAT4X4			m_matrix;
};


class StaticModel : public std::enable_shared_from_this<StaticModel>
{
	friend class Renderer;

public:
	StaticModel();

	void AddMesh(StaticMesh mesh);
	size_t GetNumMeshes() const { return m_meshes.size(); }

	void SetMatrix(const DirectX::XMFLOAT4X4& matrix);
	const DirectX::XMFLOAT4X4& GetMatrix() const { return m_matrix; }

	// Should only be called by the Renderer, or async tasks created by the renderer
	void CreateRenderThreadData();
	std::shared_ptr<RenderThread::StaticModelData> GetRenderThreadData() { return m_renderThreadData; }

private:
	std::mutex				m_meshMutex;
	std::vector<StaticMesh>	m_meshes;
	DirectX::XMFLOAT4X4		m_matrix;

	std::shared_ptr<RenderThread::StaticModelData>	m_renderThreadData;
};


struct BoxMeshDesc
{
	float sizeX{ 1.0f };
	float sizeY{ 1.0f };
	float sizeZ{ 1.0f };
	DirectX::XMFLOAT3 colors[8];
	bool genColors{ false };
	bool genNormals{ false };
	bool facesIn{ false };
};

StaticMesh MakeBoxMesh(const BoxMeshDesc& desc);

} // namespace Kodiak
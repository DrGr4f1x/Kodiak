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
	bool								isDirty{ true };
};


struct StaticMeshPerObjectData
{
	DirectX::XMMATRIX	matrix;
};


struct StaticModelData
{
	std::vector<std::shared_ptr<StaticMeshData>>	meshes;
	DirectX::XMFLOAT4X4								matrix;

	bool											isDirty{ true };

	void UpdateConstants(GraphicsCommandList& commandList);
};


} // namespace RenderThread


struct StaticMeshPart
{
	friend class StaticMesh;

	std::shared_ptr<BaseVertexBufferData>	vertexData;
	std::shared_ptr<BaseIndexBufferData>	indexData;

	PrimitiveTopology						topology;
	uint32_t								indexCount;
	uint32_t								startIndex;
	int32_t									baseVertexOffset;
};


class StaticMesh : public std::enable_shared_from_this<StaticMesh>
{
	friend class Scene;
	friend class StaticModel;
public:
	StaticMesh();

	void AddMeshPart(StaticMeshPart part);
	size_t GetNumMeshParts() const { return m_meshParts.size(); }

	void SetMatrix(const DirectX::XMFLOAT4X4& matrix);
	const DirectX::XMFLOAT4X4& GetMatrix() const { return m_matrix; }

	std::shared_ptr<StaticMesh> Clone();

private:
	void CreateRenderThreadData();

private:
	std::vector<StaticMeshPart>	m_meshParts;
	DirectX::XMFLOAT4X4			m_matrix;

	std::shared_ptr<RenderThread::StaticMeshData>	m_renderThreadData;
};


class StaticModel : public std::enable_shared_from_this<StaticModel>
{
	friend class Scene;

public:
	StaticModel();

	void AddMesh(std::shared_ptr<StaticMesh> mesh);
	size_t GetNumMeshes() const { return m_meshes.size(); }

	void SetMatrix(const DirectX::XMFLOAT4X4& matrix);
	const DirectX::XMFLOAT4X4& GetMatrix() const { return m_matrix; }

private:
	void CreateRenderThreadData();

private:
	std::mutex									m_meshMutex;
	std::vector<std::shared_ptr<StaticMesh>>	m_meshes;
	DirectX::XMFLOAT4X4							m_matrix;

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

std::shared_ptr<StaticMesh> MakeBoxMesh(const BoxMeshDesc& desc);

} // namespace Kodiak
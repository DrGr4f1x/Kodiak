// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Model.h"

#include "IndexBuffer.h"
#include "Profile.h"
#include "Renderer.h"
#include "RenderEnums.h"
#include "VertexBuffer.h"
#include "VertexFormats.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


MeshPart::MeshPart(std::shared_ptr<VertexBuffer> vbuffer,
	std::shared_ptr<IndexBuffer> ibuffer,
	PrimitiveTopology topology,
	uint32_t indexCount,
	uint32_t startIndex,
	int32_t baseVertex)
	: m_vertexBuffer(vbuffer)
	, m_indexBuffer(ibuffer)
	, m_topology(topology)
	, m_indexCount(indexCount)
	, m_startIndex(startIndex)
	, m_baseVertexOffset(baseVertex)
{
	m_loadTask = (vbuffer->loadTask && ibuffer->loadTask);
}


void Mesh::SetMeshParts(vector<MeshPart>& meshParts)
{
	m_meshParts = meshParts;

	if (!meshParts.empty())
	{
		m_loadTask = meshParts[0].m_loadTask;
	}

	for (size_t i = 0; i < m_meshParts.size(); ++i)
	{
		m_meshParts[i].m_parent = this;
		if (i > 1)
		{
			m_loadTask = (m_loadTask && m_meshParts[i].m_loadTask);
		}
	}

	if (m_parent)
	{
		m_parent->loadTask = (m_parent->loadTask && m_loadTask);
	}
}


Model::Model()
{
	XMStoreFloat4x4(&m_matrix, XMMatrixIdentity());
}


void Model::SetSingleMesh(Mesh& mesh)
{
	m_meshes.clear();
	m_meshes.push_back(mesh);

	mesh.m_parent = this;
	m_isReady = false;
	loadTask = mesh.m_loadTask;
}


void Model::SetTransform(const XMFLOAT4X4& matrix)
{
	m_matrix = matrix;
	m_isDirty = true;
}


const XMFLOAT4X4& Model::GetTransform() const
{
	return m_matrix;
}


namespace Kodiak
{

shared_ptr<Model> MakeBoxModel(const BoxModelDesc& desc)
{
	auto model = make_shared<Model>();

	shared_ptr<VertexBuffer> vbuffer;
	shared_ptr<BaseVertexBufferData> vdata;

	if (desc.genNormals && desc.genColors)
	{
		vdata.reset(new VertexBufferData<VertexPositionNormalColor>(
		{
			// -X face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[1]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[0]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[7]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[6]) },
			// +X face
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[5]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[4]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[3]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[2]) },
			// -Y face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(desc.colors[1]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(desc.colors[7]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(desc.colors[3]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(desc.colors[5]) },
			// +Y face
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(desc.colors[2]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(desc.colors[4]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(desc.colors[0]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(desc.colors[6]) },
			// -Z face
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(desc.colors[3]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(desc.colors[2]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(desc.colors[1]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(desc.colors[0]) },
			// +Z face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(desc.colors[7]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(desc.colors[6]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(desc.colors[5]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(desc.colors[4]) }

		}
		));

		vdata->SetDebugName("Box vertex buffer with normals and colors");
		vbuffer = VertexBuffer::Create(vdata, Usage::Immutable);
	}
	else if (desc.genNormals)
	{
		vdata.reset(new VertexBufferData<VertexPositionNormal>(
		{
			// -X face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			// +X face
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			// -Y face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			// +Y face
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			// -Z face
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			// +Z face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) }

		}
		));

		vdata->SetDebugName("Box vertex buffer with normals");
		vbuffer = VertexBuffer::Create(vdata, Usage::Immutable);
	}
	else if (desc.genColors)
	{
		vdata.reset(new VertexBufferData<VertexPositionColor>(
		{
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(desc.colors[0]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(desc.colors[1]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(desc.colors[2]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(desc.colors[3]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(desc.colors[4]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(desc.colors[5]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(desc.colors[6]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(desc.colors[7]) }
		}
		));

		vdata->SetDebugName("Box vertex buffer with colors");
		vbuffer = VertexBuffer::Create(vdata, Usage::Immutable);
	}
	else
	{
		vdata.reset(new VertexBufferData<VertexPosition>(
		{
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ) }
		}
		));

		vdata->SetDebugName("Box vertex buffer");
		vbuffer = VertexBuffer::Create(vdata, Usage::Immutable);
	}

	// Create the mesh and mesh parts
	Mesh mesh;
	vector<MeshPart> meshParts;
	

	if (desc.genNormals)
	{
		meshParts.reserve(6);

		// -X face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 0, 1, 2, 3 }));

			idata->SetDebugName("-X face");

			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// +X face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 4, 5, 6, 7 }));

			idata->SetDebugName("+X face");
			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// -Y face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 8, 9, 10, 11 }));

			idata->SetDebugName("-Y face");
			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// +Y face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 12, 13, 14, 15 }));

			idata->SetDebugName("+Y face");
			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// -Z face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 16, 17, 18, 19 }));

			idata->SetDebugName("-Z face");
			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// +Z face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 20, 21, 22, 23 }));

			idata->SetDebugName("+Z face");
			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}
	}
	else
	{
		meshParts.reserve(3);

		// Body
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 0, 1, 2, 3, 4, 5, 6, 7, 0, 1 }));

			idata->SetDebugName("Body");
			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 10, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// Top
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 6, 0, 4, 2 }));

			idata->SetDebugName("Top");
			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// Bottom
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 5, 3, 7, 1 }));

			idata->SetDebugName("Bottom");
			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}
	}

	mesh.SetMeshParts(meshParts);
	model->SetSingleMesh(mesh);

	return model;
}



StaticMesh::StaticMesh()
{
	XMStoreFloat4x4(&m_matrix, XMMatrixIdentity());
}


void StaticMesh::AddMeshPart(StaticMeshPart part)
{
	// TODO: Move to render thread
	m_meshParts.emplace_back(part);
}


StaticModel::StaticModel()
{
	XMStoreFloat4x4(&m_matrix, XMMatrixIdentity());
}


void StaticModel::AddMesh(StaticMesh mesh)
{
	m_meshes.emplace_back(mesh);
}


void StaticModel::CreateRenderThreadData()
{
	PROFILE(staticModel_CreateRenderThreadData);

	const auto numMeshes = m_meshes.size();

	m_renderThreadData = make_shared<RenderThread::StaticModelData>();
	m_renderThreadData->matrix = m_matrix;
	m_renderThreadData->meshes.reserve(numMeshes);

	std::vector<concurrency::task<void>> tasks;

	// Determine the maximum number of tasks we need & pre-allocate memory
	uint32_t maxTasks = 0;
	for (size_t i = 0; i < numMeshes; ++i)
	{
		maxTasks += 2 * static_cast<uint32_t>(m_meshes[i].GetNumMeshParts());
	}

	tasks.reserve(maxTasks);

	std::vector<shared_ptr<VertexBuffer>> uniqueVBuffers(maxTasks / 2);
	std::vector<shared_ptr<IndexBuffer>> uniqueIBuffers(maxTasks / 2);

	for (const auto& mesh : m_meshes)
	{
		RenderThread::StaticMeshData meshData;
		meshData.matrix = mesh.m_matrix;

		for (const auto& part : mesh.m_meshParts)
		{
			RenderThread::StaticMeshPartData meshPartData;

			// Create index buffer
			meshPartData.indexBuffer = IndexBuffer::Create(part.indexData, Usage::Immutable);

			// Track unique index buffers for this model
			if (end(uniqueIBuffers) == find(begin(uniqueIBuffers), end(uniqueIBuffers), meshPartData.indexBuffer))
			{
				tasks.emplace_back(meshPartData.indexBuffer->loadTask);
				uniqueIBuffers.emplace_back(meshPartData.indexBuffer);
			}

			// Create vertex buffer
			meshPartData.vertexBuffer = VertexBuffer::Create(part.vertexData, Usage::Immutable);

			// Track unique vertex buffers for this model
			if (end(uniqueVBuffers) == find(begin(uniqueVBuffers), end(uniqueVBuffers), meshPartData.vertexBuffer))
			{
				tasks.emplace_back(meshPartData.vertexBuffer->loadTask);
				uniqueVBuffers.emplace_back(meshPartData.vertexBuffer);
			}

			// Fill in misc data for the mesh part draw-call
			meshPartData.topology = part.topology;
			meshPartData.indexCount = part.indexCount;
			meshPartData.baseVertexOffset = part.baseVertexOffset;
			meshPartData.startIndex = part.startIndex;

			meshData.meshParts.emplace_back(meshPartData);
		}

		m_renderThreadData->meshes.emplace_back(meshData);
	}

	m_renderThreadData->prepareTask = concurrency::when_all(begin(tasks), end(tasks));
}


StaticMesh MakeBoxMesh(const BoxMeshDesc& desc)
{
	shared_ptr<BaseVertexBufferData> vdata;

	if (desc.genNormals && desc.genColors)
	{
		vdata.reset(new VertexBufferData<VertexPositionNormalColor>(
		{
			// -X face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[1]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[0]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[7]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[6]) },
			// +X face
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[5]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[4]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[3]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[2]) },
			// -Y face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(desc.colors[1]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(desc.colors[7]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(desc.colors[3]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(desc.colors[5]) },
			// +Y face
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(desc.colors[2]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(desc.colors[4]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(desc.colors[0]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(desc.colors[6]) },
			// -Z face
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(desc.colors[3]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(desc.colors[2]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(desc.colors[1]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(desc.colors[0]) },
			// +Z face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(desc.colors[7]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(desc.colors[6]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(desc.colors[5]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(desc.colors[4]) }

		}
		));
	}
	else if (desc.genNormals)
	{
		vdata.reset(new VertexBufferData<VertexPositionNormal>(
		{
			// -X face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			// +X face
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			// -Y face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			// +Y face
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			// -Z face
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			// +Z face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) }

		}
		));
	}
	else if (desc.genColors)
	{
		vdata.reset(new VertexBufferData<VertexPositionColor>(
		{
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(desc.colors[0]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(desc.colors[1]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(desc.colors[2]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(desc.colors[3]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(desc.colors[4]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(desc.colors[5]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(desc.colors[6]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(desc.colors[7]) }
		}
		));
	}
	else
	{
		vdata.reset(new VertexBufferData<VertexPosition>(
		{
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ) }
		}
		));
	}

	// Create the mesh and mesh parts
	StaticMesh mesh;
	
	if (desc.genNormals)
	{
		// -X face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 0, 1, 2, 3 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh.AddMeshPart(meshPart);
		}

		// +X face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 4, 5, 6, 7 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh.AddMeshPart(meshPart);
		}

		// -Y face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 8, 9, 10, 11 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh.AddMeshPart(meshPart);
		}

		// +Y face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 12, 13, 14, 15 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh.AddMeshPart(meshPart);
		}

		// -Z face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 16, 17, 18, 19 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh.AddMeshPart(meshPart);
		}

		// +Z face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 20, 21, 22, 23 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh.AddMeshPart(meshPart);
		}
	}
	else
	{
		// Body
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 0, 1, 2, 3, 4, 5, 6, 7, 0, 1 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 10, 0, 0 };
			mesh.AddMeshPart(meshPart);
		}

		// Top
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 6, 0, 4, 2 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh.AddMeshPart(meshPart);
		}

		// Bottom
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 5, 3, 7, 1 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh.AddMeshPart(meshPart);
		}
	}

	return mesh;
}


} // namespace Kodiak
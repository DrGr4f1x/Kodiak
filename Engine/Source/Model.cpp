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

#include "CommandList.h"
#include "ConstantBuffer.h"
#include "IndexBuffer.h"
#include "Profile.h"
#include "Renderer.h"
#include "RenderEnums.h"
#include "VertexBuffer.h"
#include "VertexFormats.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


void RenderThread::StaticModelData::UpdateConstants(GraphicsCommandList& commandList)
{
	const XMMATRIX modelToWorld = XMLoadFloat4x4(&matrix);
	
	for (auto& mesh : meshes)
	{
		if (isDirty || mesh->isDirty)
		{
			StaticMeshPerObjectData perObjectData;
			perObjectData.matrix = XMMatrixMultiply(modelToWorld, XMLoadFloat4x4(&mesh->matrix));

			auto dest = commandList.MapConstants(*mesh->perObjectConstants);
			memcpy(dest, &perObjectData, sizeof(perObjectData));
			commandList.UnmapConstants(*mesh->perObjectConstants);

			mesh->isDirty = false;
		}
	}

	isDirty = false;
}


StaticMesh::StaticMesh()
{
	XMStoreFloat4x4(&m_matrix, XMMatrixIdentity());
	CreateRenderThreadData();
}


void StaticMesh::AddMeshPart(StaticMeshPart part)
{
	// TODO: Move to render thread
	m_meshParts.emplace_back(part);

	auto indexBuffer = IndexBuffer::Create(part.indexData, Usage::Immutable);
	auto vertexBuffer = VertexBuffer::Create(part.vertexData, Usage::Immutable);

	auto thisMesh = shared_from_this();
	(indexBuffer->loadTask && vertexBuffer->loadTask).then([indexBuffer, vertexBuffer, thisMesh, part]
	{
		Renderer::GetInstance().EnqueueTask([indexBuffer, vertexBuffer, thisMesh, part](RenderTaskEnvironment& rte)
		{
			RenderThread::StaticMeshPartData data = { vertexBuffer, indexBuffer, part.topology, part.indexCount, part.startIndex, part.baseVertexOffset };
			thisMesh->m_renderThreadData->meshParts.emplace_back(data);
		});
	});
}


void StaticMesh::SetMatrix(const XMFLOAT4X4& matrix)
{
	// TODO: Move to render thread
	m_matrix = matrix;

	auto staticMeshData = m_renderThreadData;
	Renderer::GetInstance().EnqueueTask([staticMeshData, matrix](RenderTaskEnvironment& rte)
	{
		staticMeshData->matrix = matrix;
		staticMeshData->isDirty = true;
	});
}


shared_ptr<StaticMesh> StaticMesh::Clone()
{
	auto clone = make_shared<StaticMesh>();
	clone->SetMatrix(m_matrix);

	for (const auto& part : m_meshParts)
	{
		clone->AddMeshPart(part);
	}

	return clone;
}


void StaticMesh::CreateRenderThreadData()
{
	m_renderThreadData = make_shared<RenderThread::StaticMeshData>();
	m_renderThreadData->matrix = m_matrix;
	
	m_renderThreadData->perObjectConstants = make_shared<ConstantBuffer>();
	m_renderThreadData->perObjectConstants->Create(sizeof(RenderThread::StaticMeshPerObjectData), Usage::Dynamic);
}


StaticModel::StaticModel()
{
	XMStoreFloat4x4(&m_matrix, XMMatrixIdentity());
	CreateRenderThreadData();
}


void StaticModel::AddMesh(shared_ptr<StaticMesh> mesh)
{
	// TODO: Move to render thread
	m_meshes.emplace_back(mesh);

	auto staticModelData = m_renderThreadData;
	auto staticMeshData = mesh->m_renderThreadData;

	Renderer::GetInstance().EnqueueTask([staticModelData, staticMeshData](RenderTaskEnvironment& rte)
	{
		staticModelData->meshes.emplace_back(staticMeshData);
	});
}


void StaticModel::SetMatrix(const XMFLOAT4X4& matrix)
{
	m_matrix = matrix;
	auto thisModel = shared_from_this();
	Renderer::GetInstance().EnqueueTask([thisModel](RenderTaskEnvironment& rte)
	{
		thisModel->m_renderThreadData->matrix = thisModel->GetMatrix();
		thisModel->m_renderThreadData->isDirty = true;
	});
}


void StaticModel::CreateRenderThreadData()
{
	PROFILE(staticModel_CreateRenderThreadData);

	const auto numMeshes = m_meshes.size();

	m_renderThreadData = make_shared<RenderThread::StaticModelData>();
	m_renderThreadData->matrix = m_matrix;
	m_renderThreadData->meshes.reserve(numMeshes);
}


namespace Kodiak
{

shared_ptr<StaticMesh> MakeBoxMesh(const BoxMeshDesc& desc)
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
	auto mesh = make_shared<StaticMesh>();

	if (desc.genNormals)
	{
		// -X face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 0, 1, 2, 3 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// +X face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 4, 5, 6, 7 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// -Y face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 8, 9, 10, 11 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// +Y face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 12, 13, 14, 15 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// -Z face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 16, 17, 18, 19 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// +Z face
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 20, 21, 22, 23 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}
	}
	else
	{
		// Body
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 0, 1, 2, 3, 4, 5, 6, 7, 0, 1 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 10, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// Top
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 6, 0, 4, 2 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// Bottom
		{
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 5, 3, 7, 1 }));

			StaticMeshPart meshPart{ vdata, idata, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}
	}

	return mesh;
}

} // namespace Kodiak
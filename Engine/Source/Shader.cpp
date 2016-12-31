// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Shader.h"

#include "ResourceLoader.h"
#include "ShaderResource.h"


using namespace Kodiak;
using namespace std;


uint32_t VertexShader::GetPerViewDataSize() const
{
	assert(m_resource);
	return m_resource->GetPerViewDataSize();
}


uint32_t VertexShader::GetPerObjectDataSize() const
{
	assert(m_resource);
	return m_resource->GetPerObjectDataSize();
}


const ShaderReflection::Signature& VertexShader::GetSignature() const
{
	assert(m_resource);
	return m_resource->GetSignature();
}


shared_ptr<VertexShader> VertexShader::Load(const string& path)
{
	auto shader = make_shared<VertexShader>();

	shader->m_resource = ResourceLoader::GetInstance().Load<VertexShaderResource>(path);

	return shader;
}


shared_ptr<VertexShader> VertexShader::LoadImmediate(const string& path)
{
	auto shader = make_shared<VertexShader>();

	shader->m_resource = ResourceLoader::GetInstance().LoadImmediate<VertexShaderResource>(path);

	return shader;
}


void VertexShader::AddPostLoadCallback(function<void()> callback)
{
	assert(m_resource);

	m_resource->AddPostLoadCallback(callback);
}


bool VertexShader::IsReady() const
{
	return m_resource ? m_resource->IsReady() : false;
}


void VertexShader::Wait()
{
	if (m_resource)
	{
		m_resource->Wait();
	}
}


uint32_t HullShader::GetPerViewDataSize() const
{
	assert(m_resource);
	return m_resource->GetPerViewDataSize();
}


uint32_t HullShader::GetPerObjectDataSize() const
{
	assert(m_resource);
	return m_resource->GetPerObjectDataSize();
}


const ShaderReflection::Signature& HullShader::GetSignature() const
{
	assert(m_resource);
	return m_resource->GetSignature();
}


shared_ptr<HullShader> HullShader::Load(const string& path)
{
	auto shader = make_shared<HullShader>();

	shader->m_resource = ResourceLoader::GetInstance().Load<HullShaderResource>(path);

	return shader;
}


shared_ptr<HullShader> HullShader::LoadImmediate(const string& path)
{
	auto shader = make_shared<HullShader>();

	shader->m_resource = ResourceLoader::GetInstance().LoadImmediate<HullShaderResource>(path);

	return shader;
}


void HullShader::AddPostLoadCallback(function<void()> callback)
{
	assert(m_resource);

	m_resource->AddPostLoadCallback(callback);
}


bool HullShader::IsReady() const
{
	return m_resource ? m_resource->IsReady() : false;
}


void HullShader::Wait()
{
	if (m_resource)
	{
		m_resource->Wait();
	}
}


uint32_t DomainShader::GetPerViewDataSize() const
{
	assert(m_resource);
	return m_resource->GetPerViewDataSize();
}


uint32_t DomainShader::GetPerObjectDataSize() const
{
	assert(m_resource);
	return m_resource->GetPerObjectDataSize();
}


const ShaderReflection::Signature& DomainShader::GetSignature() const
{
	assert(m_resource);
	return m_resource->GetSignature();
}


shared_ptr<DomainShader> DomainShader::Load(const string& path)
{
	auto shader = make_shared<DomainShader>();

	shader->m_resource = ResourceLoader::GetInstance().Load<DomainShaderResource>(path);

	return shader;
}


shared_ptr<DomainShader> DomainShader::LoadImmediate(const string& path)
{
	auto shader = make_shared<DomainShader>();

	shader->m_resource = ResourceLoader::GetInstance().LoadImmediate<DomainShaderResource>(path);

	return shader;
}


void DomainShader::AddPostLoadCallback(function<void()> callback)
{
	assert(m_resource);

	m_resource->AddPostLoadCallback(callback);
}


bool DomainShader::IsReady() const
{
	return m_resource ? m_resource->IsReady() : false;
}


void DomainShader::Wait()
{
	if (m_resource)
	{
		m_resource->Wait();
	}
}


uint32_t GeometryShader::GetPerViewDataSize() const
{
	assert(m_resource);
	return m_resource->GetPerViewDataSize();
}


uint32_t GeometryShader::GetPerObjectDataSize() const
{
	assert(m_resource);
	return m_resource->GetPerObjectDataSize();
}


const ShaderReflection::Signature& GeometryShader::GetSignature() const
{
	assert(m_resource);
	return m_resource->GetSignature();
}


shared_ptr<GeometryShader> GeometryShader::Load(const string& path)
{
	auto shader = make_shared<GeometryShader>();

	shader->m_resource = ResourceLoader::GetInstance().Load<GeometryShaderResource>(path);

	return shader;
}


shared_ptr<GeometryShader> GeometryShader::LoadImmediate(const string& path)
{
	auto shader = make_shared<GeometryShader>();

	shader->m_resource = ResourceLoader::GetInstance().LoadImmediate<GeometryShaderResource>(path);

	return shader;
}


void GeometryShader::AddPostLoadCallback(function<void()> callback)
{
	assert(m_resource);

	m_resource->AddPostLoadCallback(callback);
}


bool GeometryShader::IsReady() const
{
	return m_resource ? m_resource->IsReady() : false;
}


void GeometryShader::Wait()
{
	if (m_resource)
	{
		m_resource->Wait();
	}
}


uint32_t PixelShader::GetPerViewDataSize() const
{
	assert(m_resource);
	return m_resource->GetPerViewDataSize();
}


uint32_t PixelShader::GetPerObjectDataSize() const
{
	assert(m_resource);
	return m_resource->GetPerObjectDataSize();
}


const ShaderReflection::Signature& PixelShader::GetSignature() const
{
	assert(m_resource);
	return m_resource->GetSignature();
}


shared_ptr<PixelShader> PixelShader::Load(const string& path)
{
	auto shader = make_shared<PixelShader>();

	shader->m_resource = ResourceLoader::GetInstance().Load<PixelShaderResource>(path);

	return shader;
}


shared_ptr<PixelShader> PixelShader::LoadImmediate(const string& path)
{
	auto shader = make_shared<PixelShader>();

	shader->m_resource = ResourceLoader::GetInstance().LoadImmediate<PixelShaderResource>(path);

	return shader;
}


void PixelShader::AddPostLoadCallback(function<void()> callback)
{
	assert(m_resource);

	m_resource->AddPostLoadCallback(callback);
}


bool PixelShader::IsReady() const
{
	return m_resource ? m_resource->IsReady() : false;
}


void PixelShader::Wait()
{
	if (m_resource)
	{
		m_resource->Wait();
	}
}


uint32_t ComputeShader::GetPerViewDataSize() const
{
	assert(m_resource);
	return m_resource->GetPerViewDataSize();
}


uint32_t ComputeShader::GetPerObjectDataSize() const
{
	assert(m_resource);
	return m_resource->GetPerObjectDataSize();
}


const ShaderReflection::Signature& ComputeShader::GetSignature() const
{
	assert(m_resource);
	return m_resource->GetSignature();
}


shared_ptr<ComputeShader> ComputeShader::Load(const string& path)
{
	auto shader = make_shared<ComputeShader>();

	shader->m_resource = ResourceLoader::GetInstance().Load<ComputeShaderResource>(path);

	return shader;
}


shared_ptr<ComputeShader> ComputeShader::LoadImmediate(const string& path)
{
	auto shader = make_shared<ComputeShader>();

	shader->m_resource = ResourceLoader::GetInstance().LoadImmediate<ComputeShaderResource>(path);

	return shader;
}


void ComputeShader::AddPostLoadCallback(function<void()> callback)
{
	assert(m_resource);

	m_resource->AddPostLoadCallback(callback);
}


bool ComputeShader::IsReady() const
{
	return m_resource ? m_resource->IsReady() : false;
}


void ComputeShader::Wait()
{
	if (m_resource)
	{
		m_resource->Wait();
	}
}
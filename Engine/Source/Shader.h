// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "RenderEnums.h"
#include "ShaderReflection.h"

namespace Kodiak
{

// Forward declarations
class VertexShaderResource;
class HullShaderResource;
class DomainShaderResource;
class GeometryShaderResource;
class PixelShaderResource;
class ComputeShaderResource;


class IShader
{
public:
	virtual uint32_t GetPerViewDataSize() const = 0;
	virtual uint32_t GetPerObjectDataSize() const = 0;
	virtual const struct ShaderReflection::Signature& GetSignature() const = 0;
	virtual ShaderType GetType() const = 0;
	virtual bool IsReady() const = 0;
	virtual void Wait() = 0;
};


class VertexShader : public IShader
{
public:
	// Reflection info
	uint32_t GetPerViewDataSize() const final override;
	uint32_t GetPerObjectDataSize() const final override;
	const struct ShaderReflection::Signature& GetSignature() const final override;
	ShaderType GetType() const final override { return ShaderType::Vertex; }
	
	static std::shared_ptr<VertexShader> Load(const std::string& path);
	static std::shared_ptr<VertexShader> LoadImmediate(const std::string& path);
	void AddPostLoadCallback(std::function<void()> callback);
	bool IsReady() const final override;
	void Wait() final override;

	VertexShaderResource* GetResource() { return m_resource.get(); }

private:
	std::shared_ptr<VertexShaderResource> m_resource;
};


class HullShader : public IShader
{
public:
	// Reflection info
	uint32_t GetPerViewDataSize() const final override;
	uint32_t GetPerObjectDataSize() const final override;
	const struct ShaderReflection::Signature& GetSignature() const final override;
	ShaderType GetType() const final override { return ShaderType::Hull; }
	
	static std::shared_ptr<HullShader> Load(const std::string& path);
	static std::shared_ptr<HullShader> LoadImmediate(const std::string& path);
	void AddPostLoadCallback(std::function<void()> callback);
	bool IsReady() const final override;
	void Wait() final override;

	HullShaderResource* GetResource() { return m_resource.get(); }

private:
	std::shared_ptr<HullShaderResource> m_resource;
};


class DomainShader : public IShader
{
public:
	// Reflection info
	uint32_t GetPerViewDataSize() const final override;
	uint32_t GetPerObjectDataSize() const final override;
	const struct ShaderReflection::Signature& GetSignature() const final override;
	ShaderType GetType() const final override { return ShaderType::Domain; }
	
	static std::shared_ptr<DomainShader> Load(const std::string& path);
	static std::shared_ptr<DomainShader> LoadImmediate(const std::string& path);
	void AddPostLoadCallback(std::function<void()> callback);
	bool IsReady() const final override;
	void Wait() final override;

	DomainShaderResource* GetResource() { return m_resource.get(); }

private:
	std::shared_ptr<DomainShaderResource> m_resource;
};


class GeometryShader : public IShader
{
public:
	// Reflection info
	uint32_t GetPerViewDataSize() const final override;
	uint32_t GetPerObjectDataSize() const final override;
	const struct ShaderReflection::Signature& GetSignature() const final override;
	ShaderType GetType() const final override { return ShaderType::Geometry; }
	
	static std::shared_ptr<GeometryShader> Load(const std::string& path);
	static std::shared_ptr<GeometryShader> LoadImmediate(const std::string& path);
	void AddPostLoadCallback(std::function<void()> callback);
	bool IsReady() const final override;
	void Wait() final override;

	GeometryShaderResource* GetResource() { return m_resource.get(); }

private:
	std::shared_ptr<GeometryShaderResource> m_resource;
};


class PixelShader : public IShader
{
public:
	// Reflection info
	uint32_t GetPerViewDataSize() const final override;
	uint32_t GetPerObjectDataSize() const final override;
	const struct ShaderReflection::Signature& GetSignature() const final override;
	ShaderType GetType() const final override { return ShaderType::Pixel; }
	
	static std::shared_ptr<PixelShader> Load(const std::string& path);
	static std::shared_ptr<PixelShader> LoadImmediate(const std::string& path);
	void AddPostLoadCallback(std::function<void()> callback);
	bool IsReady() const final override;
	void Wait() final override;

	PixelShaderResource* GetResource() { return m_resource.get(); }

private:
	std::shared_ptr<PixelShaderResource> m_resource;
};


class ComputeShader : public IShader
{
public:
	// Reflection info
	uint32_t GetPerViewDataSize() const final override;
	uint32_t GetPerObjectDataSize() const final override;
	const struct ShaderReflection::Signature& GetSignature() const final override;
	ShaderType GetType() const final override { return ShaderType::Compute; }

	static std::shared_ptr<ComputeShader> Load(const std::string& path);
	static std::shared_ptr<ComputeShader> LoadImmediate(const std::string& path);
	void AddPostLoadCallback(std::function<void()> callback);
	bool IsReady() const final override;
	void Wait() final override;

	ComputeShaderResource* GetResource() { return m_resource.get(); }

private:
	std::shared_ptr<ComputeShaderResource> m_resource;
};

} // namespace Kodiak
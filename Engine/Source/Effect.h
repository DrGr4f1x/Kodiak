#pragma once

namespace Kodiak
{

class ENGINE_API Effect
{
public:
    void SetVertexShaderPath(const std::wstring& path);
    void SetPixelShaderPath(const std::wstring& path);
};

} // namespace Kodiak
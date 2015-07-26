#pragma once

namespace Kodiak
{

class ENGINE_API Effect
{
public:
    void SetVertexShaderPath(const std::string& path);
    void SetPixelShaderPath(const std::string& path);
};

} // namespace Kodiak
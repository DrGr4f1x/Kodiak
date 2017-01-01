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

class Paths
{
public:
	static Paths& GetInstance();

	const std::string& BaseDir() const;
	const std::string& BinaryDir() const;
	const std::string& ModelDir() const;
	const std::string& TextureDir() const;
	const std::string& ShaderDir() const;
	const std::string& LogDir() const;

private:
	Paths();
	void Initialize();

private:
	std::string m_baseDir;
	std::string m_binaryDir;
	std::string m_modelDir;
	std::string m_textureDir;
	std::string m_shaderDir;
	std::string m_logDir;
};

} // namespace Kodiak
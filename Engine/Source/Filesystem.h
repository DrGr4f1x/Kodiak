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

class Filesystem
{
public:
	static Filesystem& GetInstance();

	const std::string& GetBinaryDir() const { return m_binaryDir; }
	const std::string& GetRootDir() const { return m_rootDir; }

	void SetBinaryPath(const std::string& path, bool patchRootDir = true);
	void Mount(const std::string& path, bool appendPath = false);

private:
	Filesystem();
	void Initialize();

private:
	std::string m_binaryDir;
	std::string m_binarySubpath;

	std::string m_rootDir;

	struct PathDesc
	{
		std::string mountRoot;
		std::string mountPath;
		std::string fullPath;
		PathDesc* next;
	};
	PathDesc* m_searchPaths{ nullptr };
};

} // namespace Kodiak
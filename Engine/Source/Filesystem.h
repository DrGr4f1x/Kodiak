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


enum class EFileType
{
	Regular,
	Directory,
	SymLink,
	Other
};


struct FileStat
{
	__int64 filesize;
	__int64 modtime;
	__int64 createtime;
	__int64 accesstime;
	EFileType filetype;
	int readonly;
};


class Filesystem
{
public:
	static Filesystem& GetInstance();

	const std::string& GetBinaryDir() const { return m_binaryDir; }
	const std::string& GetRootDir() const { return m_rootDir; }

	void SetRootDir(const std::string& rootDir);

	void AddSearchPath(const std::string& fname, bool appendPath = false);
	void RemoveSearchPath(const std::string& path);
	void RemoveAllSearchPaths();

	std::vector<std::string> GetSearchPaths();

	bool Exists(const std::string& fname);
	bool IsRegularFile(const std::string& fname);
	bool IsDirectory(const std::string& fname);
	bool GetFileStat(const std::string& fname, FileStat& stat);
	std::string GetFullPath(const std::string& path);

private:
	Filesystem();
	void Initialize();
	bool InternalGetFileStat(const std::string& fname, FileStat& stat);

private:
	std::string m_binaryDir;
	std::string m_binarySubpath;

	std::string m_rootDir;

	struct PathDesc
	{
		std::string localPath;
		std::string fullPath;
		PathDesc* next;
	};
	PathDesc* m_searchPaths{ nullptr };

	std::shared_mutex m_mutex;
};


} // namespace Kodiak
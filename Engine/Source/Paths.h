#pragma once


namespace Kodiak
{

class Paths
{
public:
	static Paths& GetInstance();

	const std::string& BaseDir() const;
	const std::string& BinaryDir() const;
	const std::string& AssetDir() const;
	const std::string& LogDir() const;

private:
	Paths();
	void Initialize();

private:
	std::string m_baseDir;
	std::string m_binaryDir;
	std::string m_assetDir;
	std::string m_logDir;
};

} // namespace Kodiak
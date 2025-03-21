#pragma once


class ZipAlgorithm_andyzip : public ZipAlgorithm
{
public:
	ZipAlgorithm_andyzip() {}

	bool				Open(const char* pchZipFile, const char* pchOutputDir);
	void				Close();

	size_t				GetEntryCount() const;
	const std::string&	GetEntryName(const size_t szEntryIndex) const;
	bool				UnzipEntry(const size_t szEntryIndex);

private:
	class Reader;

	std::vector<char>	m_vBuffer;
	std::string			m_strFile;
	Reader*				m_pReader = nullptr;
};

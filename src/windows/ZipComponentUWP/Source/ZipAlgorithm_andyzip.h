#pragma once


class zipfile_reader;

class ZipAlgorithm_andyzip : public ZipAlgorithm
{
public:
	ZipAlgorithm_andyzip() {}

	bool				Open(const char* pchZipFile, const char* pchOutputDir);
	void				Close();

	size_t				GetEntryCount() const;
	const std::string&	GetEntryName(const size_t szEntryIndex) const;
	bool				UnzipEntry(const size_t szEntryIndex) const;

private:
	std::vector<std::string>	m_vEntries;
	std::vector<char>			m_vBuffer;
	zipfile_reader*				m_pReader = nullptr;
};

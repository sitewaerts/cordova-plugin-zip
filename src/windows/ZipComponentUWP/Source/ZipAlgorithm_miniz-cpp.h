#pragma once


namespace miniz_cpp { class zip_file;  }

class ZipAlgorithm_miniz_cpp : public ZipAlgorithm
{
public:
	ZipAlgorithm_miniz_cpp() {}

	bool				Open(const char* pchZipFile, const char* pchOutputDir);
	void				Close();

	size_t				GetEntryCount() const;
	const std::string&	GetEntryName(const size_t szEntryIndex) const;
	bool				UnzipEntry(const size_t szEntryIndex) const;

private:
	std::vector<std::string>	m_vEntries;
	miniz_cpp::zip_file*		m_pZipFile = nullptr;
};

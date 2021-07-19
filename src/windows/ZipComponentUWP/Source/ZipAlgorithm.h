#pragma once


class ZipAlgorithm
{
public:
	static const char*			NameAndyZip;	// "andyzip"
	static const char*			NameMinizCpp;	// "miniz-cpp"

	static ZipAlgorithm*		Create(const char* pchAlgorithmName);
	static void					Destroy(ZipAlgorithm*& pAlgorithm);

	virtual bool				Open(const char* pchZipFile, const char* pchOutputDir) = 0;
	virtual void				Close() = 0;

	virtual size_t				GetEntryCount() const = 0;
	virtual const std::string&	GetEntryName(const size_t szEntryIndex) const = 0;
	virtual bool				UnzipEntry(const size_t szEntryIndex) const = 0;

protected:
	ZipAlgorithm() = default;

	void						SetOutputDirWithTrailingSlash(const char* pchOutputDir);
	bool						CreateEntryDir(const std::string& strEntryDir) const;
	void						CreateEntrySubDirs(const std::string& strEntryName) const;

	std::string					m_strOutputDir;
};

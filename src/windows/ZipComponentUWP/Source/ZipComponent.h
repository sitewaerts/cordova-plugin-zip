#pragma once


namespace ZipComponentUWP
{
	using namespace Platform;

    public ref class ZipComponent sealed
    {
    public:
		ZipComponent(String^ strAlgorithmName, String^ strZipFile, String^ strOutputDir);
		virtual ~ZipComponent();

		size_t			GetEntryCount();
		String^			GetEntryName(size_t szEntryIndex);
		bool			UnzipEntry(size_t szEntryIndex);

		String^			GetLastError();

	private:
		ZipAlgorithm*	m_pAlgorithm;
		std::string		m_strZipFileUtf8;
		std::string		m_strOutputDirUtf8;
		String^			m_strLastError;		
    };
}

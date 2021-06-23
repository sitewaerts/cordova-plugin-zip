#include "pch.h"
#include <cvt/wstring>
#include <codecvt>
#include "ZipAlgorithm.h"
#include "ZipComponent.h"


using namespace ZipComponentUWP;


static inline String^ StdStringToPlatformString(const std::string& stdString)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	const std::wstring stdwString(converter.from_bytes(stdString));
	return ref new String(stdwString.c_str());
}

static inline std::string PlatformStringToStdString(String^ platformString)
{
	stdext::cvt::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
	return convert.to_bytes(platformString->Data());
}


ZipComponent::ZipComponent(String^ strAlgorithmName, String^ strZipFile, String^ strOutputDir)
: m_pAlgorithm(nullptr)
{
	const std::string stringAlgorithmNameUtf8 = PlatformStringToStdString(strAlgorithmName);
	m_pAlgorithm = ZipAlgorithm::Create(stringAlgorithmNameUtf8.c_str());
	if( m_pAlgorithm == nullptr )
	{
		m_strLastError = "Failed to create algorithm: " + StdStringToPlatformString(stringAlgorithmNameUtf8);
		return;
	}

	m_strZipFileUtf8 = PlatformStringToStdString(strZipFile);
	m_strOutputDirUtf8 = PlatformStringToStdString(strOutputDir);

	if( !m_pAlgorithm->Open(m_strZipFileUtf8.c_str(), m_strOutputDirUtf8.c_str()) )
	{
		m_strLastError  = "Failed to open zip file " + StdStringToPlatformString(m_strZipFileUtf8)
						+ " (output dir = " + StdStringToPlatformString(m_strOutputDirUtf8) + ")";

		m_pAlgorithm->Close();
		ZipAlgorithm::Destroy(m_pAlgorithm);
	}
}

ZipComponent::~ZipComponent()
{
	if( m_pAlgorithm )
	{
		m_pAlgorithm->Close();
		ZipAlgorithm::Destroy(m_pAlgorithm);
	}
}

size_t ZipComponent::GetEntryCount()
{
	if( m_pAlgorithm == nullptr )
	{
		m_strLastError = "No algorithm initialized!";
		return 0u;
	}

	return m_pAlgorithm->GetEntryCount();
}

String^ ZipComponent::GetEntryName(const size_t szEntryIndex)
{
	if( m_pAlgorithm == nullptr )
	{
		m_strLastError = "No algorithm initialized!";
		return "";
	}

	return StdStringToPlatformString(m_pAlgorithm->GetEntryName(szEntryIndex));
}

bool ZipComponent::UnzipEntry(const size_t szEntryIndex)
{
	if( m_pAlgorithm == nullptr )
	{
		m_strLastError = "No algorithm initialized!";
		return false;
	}

	return m_pAlgorithm->UnzipEntry(szEntryIndex);
}

String^ ZipComponent::GetLastError()
{
	return m_strLastError;
}

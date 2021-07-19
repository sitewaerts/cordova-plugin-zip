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
	try
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
			m_strLastError	= "Failed to open zip file " + StdStringToPlatformString(m_strZipFileUtf8)
							+ " (output dir = " + StdStringToPlatformString(m_strOutputDirUtf8) + ")";

			m_pAlgorithm->Close();
			ZipAlgorithm::Destroy(m_pAlgorithm);
		}
	}
	catch( std::exception exception )
	{
		if( exception.what() )
			m_strLastError += StdStringToPlatformString(std::string("\n\n") + exception.what());
	}
}

ZipComponent::~ZipComponent()
{
	if( m_pAlgorithm )
	{
		try
		{
			m_pAlgorithm->Close();
			ZipAlgorithm::Destroy(m_pAlgorithm);
		}
		catch( std::exception exception )
		{
			// Can't do much here, since we're destructing the object
		}
	}
}

size_t ZipComponent::GetEntryCount()
{
	if( m_pAlgorithm == nullptr )
	{
		m_strLastError = "No algorithm initialized!";
		return 0u;
	}

	try
	{
		return m_pAlgorithm->GetEntryCount();
	}
	catch( std::exception exception )
	{
		if( exception.what() )
			m_strLastError += StdStringToPlatformString(std::string("\n\n") + exception.what());
	}

	return 0u;
}

String^ ZipComponent::GetEntryName(const size_t szEntryIndex)
{
	if( m_pAlgorithm == nullptr )
	{
		m_strLastError = "No algorithm initialized!";
		return "";
	}

	try
	{
		return StdStringToPlatformString(m_pAlgorithm->GetEntryName(szEntryIndex));
	}
	catch( std::exception exception )
	{
		if( exception.what() )
			m_strLastError += StdStringToPlatformString(std::string("\n\n") + exception.what());
	}

	return "";
}

bool ZipComponent::UnzipEntry(const size_t szEntryIndex)
{
	if( m_pAlgorithm == nullptr )
	{
		m_strLastError = "No algorithm initialized!";
		return false;
	}

	try
	{
		return m_pAlgorithm->UnzipEntry(szEntryIndex);
	}
	catch( std::exception exception )
	{
		if( exception.what() )
			m_strLastError += StdStringToPlatformString(std::string("\n\n") + exception.what());
	}

	return false;
}

String^ ZipComponent::GetLastError()
{
	return m_strLastError;
}

// This method is defined in ZipComponent.Version.cpp
// UWP components don't have access to the methods required to read entries out of the modules resource section.
// The pre-build executed UpdateVersion.bat script reads it and updates ZipComponent.Version.cpp accordingly.
//
// String^ ZipComponent::GetVersion()
// {
//     return ...;
// }

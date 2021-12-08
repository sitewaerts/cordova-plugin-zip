#pragma once
#include <string>
#include <vector>
#include "ZipAlgorithm.h"
#include "ZipAlgorithm_miniz-cpp.h"

// Some windows headers define min/max and break usages of std::min in zip_file.hpp
// Undefine them here globally, since we don't need them anyway
#if defined(min)
#	undef min
#endif
#if defined(max)
#	undef max
#endif

#include "miniz-cpp/zip_file.hpp"

using namespace miniz_cpp;


bool ZipAlgorithm_miniz_cpp::Open(const char* pchZipFile, const char* pchOutputDir)
{
	try
	{
		m_pZipFile = new zip_file(pchZipFile);
	}
	catch( std::exception ex )
	{
		Close();
		return false;
	}

	SetOutputDirWithTrailingSlash(pchOutputDir);

	m_vEntries = m_pZipFile->namelist();

	return true;
}

void ZipAlgorithm_miniz_cpp::Close()
{
	m_strOutputDir.clear();
	m_vEntries.clear();

	if( m_pZipFile )
	{
		delete m_pZipFile;
		m_pZipFile = nullptr;
	}
}

size_t ZipAlgorithm_miniz_cpp::GetEntryCount() const
{
	return m_vEntries.size();
}

const std::string& ZipAlgorithm_miniz_cpp::GetEntryName(const size_t szEntryIndex) const
{
	if( szEntryIndex >= m_vEntries.size() )
	{
		std::ostringstream stringStream;
		stringStream << "ZipAlgorithm_miniz_cpp::GetEntryName(): invalid index " << szEntryIndex;
		throw std::runtime_error(stringStream.str());
	}

	return m_vEntries[szEntryIndex];
}

bool ZipAlgorithm_miniz_cpp::UnzipEntry(const size_t szEntryIndex)
{
	if( m_pZipFile == nullptr )
		return false;

	const std::string strEntryName = GetEntryName(szEntryIndex);
	CreateEntrySubDirs(strEntryName);

	// If entry is a directory, stop here, since CreateEntrySubDirs() already created it
	const char chLastChar = strEntryName[strEntryName.length() - 1];
	if( chLastChar == '\\' || chLastChar == '/' )
		return true;

	m_strCache = m_strOutputDir;
	m_strCache += strEntryName;
	std::ofstream ofFile(m_strCache, std::ios::binary | std::ios::out);
	if( ofFile.rdstate() != 0 )
		return false;

	ofFile << m_pZipFile->open(strEntryName).rdbuf();
	if( ofFile.rdstate() != 0 && m_pZipFile->getinfo(strEntryName).file_size == 0 )
		return true;

	return false;
}

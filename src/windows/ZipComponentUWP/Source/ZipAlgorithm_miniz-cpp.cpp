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

	InitializeOutputDir(pchOutputDir);

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
	return m_vEntries[szEntryIndex];
}

bool ZipAlgorithm_miniz_cpp::UnzipEntry(const size_t szEntryIndex) const
{
	if( m_pZipFile )
	{
		const std::string strEntry = GetEntryName(szEntryIndex);
		if( CreateEntryDir(strEntry) )
			return true;

		m_pZipFile->extract(strEntry, m_strOutputDir);
		return true;
	}

	return false;
}

#include <vector>
#include <iostream>
#include <sstream>
#include <direct.h>
#include "ZipAlgorithm.h"
#include "ZipAlgorithm_andyzip.h"
#include "ZipAlgorithm_miniz-cpp.h"


const char* ZipAlgorithm::NameAndyZip	= "andyzip";
const char* ZipAlgorithm::NameMinizCpp	= "miniz-cpp";


ZipAlgorithm* ZipAlgorithm::Create(const char* pchAlgorithmName)
{
	if( pchAlgorithmName == nullptr || ::strlen(pchAlgorithmName) == 0 )
	{
		throw std::runtime_error("ZipAlgorithm::Create(): invalid algorithm name");
		return nullptr;
	}

	if( ::strcmp(pchAlgorithmName, NameAndyZip) == 0 )
		return new ZipAlgorithm_andyzip();
	else if( ::strcmp(pchAlgorithmName, NameMinizCpp) == 0 )
		return new ZipAlgorithm_miniz_cpp();

	std::ostringstream stringStream;
	stringStream << "ZipAlgorithm::Create(): unsupported algorithm name: " << pchAlgorithmName;
	throw std::runtime_error(stringStream.str());

	return nullptr;
}

void ZipAlgorithm::Destroy(ZipAlgorithm*& pAlgorithm)
{
	if( pAlgorithm )
	{
		delete pAlgorithm;
		pAlgorithm = nullptr;
	}
}

void ZipAlgorithm::SetOutputDirWithTrailingSlash(const char* pchOutputDir)
{
	// Only for empty paths we don't append trailing slashes
	if( pchOutputDir == nullptr || pchOutputDir[0] == '\0' )
		m_strOutputDir = "";

	m_strOutputDir = pchOutputDir;

	const char chLastChar = m_strOutputDir[m_strOutputDir.length() - 1];
	if( chLastChar != '/' && chLastChar != '\\' )
		m_strOutputDir += '/';
}

void ZipAlgorithm::CreateEntrySubDirs(const std::string& strEntryName)
{
	// If strEntryName is a filename in a sub folder, we create all required folders first
	// e.g. For strEntryName = "path/to/filename" we create the folders "path" and "path/to".
	size_t szPos = strEntryName.find('/');
	while( szPos != std::string::npos )
	{
		m_strCache = m_strOutputDir; // Reuse m_strCache to reduce string allocations
		m_strCache.append(strEntryName, 0, szPos);
		_mkdir(m_strCache.c_str());
		szPos = strEntryName.find('/', szPos + 1);
	}
}

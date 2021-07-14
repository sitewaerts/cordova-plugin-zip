#include <vector>
#include <iostream>
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

bool ZipAlgorithm::CreateEntryDir(const std::string& strEntryDir) const
{
	const size_t szEntryDirLength = strEntryDir.length();
	if( szEntryDirLength < 1 )
		throw std::runtime_error("ZipAlgorithm::CreateEntryDir(): invalid/empty entry dir");

	const char& chLastChar = strEntryDir[szEntryDirLength - 1];
	if( chLastChar != '/' && chLastChar != '\\' )
		return false;

	const std::string strDir(m_strOutputDir + strEntryDir);
	_mkdir(strDir.c_str());
	return true;
}

void ZipAlgorithm::CreateEntrySubDirs(const std::string& strEntryName) const
{
	// If strEntryName is a filename in a sub folder, we create all required folders first
	// e.g. For strEntryName = "path/to/filename" we create the folders "path" and "path/to".
	size_t szPos = strEntryName.find('/');
	while( szPos != std::string::npos )
	{
		const std::string strFolder(m_strOutputDir + strEntryName.substr(0, szPos));
		_mkdir(strFolder.c_str());
		szPos = strEntryName.find('/', szPos + 1);
	}
}

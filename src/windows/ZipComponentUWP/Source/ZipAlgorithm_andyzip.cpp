#pragma once
#include <fstream>
#include "andyzip/zipfile_reader.hpp"
#include "ZipAlgorithm.h"
#include "ZipAlgorithm_andyzip.h"


bool ZipAlgorithm_andyzip::Open(const char* pchZipFile, const char* pchOutputDir)
{
	std::ifstream ifZipFile(pchZipFile, std::ios::binary | std::ios::ate);
	if( ifZipFile.rdstate() )
		return false;

	const std::streamsize size = ifZipFile.tellg();
	ifZipFile.seekg(0, std::ios::beg);
	m_vBuffer.resize(static_cast<size_t>(size));

	ifZipFile.read(m_vBuffer.data(), size);
	if( ifZipFile.rdstate() )
	{
		Close();
		return false;
	}

	InitializeOutputDir(pchOutputDir);

	m_pReader  = new zipfile_reader(reinterpret_cast<uint8_t*>(m_vBuffer.data()), reinterpret_cast<uint8_t*>(m_vBuffer.data()) + size);
	m_vEntries = m_pReader->filenames();

	return true;
}

void ZipAlgorithm_andyzip::Close()
{
	m_strOutputDir.clear();
	m_vEntries.clear();
	m_vBuffer.clear();

	if( m_pReader )
	{
		delete m_pReader;
		m_pReader = nullptr;
	}
}

size_t ZipAlgorithm_andyzip::GetEntryCount() const
{
	return m_vEntries.size();
}

const std::string& ZipAlgorithm_andyzip::GetEntryName(const size_t szEntryIndex) const
{
	return m_vEntries[szEntryIndex];
}

bool ZipAlgorithm_andyzip::UnzipEntry(const size_t szEntryIndex) const
{
	const std::string& strEntryName = GetEntryName(szEntryIndex);
	if( CreateEntryDir(strEntryName) )
		return true;

	const std::vector<uint8_t>	filedata(m_pReader->read(strEntryName));
	std::ofstream				ofFile(m_strOutputDir + strEntryName, std::ios::binary | std::ios::out);

	ofFile.write(reinterpret_cast<const char*>(filedata.data()), filedata.size());

	return ofFile.rdstate() == 0;
}

#pragma once
#include <fstream>
#include <vector>
#include <stdexcept>
#include <direct.h>
#include <andyzip/deflate_decoder.hpp>
#include "ZipAlgorithm.h"
#include "ZipAlgorithm_andyzip.h"


// Zip structures and definitions taken from: https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
__pragma(pack(push, 1))

	struct SCentralFileHeader // size = 46
	{
		uint32_t central_file_header_signature;   // 4 bytes(0x02014b50)
		uint16_t version_made_by;                 // 2 bytes
		uint16_t version_needed_to_extract;       // 2 bytes
		uint16_t general_purpose_bit_flag;        // 2 bytes
		uint16_t compression_method;              // 2 bytes
		uint16_t last_mod_file_time;              // 2 bytes
		uint16_t last_mod_file_date;              // 2 bytes
		uint32_t crc32;                           // 4 bytes
		uint32_t compressed_size;                 // 4 bytes
		uint32_t uncompressed_size;               // 4 bytes
		uint16_t file_name_length;                // 2 bytes
		uint16_t extra_field_length;              // 2 bytes
		uint16_t file_comment_length;             // 2 bytes
		uint16_t disk_number_start;               // 2 bytes
		uint16_t internal_file_attributes;        // 2 bytes
		uint32_t external_file_attributes;        // 4 bytes
		uint32_t relative_offset_of_local_header; // 4 bytes
		// file name(variable size)
		// extra field(variable size)
		// file comment(variable size)
	};

	struct SLocalFileHeader // size = 30
	{
		uint32_t local_file_header_signature;     // 4 bytes(0x04034b50)
		uint16_t version_needed_to_extract;       // 2 bytes
		uint16_t general_purpose_bit_flag;        // 2 bytes
		uint16_t compression_method;              // 2 bytes
		uint16_t last_mod_file_time;              // 2 bytes
		uint16_t last_mod_file_date;              // 2 bytes
		uint32_t crc32;                           // 4 bytes
		uint32_t compressed_size;                 // 4 bytes
		uint32_t uncompressed_size;               // 4 bytes
		uint16_t file_name_length;                // 2 bytes
		uint16_t extra_field_length;              // 2 bytes
		// file name(variable size)
		// extra field(variable size)
	};

	struct SEndOfCentralDirectory // size = 22
	{
		uint32_t end_of_central_dir_signature;                                                  // 4 bytes(0x06054b50)
		uint16_t number_of_this_disk;                                                           // 2 bytes
		uint16_t number_of_the_disk_with_the_start_of_the_central_directory;                    // 2 bytes
		uint16_t total_number_of_entries_in_the_central_directory_on_this_disk;                 // 2 bytes
		uint16_t total_number_of_entries_in_the_central_directory;                              // 2 bytes
		uint32_t size_of_the_central_directory;                                                 // 4 bytes
		uint32_t offset_of_start_of_central_directory_with_respect_to_the_starting_disk_number; // 4 bytes
		uint16_t ZIP_file_comment_length;                                                       // 2 bytes
		// .ZIP file comment(variable size)
	};

__pragma(pack(pop))


// Optimized reader that minimizes allocations and assumes little endianness
class ZipAlgorithm_andyzip::Reader
{
public:
	Reader(char* pBegin, char* pEnd)
	: m_pBegin(reinterpret_cast<const uint8_t*>(pBegin))
	, m_pEnd(reinterpret_cast<const uint8_t*>(pEnd))
	{
		const uint8_t* p = m_pEnd - sizeof(SEndOfCentralDirectory);
		
		for(; p >= m_pBegin; --p)
			if( *reinterpret_cast<const uint32_t*>(p) == 0x06054b50 )
				break;
	
		m_pCentralDirBegin	= p - reinterpret_cast<const SEndOfCentralDirectory*>(p)->size_of_the_central_directory;
		m_pCentralDirEnd	= p;

		m_vBuffer.reserve(1024 * 1024);
		m_vFiles.reserve(static_cast<size_t>(reinterpret_cast<const SEndOfCentralDirectory*>(p)->total_number_of_entries_in_the_central_directory));

		for(const uint8_t* p = m_pCentralDirBegin; p < m_pCentralDirEnd;)
		{
			if( *reinterpret_cast<const uint32_t*>(p) != 0x02014b50 )
				throw std::runtime_error("ZipAlgorithm_andyzip::ZipReader::filenames(): bad directory entry");

			auto pCFH = reinterpret_cast<const SCentralFileHeader*>(p);

			m_vFiles.emplace_back(reinterpret_cast<const char*>(p + sizeof(SCentralFileHeader)),
								  reinterpret_cast<const char*>(p + sizeof(SCentralFileHeader)) + pCFH->file_name_length);

			p += sizeof(SCentralFileHeader) + pCFH->file_name_length + pCFH->extra_field_length + pCFH->file_comment_length;
		}
	}

	size_t GetEntryCount() const
	{
		return m_vFiles.size();
	}

	const std::string& GetEntryName(const size_t szEntryIndex) const
	{
		return m_vFiles[szEntryIndex];
	}

	const std::vector<uint8_t>& Read(const std::string& strEntryName)
	{
		for(const uint8_t* p = m_pCentralDirBegin; p < m_pCentralDirEnd;)
		{
			auto pCFHeader = reinterpret_cast<const SCentralFileHeader*>(p);
			if(		pCFHeader->file_name_length == static_cast<uint16_t>(strEntryName.size())
				&&	!memcmp(strEntryName.data(), p + sizeof(SCentralFileHeader), pCFHeader->file_name_length) )
			{
				auto			pLFHeader	= reinterpret_cast<const SLocalFileHeader*>(m_pBegin + *reinterpret_cast<const uint32_t*>(p + 42));
				size_t			szDataSize	= 0;
				const uint8_t*	pDataBegin	= nullptr;

				if( pLFHeader->uncompressed_size == 0 )
				{
					szDataSize = static_cast<size_t>(pCFHeader->uncompressed_size);
					pDataBegin = m_pBegin
							   + pCFHeader->relative_offset_of_local_header
							   + sizeof(SLocalFileHeader)
							   + pCFHeader->file_name_length
							   + pCFHeader->extra_field_length;
				}
				else
				{
					szDataSize = static_cast<size_t>(pLFHeader->uncompressed_size);
					pDataBegin = reinterpret_cast<const uint8_t*>(pLFHeader)
							   + sizeof(SLocalFileHeader)
							   + pLFHeader->file_name_length
							   + pLFHeader->extra_field_length;
				}

				m_vBuffer.resize(szDataSize);

				if( !m_Decoder.decode(m_vBuffer.data(), m_vBuffer.data() + szDataSize, pDataBegin, pDataBegin + szDataSize) )
				{
					m_vBuffer.resize(0);
					throw std::runtime_error("ZipAlgorithm_andyzip::ZipReader::read(): deflate decode failure");
				}

				return m_vBuffer;
			}

			p += sizeof(SCentralFileHeader) + pCFHeader->file_name_length + pCFHeader->extra_field_length + pCFHeader->file_comment_length;
		}

		m_vBuffer.resize(0);
		throw std::runtime_error("ZipAlgorithm_andyzip::ZipReader::read(): entry not found");

		return m_vBuffer;
	}

private:
	const uint8_t*				m_pBegin;
	const uint8_t*				m_pEnd;
	const uint8_t*				m_pCentralDirBegin;
	const uint8_t*				m_pCentralDirEnd;
	andyzip::deflate_decoder	m_Decoder;
	std::vector<uint8_t>		m_vBuffer;
	std::vector<std::string>	m_vFiles;
};


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

	m_pReader = new Reader(m_vBuffer.data(), m_vBuffer.data() + size);

	return true;
}

void ZipAlgorithm_andyzip::Close()
{
	m_strOutputDir.clear();
	m_vBuffer.clear();

	if( m_pReader )
	{
		delete m_pReader;
		m_pReader = nullptr;
	}
}

size_t ZipAlgorithm_andyzip::GetEntryCount() const
{
	return m_pReader->GetEntryCount();
}

const std::string& ZipAlgorithm_andyzip::GetEntryName(const size_t szEntryIndex) const
{
	return m_pReader->GetEntryName(szEntryIndex);
}

bool ZipAlgorithm_andyzip::UnzipEntry(const size_t szEntryIndex) const
{
	const std::string& strEntryName = GetEntryName(szEntryIndex);

	size_t szPos = strEntryName.find('/');
	while( szPos != std::string::npos )
	{
		std::string strFolder(m_strOutputDir + strEntryName.substr(0, szPos));
		_mkdir(strFolder.c_str());
		szPos = strEntryName.find('/', szPos + 1);
	}

	const std::vector<uint8_t>&	filedata(m_pReader->Read(strEntryName));
	std::ofstream				ofFile(m_strOutputDir + strEntryName, std::ios::binary | std::ios::out);

	ofFile.write(reinterpret_cast<const char*>(filedata.data()), filedata.size());

	return ofFile.rdstate() == 0;
}

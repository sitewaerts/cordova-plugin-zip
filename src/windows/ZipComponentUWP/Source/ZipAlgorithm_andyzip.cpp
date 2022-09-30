#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <direct.h>
#include <andyzip/deflate_decoder.hpp>
#include "ZipAlgorithm.h"
#include "ZipAlgorithm_andyzip.h"


// Zip structures and definitions taken from: https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
__pragma(pack(push, 2))

	struct SCentralFileHeader // size = 46
	{
		static constexpr const uint32_t Signature = 0x02014b50;

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
		// file_name                                 variable size
		// extra_field                               variable size
		// file_comment                              variable size
	};

	struct SLocalFileHeader // size = 30
	{
		static constexpr const uint32_t Signature = 0x04034b50;

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
		// file_name                                 variable size
		// extra_field                               variable size
	};

	struct SEndOfCentralDirectory // size = 22
	{
		static constexpr const uint32_t Signature = 0x06054b50;

		uint32_t end_of_central_dir_signature;                                                  // 4 bytes(0x06054b50)
		uint16_t number_of_this_disk;                                                           // 2 bytes
		uint16_t number_of_the_disk_with_the_start_of_the_central_directory;                    // 2 bytes
		uint16_t total_number_of_entries_in_the_central_directory_on_this_disk;                 // 2 bytes
		uint16_t total_number_of_entries_in_the_central_directory;                              // 2 bytes
		uint32_t size_of_the_central_directory;                                                 // 4 bytes
		uint32_t offset_of_start_of_central_directory_with_respect_to_the_starting_disk_number; // 4 bytes
		uint16_t ZIP_file_comment_length;                                                       // 2 bytes
		// ZIP_file_comment                                                                        variable size
	};

	template < class SZipStruct > static bool IsZipSignature(const uint8_t* p)
	{
		return *reinterpret_cast<const uint32_t*>(p) == SZipStruct::Signature;
	}

__pragma(pack(pop))


// This is a replacement for the zipfile_reader class.
// It fixes an issue in zipfile_reader with local file headers not containing
// the uncompressed size and reduces allocations by reserving memory upfront.
// Assumes little endianness and supports only deflate for decompression!
class ZipAlgorithm_andyzip::Reader
{
public:
	Reader(char* pBegin, char* pEnd)
	: m_pBegin(reinterpret_cast<const uint8_t*>(pBegin))
	, m_pEnd(reinterpret_cast<const uint8_t*>(pEnd))
	{
		// Find the end of central directory structure
		const uint8_t* p = m_pEnd - sizeof(SEndOfCentralDirectory);
		for(; p >= m_pBegin; --p)
			if( IsZipSignature<SEndOfCentralDirectory>(p) )
				break;
	
		auto pEndOfCentralDir	= reinterpret_cast<const SEndOfCentralDirectory*>(p);
		m_pCentralDirBegin		= p - pEndOfCentralDir->size_of_the_central_directory;
		m_pCentralDirEnd		= p;

		// Reduce allocations by reserving memory upfront
		m_vBuffer.reserve(1024 * 1024);
		m_vFiles.reserve(static_cast<size_t>(pEndOfCentralDir->total_number_of_entries_in_the_central_directory));

		// Iterate over all central file headers and read all file names
		for(const uint8_t* p = m_pCentralDirBegin; p < m_pCentralDirEnd;)
		{
			if( !IsZipSignature<SCentralFileHeader>(p) )
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
		if( szEntryIndex >= m_vFiles.size() )
		{
			std::ostringstream stringStream;
			stringStream << "ZipAlgorithm_andyzip::Reader::GetEntryName(): invalid index " << szEntryIndex;
			throw std::runtime_error(stringStream.str());
		}

		return m_vFiles[szEntryIndex];
	}

	const SCentralFileHeader* GetCentralFileHeader(const std::string& strEntryName) const
	{
		for(const uint8_t* p = m_pCentralDirBegin; p < m_pCentralDirEnd;)
		{
			// Check if this central file header contains the entry's file name
			auto pCFHeader = reinterpret_cast<const SCentralFileHeader*>(p);
			if(		pCFHeader->file_name_length == static_cast<uint16_t>(strEntryName.size() )
				&&	!::memcmp(strEntryName.data(), p + sizeof(SCentralFileHeader), pCFHeader->file_name_length) )
				return pCFHeader;

			p += sizeof(SCentralFileHeader) + pCFHeader->file_name_length + pCFHeader->extra_field_length + pCFHeader->file_comment_length;
		}

		return nullptr;
	}

	const std::vector<uint8_t>& GetEntryData(const std::string& strEntryName)
	{
		const SCentralFileHeader* pCFHeader = GetCentralFileHeader(strEntryName);
		if( pCFHeader == nullptr )
		{
			throw std::runtime_error("ZipAlgorithm_andyzip::ZipReader::read(): entry not found");
			m_vBuffer.resize(0);
			return m_vBuffer;
		}

		// Get the corresponding local file header
		auto			pLFHeader	= reinterpret_cast<const SLocalFileHeader*>(m_pBegin + pCFHeader->relative_offset_of_local_header);
		const size_t	szDataSize	= pLFHeader->uncompressed_size == 0
									? static_cast<size_t>(pCFHeader->uncompressed_size)
									: static_cast<size_t>(pLFHeader->uncompressed_size);
		const uint8_t*	pDataBegin	= reinterpret_cast<const uint8_t*>(pLFHeader)
									+ sizeof(SLocalFileHeader)
									+ pLFHeader->file_name_length
									+ pLFHeader->extra_field_length;

		m_vBuffer.resize(szDataSize);

		if( pCFHeader->compression_method == 8 ) // 8 = deflate algorithm (see zip file format specifications)
		{
			if( !m_Decoder.decode(m_vBuffer.data(), m_vBuffer.data() + szDataSize, pDataBegin, pDataBegin + szDataSize) )
			{
				throw std::runtime_error("ZipAlgorithm_andyzip::ZipReader::read(): deflate decode failure");
				m_vBuffer.resize(0);
			}
		}
		else if( pCFHeader->compression_method == 0 ) // 0 = not compressed (see zip file format specifications)
		{
			::memcpy(m_vBuffer.data(), pDataBegin, szDataSize);
		}
		else
		{
			throw std::runtime_error("ZipAlgorithm_andyzip::ZipReader::read(): unsupported compression method");
			m_vBuffer.resize(0);
		}

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
	if( size < 22 ) // Minimum zip file size (see "https://en.wikipedia.org/wiki/ZIP_(file_format)")
		return false;

	ifZipFile.seekg(0, std::ios::beg);
	m_vBuffer.resize(static_cast<size_t>(size));

	ifZipFile.read(m_vBuffer.data(), size);
	if( ifZipFile.rdstate() )
	{
		Close();
		return false;
	}

	SetOutputDirWithTrailingSlash(pchOutputDir);

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
	return m_pReader ? m_pReader->GetEntryCount() : 0u;
}

const std::string& ZipAlgorithm_andyzip::GetEntryName(const size_t szEntryIndex) const
{
	if( m_pReader == nullptr )
		throw std::runtime_error("ZipAlgorithm_andyzip::GetEntryName(): reader not initialized");

	return m_pReader->GetEntryName(szEntryIndex);
}

bool ZipAlgorithm_andyzip::UnzipEntry(const size_t szEntryIndex)
{
	if( m_pReader == nullptr )
		throw std::runtime_error("ZipAlgorithm_andyzip::GetEntryName(): reader not initialized");

	const std::string& strEntryName = GetEntryName(szEntryIndex);
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

	const std::vector<uint8_t>&	filedata(m_pReader->GetEntryData(strEntryName));
	if( filedata.size() == 0 )
		return true;

	ofFile.write(reinterpret_cast<const char*>(filedata.data()), filedata.size());
	return ofFile.rdstate() == 0;
}

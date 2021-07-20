#include <string>
#include <iostream>
#include <chrono>
#include "ZipAlgorithm.h"


int main(int argc, const char* argv[])
{
	try
	{
		// Print app usage if parameter count doesn't match
		if( argc != 4 )
		{
			const std::string strPath(argv[0]);
			const std::string strAppExecutable(strPath.substr(strPath.find_last_of("/\\") + 1));
			std::cout << "Usage: " << strAppExecutable << " zipFile outputDir algorithm\n\nSupported algorithms:\n";
			std::cout << "  " << ZipAlgorithm::NameAndyZip << "\n";
			std::cout << "  " << ZipAlgorithm::NameMinizCpp << "\n";
			return EXIT_FAILURE;
		}

		// Retrieve input parameters and unzip algorithm matching the provided algorithm name
		const char*	pchZipFile			= argv[1];
		const char*	pchOutputDir		= argv[2];
		const char*	pchAlgorithmName	= argv[3];
		auto*		pAlgorithm			= ZipAlgorithm::Create(pchAlgorithmName);

		if( pAlgorithm == nullptr )
		{
			std::cout << "Unsupported algorithm: " << pchAlgorithmName << "\n";
			return EXIT_FAILURE;
		}

		if( !pAlgorithm->Open(pchZipFile, pchOutputDir) )
		{
			std::cout << "Failed to open zip file: " << pchZipFile << "\n";
			ZipAlgorithm::Destroy(pAlgorithm);
			return EXIT_FAILURE;
		}

		auto unzip = [](ZipAlgorithm* pAlgorithm, const char* pchZipFile, const char* pchOutputDir)
		{
			bool bErrorsOccured = false;
			const size_t szEntries = pAlgorithm->GetEntryCount();
			for( size_t i = 0; i < szEntries; ++i )
				if( !pAlgorithm->UnzipEntry(i) )
					bErrorsOccured = true;

			return bErrorsOccured ? EXIT_FAILURE : EXIT_SUCCESS;
		};

		// Execute the decompression and measure duration
		const auto	timeBegin	= std::chrono::steady_clock::now();
		const int	iResult		= unzip(pAlgorithm, pchZipFile, pchOutputDir);
		const auto	timeEnd		= std::chrono::steady_clock::now();

		pAlgorithm->Close();
		ZipAlgorithm::Destroy(pAlgorithm);

		std::cout << "Duration: " << std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeBegin).count() << " ms";
		if( iResult != EXIT_SUCCESS )
			std::cout << "Error: " << iResult << std::endl;

		return iResult;
	}
	catch(std::exception exception)
	{
		std::cout << "Exception: " << exception.what() << std::endl;
	}

	return EXIT_FAILURE;
}

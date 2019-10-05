#pragma once
#include "stdafx.h"
#include "KPackageProjectPart.h"

namespace Kortex::PackageDesigner
{
	class KPackageProjectConfig: public KPackageProjectPart
	{
		public:
			static bool IsCompressionMethodSupported(const wxString& value);
	
		public:
			static const int ms_MinCompressionLevel = 0;
			static const int ms_MaxCompressionLevel = 9;
			static const int ms_DefaultCompressionLevel = 5;
	
			static const int ms_MinDictionarySize = 0;
			static const int ms_MaxDictionarySize = 10;
			static const int ms_DefaultDictionarySize = 5;
			static const wxString ms_DefaultCompressionMethod;
	
		private:
			wxString m_InstallPackageFile;
	
			wxString m_CompressionMethod = ms_DefaultCompressionMethod;
			int m_CompressionLevel = ms_DefaultCompressionLevel;
			int m_CompressionDictionarySize = ms_DefaultDictionarySize;
			bool m_CompressionUseMultithreading = false;
			bool m_CompressionSolidArchive = false;
	
		public:
			KPackageProjectConfig(KPackageProject& project);
			virtual ~KPackageProjectConfig();
	
		public:
			const wxString& GetInstallPackageFile() const
			{
				return m_InstallPackageFile;
			}
			void SetInstallPackageFile(const wxString& value)
			{
				m_InstallPackageFile = value;
			}
	
			const wxString& GetCompressionMethod() const
			{
				return m_CompressionMethod;
			}
			void SetCompressionMethod(const wxString& value);
			
			int GetCompressionLevel() const
			{
				return m_CompressionLevel;
			}
			void SetCompressionLevel(int value);
			
			int GetCompressionDictionarySize() const
			{
				return m_CompressionDictionarySize;
			}
			void SetCompressionDictionarySize(int value);
			
			bool IsMultithreadingUsed() const
			{
				return m_CompressionUseMultithreading;
			}
			void SetUseMultithreading(bool value)
			{
				m_CompressionUseMultithreading = value;
			}
			
			bool IsSolidArchive() const
			{
				return m_CompressionSolidArchive;
			}
			void SetSolidArchive(bool value)
			{
				m_CompressionSolidArchive = value;
			}
	};
}

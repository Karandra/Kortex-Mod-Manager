#pragma once
#include "stdafx.h"
#include "ModPackages/IPackageManager.h"
#include "PackageProject/KPackageProject.h"
#include "Archive/KArchive.h"
#include "Utility/KLabeledValue.h"
#include <KxFramework/KxFileStream.h>

namespace Kortex
{
	class ModPackage final
	{
		private:
			static const size_t ms_InvalidIndex = std::numeric_limits<size_t>::max();

		private:
			KArchive m_Archive;
			KxFileStream m_Stream;
			PackageDesigner::KPackageProject m_Config;
			wxString m_PackageFilePath;
			wxString m_EffectiveArchiveRoot;
			PackageDesigner::KPPPackageType m_PackageType = PackageDesigner::KPP_PACCKAGE_UNKNOWN;

			KArchive::BufferMap m_DocumentsBuffer;
			bool m_DocumentsLoaded = false;

		public:
			wxBitmap ReadImage(const KArchive::Buffer& buffer) const;
			wxBitmap ReadImage(size_t index) const
			{
				if (index != ms_InvalidIndex)
				{
					return ReadImage(m_Archive.ExtractToMemory(index));
				}
				return wxNullBitmap;
			}
			wxString ReadString(const KArchive::Buffer& buffer, bool isASCII = false) const;
			wxString ReadString(size_t index, bool isASCII = false) const
			{
				if (index != ms_InvalidIndex)
				{
					return ReadString(m_Archive.ExtractToMemory(index), isASCII);
				}
				return wxEmptyString;
			}

		private:
			void LoadConfig(PackageDesigner::KPackageProject& project);
			void LoadConfigNative(PackageDesigner::KPackageProject& project, size_t index);
			void LoadConfigSMI(PackageDesigner::KPackageProject& project, size_t index);
			void LoadConfigFOMod(PackageDesigner::KPackageProject& project, size_t infoIndex, size_t moduleConfigIndex);
		
			wxString DetectEffectiveArchiveRoot(const KxFileItem& item, const wxString& subPath = {}) const;
			void SetModIDIfNone();

			void LoadBasicResources();
			void LoadImageResources();
			void LoadDocumentResources();

		private:
			void Init(const wxString& archivePath);

		public:
			ModPackage();
			ModPackage(const wxString& archivePath);
			ModPackage(const wxString& archivePath, PackageDesigner::KPackageProject& project);
			bool Create(const wxString& archivePath);
			bool Create(const wxString& archivePath, PackageDesigner::KPackageProject& project);
			~ModPackage();

		public:
			bool IsOK() const;
			bool IsTypeSupported() const;

			const wxString& GetPackageFilePath() const
			{
				return m_PackageFilePath;
			}
			const KArchive& GetArchive() const
			{
				return m_Archive;
			}
			KArchive& GetArchive()
			{
				return m_Archive;
			}
			PackageDesigner::KPPPackageType GetType() const
			{
				return m_PackageType;
			}
			const PackageDesigner::KPackageProject& GetConfig() const
			{
				return m_Config;
			}
			PackageDesigner::KPackageProject& GetConfig()
			{
				return m_Config;
			}
		
			void LoadResources();

			wxString GetName() const
			{
				return m_Config.GetModName();
			}
			const PackageDesigner::KPPIImageEntry* GetLogoImage() const
			{
				return m_Config.GetInterface().GetMainImageEntry();
			}
			const wxString& GetDescription() const
			{
				return m_Config.GetInfo().GetDescription();
			}
		
			const KArchive::BufferMap& GetDocumentsBuffer() const
			{
				return m_DocumentsBuffer;
			}
			const KArchive::Buffer& GetDocumentBuffer(const KLabeledValue& entry) const
			{
				return m_DocumentsBuffer.at((size_t)entry.GetClientData());
			}
			wxString GetSimpleDocument(const KLabeledValue& entry) const
			{
				return ReadString(GetDocumentBuffer(entry));
			}
	};
}

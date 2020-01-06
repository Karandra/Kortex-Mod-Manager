#pragma once
#include "stdafx.h"
#include "ModPackages/IPackageManager.h"
#include "PackageProject/ModPackageProject.h"
#include "Archive/GenericArchive.h"
#include "Utility/LabeledValue.h"
#include <KxFramework/KxFileStream.h>

namespace Kortex
{
	class ModPackage final
	{
		private:
			static const size_t ms_InvalidIndex = std::numeric_limits<size_t>::max();

		private:
			GenericArchive m_Archive;
			KxFileStream m_Stream;
			ModPackageProject m_Config;
			wxString m_PackageFilePath;
			wxString m_EffectiveArchiveRoot;
			PackageProject::PackageType m_PackageType = PackageProject::PackageType::Unknown;

			std::unordered_map<KxArchive::FileIndex, std::unique_ptr<wxMemoryOutputStream>> m_DocumentsBuffer;
			bool m_DocumentsLoaded = false;

		public:
			wxBitmap ReadImage(wxInputStream& stream) const;
			wxBitmap ReadImage(wxMemoryOutputStream& stream) const
			{
				wxMemoryInputStream inStream(stream);
				return ReadImage(inStream);
			}
			wxBitmap ReadImage(size_t index) const;
			wxString ReadString(wxInputStream& stream, bool isASCII = false) const;
			wxString ReadString(wxMemoryOutputStream& stream, bool isASCII = false) const
			{
				wxMemoryInputStream inStream(stream);
				return ReadString(inStream, isASCII);
			}
			wxString ReadString(size_t index, bool isASCII = false) const;

		private:
			void LoadConfig(ModPackageProject& project);
			void LoadConfigNative(ModPackageProject& project, size_t index);
			void LoadConfigSMI(ModPackageProject& project, size_t index);
			void LoadConfigFOMod(ModPackageProject& project, size_t infoIndex, size_t moduleConfigIndex);
		
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
			ModPackage(const wxString& archivePath, ModPackageProject& project);
			bool Create(const wxString& archivePath);
			bool Create(const wxString& archivePath, ModPackageProject& project);
			~ModPackage();

		public:
			bool IsOK() const;
			bool IsTypeSupported() const;

			const wxString& GetPackageFilePath() const
			{
				return m_PackageFilePath;
			}
			const GenericArchive& GetArchive() const
			{
				return m_Archive;
			}
			GenericArchive& GetArchive()
			{
				return m_Archive;
			}
			PackageProject::PackageType GetType() const
			{
				return m_PackageType;
			}
			const ModPackageProject& GetConfig() const
			{
				return m_Config;
			}
			ModPackageProject& GetConfig()
			{
				return m_Config;
			}
		
			void LoadResources();

			wxString GetName() const
			{
				return m_Config.GetModName();
			}
			const PackageProject::ImageItem* GetLogoImage() const
			{
				return m_Config.GetInterface().GetMainItem();
			}
			const wxString& GetDescription() const
			{
				return m_Config.GetInfo().GetDescription();
			}

			std::unique_ptr<wxInputStream> GetDocumentStream(const Utility::LabeledValue& item) const;
			wxString GetSimpleDocument(const Utility::LabeledValue& item) const
			{
				if (auto stream = GetDocumentStream(item))
				{
					return ReadString(*stream);
				}
				return {};
			}
	};
}

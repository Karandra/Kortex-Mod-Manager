#pragma once
#include "stdafx.h"
#include "KPackageManager.h"
#include "PackageProject/KPackageProject.h"
#include "Archive/KArchive.h"
#include "Utility/KLabeledValue.h"
#include <KxFramework/KxFileStream.h>

class KModPackage
{
	private:
		static const size_t ms_InvalidIndex = (size_t)-1;

	private:
		wxString m_PackageFilePath;
		KArchive m_Archive;
		KxFileStream m_Stream;
		KPPPackageType m_PackageType = KPP_PACCKAGE_UNKNOWN;
		KPackageProject m_Config;

		wxString m_EffectiveArchiveRoot;
		bool m_DocumentsLoaded = false;
		KArchive::BufferMap m_DocumentsBuffer;

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
		void LoadConfig(KPackageProject& project);
		void LoadConfigNative(KPackageProject& project, size_t index);
		void LoadConfigSMI(KPackageProject& project, size_t index);
		void LoadConfigFOMod(KPackageProject& project, size_t infoIndex, size_t moduleConfigIndex);
		
		wxString DetectEffectiveArchiveRoot(const KxFileItem& item) const;
		void SetModIDIfNone();

		void LoadBasicResources();
		void LoadImageResources();
		void LoadDocumentResources();

	private:
		void Init(const wxString& archivePath);

	public:
		KModPackage();
		KModPackage(const wxString& archivePath);
		KModPackage(const wxString& archivePath, KPackageProject& project);
		bool Create(const wxString& archivePath);
		bool Create(const wxString& archivePath, KPackageProject& project);
		virtual ~KModPackage();

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
		KPPPackageType GetType() const
		{
			return m_PackageType;
		}
		const KPackageProject& GetConfig() const
		{
			return m_Config;
		}
		KPackageProject& GetConfig()
		{
			return m_Config;
		}
		
		void LoadResources();

		wxString GetName() const
		{
			return m_Config.GetModName();
		}
		const KPPIImageEntry* GetLogoImage() const
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

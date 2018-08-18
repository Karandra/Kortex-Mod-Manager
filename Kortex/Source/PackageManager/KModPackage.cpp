#include "stdafx.h"
#include "KModPackage.h"
#include "PackageProject/KPackageProjectSerializerKMP.h"
#include "PackageProject/KPackageProjectSerializerSMI.h"
#include "PackageProject/KPackageProjectSerializerFOMod.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxShell.h>

wxBitmap KModPackage::ReadImage(const KAcrhiveBuffer& buffer) const
{
	wxMemoryInputStream stream(buffer.data(), buffer.size());
	wxImage image;
	image.LoadFile(stream);

	return wxBitmap(image, 32);
}
wxString KModPackage::ReadString(const KAcrhiveBuffer& buffer, bool isASCII) const
{
	if (!isASCII)
	{
		// Try to detect if this is UTF-16LE (using byte order mark)
		if (buffer.size() >= 2)
		{
			uint8_t bom[2] = {buffer[0], buffer[1]};
			if (bom[0] == 0xFF && bom[1] == 0xFE)
			{
				return wxString((const wchar_t*)buffer.data(), buffer.size() / sizeof(wchar_t));
			}
		}
		return wxString::FromUTF8((const char*)buffer.data(), buffer.size());
	}
	else
	{
		return wxString((const char*)buffer.data(), buffer.size());
	}
}
void KModPackage::LoadConfig(KPackageProject& project)
{
	if (m_Archive.GetProperty_CompressionFormat() != KARC_FORMAT_UNKNOWN)
	{
		// Native format
		{
			size_t index = m_Archive.FindFirstFile("KortexPackage.xml");
			if (index != ms_InvalidIndex)
			{
				m_PackageType = KPP_PACCKAGE_NATIVE;
				LoadConfigNative(project, index);
				return;
			}
		}

		// SMI/AMI legacy format
		{
			size_t index = m_Archive.FindFirstFileIn("SetupInfo", "Setup.xml");
			if (index == ms_InvalidIndex)
			{
				index = m_Archive.FindFirstFileIn("SetupInfo", "Project.smp");
			}
			if (index != ms_InvalidIndex)
			{
				m_PackageType = KPP_PACCKAGE_LEGACY;
				LoadConfigSMI(project, index);
				return;
			}
		}

		// FOMod (Configurable)
		auto TryScriptedFOMod = [this]()
		{
			size_t index = m_Archive.FindFirstFile("Script.cs");
			if (index != ms_InvalidIndex)
			{
				return index;
			}
			return ms_InvalidIndex;
		};

		{
			size_t infoIndex = m_Archive.FindFirstFile("Info.xml");
			size_t moduleConfigIndex = m_Archive.FindFirstFile("ModuleConfig.xml");
			if (infoIndex != ms_InvalidIndex || moduleConfigIndex != ms_InvalidIndex)
			{
				// No module config
				if (moduleConfigIndex == ms_InvalidIndex && TryScriptedFOMod() != ms_InvalidIndex)
				{
					// Set as scripted but load available XMLs
					m_PackageType = KPP_PACCKAGE_FOMOD_CSHARP;
					LoadConfigFOMod(project, infoIndex, moduleConfigIndex);
					return;
				}

				// This is valid configurable FOMod
				m_PackageType = KPP_PACCKAGE_FOMOD_XML;
				LoadConfigFOMod(project, infoIndex, moduleConfigIndex);
				return;
			}
		}

		// FOMod (Scripted)
		if (TryScriptedFOMod() != ms_InvalidIndex)
		{
			m_PackageType = KPP_PACCKAGE_FOMOD_CSHARP;
		}
	}
}
void KModPackage::LoadConfigNative(KPackageProject& project, size_t index)
{
	KPackageProjectSerializerKMP serializer(false);
	serializer.SetData(ReadString(index));
	serializer.Structurize(&project);
}
void KModPackage::LoadConfigSMI(KPackageProject& project, size_t index)
{
	KPackageProjectSerializerSMI serializer;
	serializer.SetData(ReadString(index, true));
	serializer.Structurize(&project);
}
void KModPackage::LoadConfigFOMod(KPackageProject& project, size_t infoIndex, size_t moduleConfigIndex)
{
	KAcrhiveBufferMap buffers = m_Archive.Extract({(uint32_t)infoIndex, (uint32_t)moduleConfigIndex});

	KPackageProjectSerializerFOMod serializer(ReadString(buffers[infoIndex]), ReadString(buffers[moduleConfigIndex]));
	serializer.SetEffectiveArchiveRoot(DetectEffectiveArchiveRoot(infoIndex != ms_InvalidIndex ? infoIndex : moduleConfigIndex));
	serializer.Structurize(&project);
}
const wxString& KModPackage::DetectEffectiveArchiveRoot(size_t index)
{
	// This currently only works for FOMod's
	if (index != ms_InvalidIndex)
	{
		wxString path = m_Archive.GetItemName(index);
		size_t startIndex = KxString::Find(path, "FOMod", 0, false);

		// If 'FOMod' folder found not at string beginning,
		// then effective root folder is right before it.
		if (startIndex != wxNOT_FOUND && startIndex != 0)
		{
			// Subtract 1 as we do not need the path separator
			m_EffectiveArchiveRoot = path.Mid(0, startIndex - 1);
		}
	}
	return m_EffectiveArchiveRoot;
}
void KModPackage::LoadBasicResources()
{
	KPPIImageEntry* pLogo = m_Config.GetInterface().GetMainImageEntry();
	if (pLogo)
	{
		pLogo->SetBitmap(ReadImage(m_Archive.FindFirstFile(pLogo->GetPath(), false)));
	}
}
void KModPackage::LoadImageResources()
{
	KxUInt32Vector indexes;
	std::unordered_map<size_t, KPPIImageEntry*> entriesMap;
	for (KPPIImageEntry& entry: m_Config.GetInterface().GetImages())
	{
		size_t index = m_Archive.FindFirstFile(entry.GetPath(), false);
		if (index != ms_InvalidIndex)
		{
			entriesMap.insert_or_assign(index, &entry);
			indexes.push_back(index);
		}
	}

	if (!indexes.empty())
	{
		KAcrhiveBufferMap buffers = m_Archive.Extract(indexes);
		for (auto& v: buffers)
		{
			entriesMap[v.first]->SetBitmap(ReadImage(v.second));
		}
	}
}
void KModPackage::LoadDocumentResources()
{
	if (!m_DocumentsLoaded)
	{
		KxUInt32Vector indexes;
		std::unordered_map<size_t, KLabeledValue*> entriesMap;
		for (KLabeledValue& entry: m_Config.GetInfo().GetDocuments())
		{
			size_t index = m_Archive.FindFirstFile(entry.GetValue(), false);
			if (index != ms_InvalidIndex)
			{
				entriesMap.insert_or_assign(index, &entry);
				indexes.push_back(index);
			}
		}

		if (!indexes.empty())
		{
			m_DocumentsBuffer = m_Archive.Extract(indexes);
			for (auto& v: m_DocumentsBuffer)
			{
				entriesMap[v.first]->SetClientData((void*)v.first);
			}
			m_DocumentsLoaded = true;
		}
	}
}

void KModPackage::Init(const wxString& archivePath)
{
	m_PackageFilePath = archivePath;

	m_Stream.Close();
	m_Stream.Open(m_PackageFilePath, KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
}

KModPackage::KModPackage()
{
}
KModPackage::KModPackage(const wxString& archivePath)
{
	Create(archivePath);
}
KModPackage::KModPackage(const wxString& archivePath, KPackageProject& project)
{
	Create(archivePath, project);
}
bool KModPackage::Create(const wxString& archivePath)
{
	Init(archivePath);

	if (m_Archive.Open(m_PackageFilePath))
	{
		LoadConfig(m_Config);
		if (IsOK())
		{
			LoadBasicResources();
			return true;
		}
	}
	return false;
}
bool KModPackage::Create(const wxString& archivePath, KPackageProject& project)
{
	Init(archivePath);

	if (m_Archive.Open(m_PackageFilePath))
	{
		LoadConfig(project);
		return IsOK();
	}
	return false;
}
KModPackage::~KModPackage()
{
}

bool KModPackage::IsOK() const
{
	return !m_PackageFilePath.IsEmpty() &&
		m_Archive.GetProperty_CompressionFormat() != KARC_FORMAT_UNKNOWN &&
		m_PackageType != KPP_PACCKAGE_UNKNOWN &&
		!m_Config.ComputeModID().IsEmpty();
}
bool KModPackage::IsTypeSupported() const
{
	return m_PackageType == KPP_PACCKAGE_NATIVE ||
		m_PackageType == KPP_PACCKAGE_LEGACY ||
		m_PackageType == KPP_PACCKAGE_FOMOD_XML;
}

void KModPackage::LoadResources()
{
	LoadImageResources();
	LoadDocumentResources();
}

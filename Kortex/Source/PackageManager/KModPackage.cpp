#include "stdafx.h"
#include "KModPackage.h"
#include "PackageProject/KPackageProjectSerializerKMP.h"
#include "PackageProject/KPackageProjectSerializerSMI.h"
#include "PackageProject/KPackageProjectSerializerFOMod.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxShell.h>

wxBitmap KModPackage::ReadImage(const KArchive::Buffer& buffer) const
{
	wxMemoryInputStream stream(buffer.data(), buffer.size());
	wxImage image;
	image.LoadFile(stream);

	return wxBitmap(image, 32);
}
wxString KModPackage::ReadString(const KArchive::Buffer& buffer, bool isASCII) const
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
	if (m_Archive.GetPropertyInt(KArchiveNS::PropertyInt::Format) != (int)KArchiveNS::Format::Unknown)
	{
		SetModIDIfNone();

		// Native format
		{
			KxFileItem item;
			if (m_Archive.FindFile("KortexPackage.xml", item))
			{
				m_PackageType = KPP_PACCKAGE_NATIVE;
				LoadConfigNative(project, item.GetExtraData<size_t>());
				return;
			}
		}

		// SMI/AMI legacy format
		{
			KxFileItem item;
			if (!m_Archive.FindFileInFolder("SetupInfo", "Setup.xml", item))
			{
				m_Archive.FindFileInFolder("SetupInfo", "Project.smp", item);
			}
			if (item.IsOK())
			{
				m_PackageType = KPP_PACCKAGE_LEGACY;
				LoadConfigSMI(project, item.GetExtraData<size_t>());
				return;
			}
		}

		// FOMod (Configurable)
		auto TryScriptedFOMod = [this]()
		{
			KxFileItem item;
			if (m_Archive.FindFile("Script.cs", item))
			{
				return item.GetExtraData<size_t>();
			}
			return ms_InvalidIndex;
		};

		{
			KxFileItem infoItem;
			KxFileItem moduleConfigItem;

			m_Archive.FindFile("Info.xml", infoItem);
			m_Archive.FindFile("ModuleConfig.xml", moduleConfigItem);

			if (infoItem.IsOK() || moduleConfigItem.IsOK())
			{
				// No module config
				if (!moduleConfigItem.IsOK() && TryScriptedFOMod() != ms_InvalidIndex)
				{
					// Set as scripted but load available XMLs
					m_PackageType = KPP_PACCKAGE_FOMOD_CSHARP;
					LoadConfigFOMod(project, infoItem.GetExtraData<size_t>(), moduleConfigItem.GetExtraData<size_t>());
					return;
				}

				// This is valid configurable FOMod
				m_PackageType = KPP_PACCKAGE_FOMOD_XML;
				LoadConfigFOMod(project, infoItem.GetExtraData<size_t>(), moduleConfigItem.GetExtraData<size_t>());
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
	KArchive::BufferMap buffers = m_Archive.ExtractToMemory({(uint32_t)infoIndex, (uint32_t)moduleConfigIndex});

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
void KModPackage::SetModIDIfNone()
{
	if (m_Config.ComputeModID().IsEmpty())
	{
		m_Config.SetModID(m_PackageFilePath.AfterLast('\\').BeforeLast('.'));
	}
}

void KModPackage::LoadBasicResources()
{
	KPPIImageEntry* logo = m_Config.GetInterface().GetMainImageEntry();
	if (logo)
	{
		KxFileItem item;
		if (m_Archive.FindFile(logo->GetPath(), item))
		{
			logo->SetBitmap(ReadImage(item.GetExtraData<size_t>()));
		}
	}
}
void KModPackage::LoadImageResources()
{
	KxUInt32Vector indexes;
	std::unordered_map<size_t, KPPIImageEntry*> entriesMap;
	for (KPPIImageEntry& entry: m_Config.GetInterface().GetImages())
	{
		KxFileItem item;
		if (m_Archive.FindFile(entry.GetPath(), item))
		{
			size_t index = item.GetExtraData<size_t>();
			entriesMap.insert_or_assign(index, &entry);
			indexes.push_back(index);
		}
	}

	if (!indexes.empty())
	{
		KArchive::BufferMap buffers = m_Archive.ExtractToMemory(indexes);
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
			KxFileItem item;
			if (m_Archive.FindFile(entry.GetValue(), item))
			{
				size_t index = item.GetExtraData<size_t>();
				entriesMap.insert_or_assign(index, &entry);
				indexes.push_back(index);
			}
		}

		if (!indexes.empty())
		{
			m_DocumentsBuffer = m_Archive.ExtractToMemory(indexes);
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
		m_Archive.GetPropertyInt(KArchiveNS::PropertyInt::Format) != (int)KArchiveNS::Format::Unknown &&
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

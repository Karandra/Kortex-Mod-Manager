#include "stdafx.h"
#include "ModPackage.h"
#include "PackageProject/NativeSerializer.h"
#include "PackageProject/LegacySerializer.h"
#include "PackageProject/FOModSerializer.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxShell.h>

namespace Kortex
{
	wxBitmap ModPackage::ReadImage(const KArchive::Buffer& buffer) const
	{
		wxMemoryInputStream stream(buffer.data(), buffer.size());
		wxImage image;
		image.LoadFile(stream);

		return wxBitmap(image, 32);
	}
	wxString ModPackage::ReadString(const KArchive::Buffer& buffer, bool isASCII) const
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

	void ModPackage::LoadConfig(ModPackageProject& project)
	{
		if (m_Archive.GetPropertyInt(KArchiveNS::PropertyInt::Format) != (int)KArchiveNS::Format::Unknown)
		{
			SetModIDIfNone();

			// Native format
			{
				KxFileItem item;
				if (m_Archive.FindFile("KortexPackage.xml", item))
				{
					m_PackageType = PackageProject::PackageType::Native;
					m_EffectiveArchiveRoot = DetectEffectiveArchiveRoot(item);
					LoadConfigNative(project, item.GetExtraData<size_t>());
					return;
				}
			}

			// SMI/AMI legacy format
			{
				KxFileItem item;
				wxString rootFolder = "SetupInfo";
				if (!m_Archive.FindFileInFolder(rootFolder, "Setup.xml", item))
				{
					m_Archive.FindFileInFolder(rootFolder, "Project.smp", item);
				}
				if (item.IsOK())
				{
					m_PackageType = PackageProject::PackageType::Legacy;
					m_EffectiveArchiveRoot = DetectEffectiveArchiveRoot(item, rootFolder);
					LoadConfigSMI(project, item.GetExtraData<size_t>());
					return;
				}
			}

			// FOMod (Configurable)
			auto TryScriptedFOMod = [this]()
			{
				KxFileItem item;
				if (m_Archive.FindFile("*Script.cs", item))
				{
					m_EffectiveArchiveRoot = DetectEffectiveArchiveRoot(item);
					return item.GetExtraData<size_t>();
				}
				return ms_InvalidIndex;
			};

			{
				KxFileItem infoItem;
				KxFileItem moduleConfigItem;
				m_Archive.FindFile("*Info.xml", infoItem);
				m_Archive.FindFile("*ModuleConfig.xml", moduleConfigItem);

				if (infoItem.IsOK() || moduleConfigItem.IsOK())
				{
					m_EffectiveArchiveRoot = DetectEffectiveArchiveRoot(moduleConfigItem.IsOK() ? moduleConfigItem : infoItem, "FOMod");

					// No module config
					if (!moduleConfigItem.IsOK() && TryScriptedFOMod() != ms_InvalidIndex)
					{
						// Set as scripted but load available XMLs
						m_PackageType = PackageProject::PackageType::FOModCSharp;
						LoadConfigFOMod(project, infoItem.GetExtraData<size_t>(), moduleConfigItem.GetExtraData<size_t>());
						return;
					}

					// This is valid configurable FOMod
					m_PackageType = PackageProject::PackageType::FOModXML;
					LoadConfigFOMod(project, infoItem.GetExtraData<size_t>(), moduleConfigItem.GetExtraData<size_t>());
					return;
				}
			}

			// FOMod (Scripted)
			if (TryScriptedFOMod() != ms_InvalidIndex)
			{
				m_PackageType = PackageProject::PackageType::FOModCSharp;
			}
		}
	}
	void ModPackage::LoadConfigNative(ModPackageProject& project, size_t index)
	{
		PackageProject::NativeSerializer serializer(false);
		serializer.SetData(ReadString(index));
		serializer.Structurize(&project);
	}
	void ModPackage::LoadConfigSMI(ModPackageProject& project, size_t index)
	{
		PackageProject::LegacySerializer serializer;
		serializer.SetData(ReadString(index, true));
		serializer.Structurize(&project);
	}
	void ModPackage::LoadConfigFOMod(ModPackageProject& project, size_t infoIndex, size_t moduleConfigIndex)
	{
		KArchive::BufferMap buffers = m_Archive.ExtractToMemory({(uint32_t)infoIndex, (uint32_t)moduleConfigIndex});
		PackageProject::FOModSerializer serializer(ReadString(buffers[infoIndex]), ReadString(buffers[moduleConfigIndex]));
		serializer.SetEffectiveArchiveRoot(m_EffectiveArchiveRoot);
		serializer.Structurize(&project);
	}

	wxString ModPackage::DetectEffectiveArchiveRoot(const KxFileItem& item, const wxString& subPath) const
	{
		wxString path = item.GetSource();
		if (!path.IsEmpty())
		{
			if (!subPath.IsEmpty())
			{
				// If path is going to be something like "123456\\FOMod" then 'subPath' should contain "FOMod" part.
				// Here we're going to remove it to get effective root.
				wxString afterRoot;
				if (wxString actualRoot = path.BeforeLast(wxS('\\'), &afterRoot); KxComparator::IsEqual(afterRoot, subPath, true))
				{
					return actualRoot;
				}
			}
			return path;
		}
		return {};
	}
	void ModPackage::SetModIDIfNone()
	{
		if (m_Config.GetModID().IsEmpty())
		{
			m_Config.SetModID(m_PackageFilePath.AfterLast('\\').BeforeLast('.'));
		}
	}

	void ModPackage::LoadBasicResources()
	{
		PackageProject::ImageItem* logo = m_Config.GetInterface().GetMainImageEntry();
		if (logo)
		{
			KxFileItem item;
			if (m_Archive.FindFile(logo->GetPath(), item))
			{
				logo->SetBitmap(ReadImage(item.GetExtraData<size_t>()));
			}
		}
	}
	void ModPackage::LoadImageResources()
	{
		KxUInt32Vector indexes;
		std::unordered_map<size_t, PackageProject::ImageItem*> entriesMap;
		for (PackageProject::ImageItem& entry: m_Config.GetInterface().GetImages())
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
	void ModPackage::LoadDocumentResources()
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

	void ModPackage::Init(const wxString& archivePath)
	{
		m_PackageFilePath = archivePath;

		m_Stream.Close();
		m_Stream.Open(m_PackageFilePath, KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
	}

	ModPackage::ModPackage()
	{
	}
	ModPackage::ModPackage(const wxString& archivePath)
	{
		Create(archivePath);
	}
	ModPackage::ModPackage(const wxString& archivePath, ModPackageProject& project)
	{
		Create(archivePath, project);
	}
	bool ModPackage::Create(const wxString& archivePath)
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
	bool ModPackage::Create(const wxString& archivePath, ModPackageProject& project)
	{
		Init(archivePath);

		if (m_Archive.Open(m_PackageFilePath))
		{
			LoadConfig(project);
			return IsOK();
		}
		return false;
	}
	ModPackage::~ModPackage()
	{
	}

	bool ModPackage::IsOK() const
	{
		return !m_PackageFilePath.IsEmpty() &&
			m_Archive.GetPropertyInt(KArchiveNS::PropertyInt::Format) != (int)KArchiveNS::Format::Unknown &&
			m_PackageType != PackageProject::PackageType::Unknown &&
			!m_Config.GetModID().IsEmpty();
	}
	bool ModPackage::IsTypeSupported() const
	{
		return m_PackageType == PackageProject::PackageType::Native ||
			m_PackageType == PackageProject::PackageType::Legacy ||
			m_PackageType == PackageProject::PackageType::FOModXML;
	}

	void ModPackage::LoadResources()
	{
		LoadImageResources();
		LoadDocumentResources();
	}
}

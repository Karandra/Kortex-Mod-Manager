#include "stdafx.h"
#include "ModPackage.h"
#include "PackageProject/Serializer/NativeSerializer.h"
#include "PackageProject/Serializer/LegacySerializer.h"
#include "PackageProject/Serializer/FOModSerializer.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxStreamDelegate.h>
#include <KxFramework/KxShell.h>

namespace
{
	std::vector<uint8_t> ReadStringBuffer(wxInputStream& stream)
	{
		std::vector<uint8_t> buffer;
		buffer.resize(stream.GetLength());

		if (!stream.ReadAll(buffer.data(), buffer.size()))
		{
			buffer.clear();
		}
		return buffer;
	}
}

namespace Kortex
{
	wxBitmap ModPackage::ReadImage(wxInputStream& stream) const
	{
		wxImage image;
		image.LoadFile(stream);

		return wxBitmap(image, 32);
	}
	wxBitmap ModPackage::ReadImage(size_t index) const
	{
		if (index != ms_InvalidIndex)
		{
			wxMemoryOutputStream stream;
			if (m_Archive.ExtractToStream(index, stream))
			{
				wxMemoryInputStream inputStream(stream);
				return ReadImage(inputStream);
			}
		}
		return wxNullBitmap;
	}

	wxString ModPackage::ReadString(wxInputStream& stream, bool isASCII) const
	{
		if (!isASCII)
		{
			constexpr uint8_t BOM[] = {0xFF, 0xFE};

			// Try to detect if this is UTF-16LE (using byte order mark)
			if (stream.GetLength() >= 2)
			{
				if (stream.GetC() == BOM[0] && stream.GetC() == BOM[1])
				{
					auto buffer = ReadStringBuffer(stream);
					return wxString(reinterpret_cast<const wchar_t*>(buffer.data()), buffer.size() / sizeof(wchar_t));
				}

				stream.SeekI(0, wxSeekMode::wxFromStart);
			}

			auto buffer = ReadStringBuffer(stream);
			return wxString::FromUTF8(reinterpret_cast<const char*>(buffer.data()), buffer.size());
		}
		else
		{
			auto buffer = ReadStringBuffer(stream);
			return wxString(reinterpret_cast<const char*>(buffer.data()), buffer.size());
		}
	}
	wxString ModPackage::ReadString(size_t index, bool isASCII) const
	{
		if (index != ms_InvalidIndex)
		{
			wxMemoryOutputStream stream;
			if (m_Archive.ExtractToStream(index, stream))
			{
				wxMemoryInputStream inputStream(stream);
				return ReadString(inputStream, isASCII);
			}
		}
		return wxEmptyString;
	}

	void ModPackage::LoadConfig(ModPackageProject& project)
	{
		if (m_Archive.GetPropertyInt(Archive::PropertyInt::Format) != (int)Archive::Format::Unknown)
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
		serializer.Structurize(project);
	}
	void ModPackage::LoadConfigSMI(ModPackageProject& project, size_t index)
	{
		PackageProject::LegacySerializer serializer;
		serializer.SetData(ReadString(index, true));
		serializer.Structurize(project);
	}
	void ModPackage::LoadConfigFOMod(ModPackageProject& project, size_t infoIndex, size_t moduleConfigIndex)
	{
		wxMemoryOutputStream infoOutStream;
		wxMemoryOutputStream moduleConfigOutStream;
		m_Archive.ExtractWith().OnGetStream([&](size_t fileIndex) -> KxDelegateOutputStream
		{
			if (fileIndex == infoIndex || fileIndex == moduleConfigIndex)
			{
				KxFileItem fileItem = m_Archive.GetItem(fileIndex);
				if (fileItem && !fileItem.IsDirectory())
				{
					if (fileIndex == infoIndex)
					{
						return infoOutStream;
					}
					else if (fileIndex == moduleConfigIndex)
					{
						return moduleConfigOutStream;
					}
				}
			}
			return nullptr;
		}).Execute();

		wxMemoryInputStream infoStream(infoOutStream);
		wxMemoryInputStream moduleConfigStream(moduleConfigOutStream);

		PackageProject::FOModSerializer serializer(ReadString(infoStream), ReadString(moduleConfigStream));
		serializer.SetEffectiveArchiveRoot(m_EffectiveArchiveRoot);
		serializer.Structurize(project);
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
		PackageProject::ImageItem* logo = m_Config.GetInterface().GetMainItem();
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
		KxArchive::FileIndexVector files;
		std::unordered_map<size_t, PackageProject::ImageItem*> entriesMap;
		for (PackageProject::ImageItem& entry: m_Config.GetInterface().GetImages())
		{
			KxFileItem item;
			if (m_Archive.FindFile(entry.GetPath(), item))
			{
				size_t index = item.GetExtraData<size_t>();
				entriesMap.insert_or_assign(index, &entry);
				files.push_back(index);
			}
		}

		m_Archive.ExtractWith<wxMemoryOutputStream>().OnGetStream([&](size_t fileIndex) -> KxDelegateOutputStream
		{
			return std::make_unique<wxMemoryOutputStream>();
		}).OnOperationCompleted([&](size_t fileIndex, wxMemoryOutputStream& stream)
		{
			entriesMap[fileIndex]->SetBitmap(ReadImage(stream));
			return true;
		}).Execute(files);
	}
	void ModPackage::LoadDocumentResources()
	{
		if (!m_DocumentsLoaded)
		{
			KxUInt32Vector files;
			std::unordered_map<size_t, Utility::LabeledValue*> entriesMap;
			for (Utility::LabeledValue& entry: m_Config.GetInfo().GetDocuments())
			{
				KxFileItem item;
				if (m_Archive.FindFile(entry.GetValue(), item))
				{
					size_t index = item.GetExtraData<size_t>();
					entriesMap.insert_or_assign(index, &entry);
					files.push_back(index);
				}
			}

			m_Archive.ExtractWith().OnGetStream([&](size_t fileIndex) -> KxDelegateOutputStream
			{
				entriesMap[fileIndex]->SetClientData(reinterpret_cast<void*>(fileIndex));

				auto it = m_DocumentsBuffer.insert_or_assign(fileIndex, std::make_unique<wxMemoryOutputStream>());
				return *it.first->second;
			}).Execute(files);
			m_DocumentsLoaded = !m_DocumentsBuffer.empty();
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
			m_Archive.GetPropertyInt(Archive::PropertyInt::Format) != (int)Archive::Format::Unknown &&
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

	std::unique_ptr<wxInputStream> ModPackage::GetDocumentStream(const Utility::LabeledValue& item) const
	{
		auto it = m_DocumentsBuffer.find(reinterpret_cast<size_t>(item.GetClientData()));
		if (it != m_DocumentsBuffer.end())
		{
			return std::make_unique<wxMemoryInputStream>(*it->second);
		}
		return nullptr;
	}
}

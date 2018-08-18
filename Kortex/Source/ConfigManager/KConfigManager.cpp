#include "stdafx.h"
#include "KConfigManager.h"
#include "KCMConfigEntry.h"
#include "KCMDataProviderINI.h"
#include "Profile/KProfile.h"
#include "Profile/KConfigManagerConfig.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModManagerDispatcher.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxSystemSettings.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxXML.h>

//KxSingletonPtr_Define(KConfigManager);

#define MIN(T)	std::numeric_limits<T>::lowest()
#define MAX(T)	std::numeric_limits<T>::max()

const KConfigManager::NameToTypeMap KConfigManager::ms_NameToTypeMap
{
	std::make_pair("unknown", KCMDT_UNKNOWN),
	std::make_pair("int8", KCMDT_INT8),
	std::make_pair("uint8", KCMDT_UINT8),
	std::make_pair("int16", KCMDT_INT16),
	std::make_pair("uint16", KCMDT_UINT16),
	std::make_pair("int32", KCMDT_INT32),
	std::make_pair("uint32", KCMDT_UINT32),
	std::make_pair("int64", KCMDT_INT64),
	std::make_pair("uint64", KCMDT_UINT64),
	std::make_pair("float32", KCMDT_FLOAT32),
	std::make_pair("float64", KCMDT_FLOAT64),
	std::make_pair("bool", KCMDT_BOOL),
	std::make_pair("string", KCMDT_STRING),
};

const wxString& KConfigManager::GetCategorySeparator()
{
	static const wxString ms_CategorySeparator('/');
	return ms_CategorySeparator;
}
wxString KConfigManager::GetConfigFile(const wxString& fileName)
{
	return wxString::Format("%s\\ConfigManager\\%s.xml", KApp::Get().GetDataFolder(), fileName);
}
wxString KConfigManager::GetConfigFileForTemplate(const wxString& templateID)
{
	return wxString::Format("%s\\ConfigManager\\Profiles\\%s.xml", KApp::Get().GetDataFolder(), templateID);
}
const KConfigManagerConfig* KConfigManager::GetGameConfig()
{
	return KConfigManagerConfig::GetInstance();
}

bool KConfigManager::IsIntType(KCMDataType type)
{
	switch (type)
	{
		case KCMDT_INT8:
		case KCMDT_UINT8:
		case KCMDT_INT16:
		case KCMDT_UINT16:
		case KCMDT_INT32:
		case KCMDT_UINT32:
		case KCMDT_INT64:
		case KCMDT_UINT64:
		{
			return true;
		}
	};
	return false;
}
bool KConfigManager::IsSignedIntType(KCMDataType type)
{
	switch (type)
	{
		case KCMDT_INT8:
		case KCMDT_INT16:
		case KCMDT_INT32:
		case KCMDT_INT64:
		{
			return true;
		}
	};
	return false;
}
bool KConfigManager::IsUnsignedIntType(KCMDataType type)
{
	switch (type)
	{
		case KCMDT_UINT8:
		case KCMDT_UINT16:
		case KCMDT_UINT32:
		case KCMDT_UINT64:
		{
			return true;
		}
	};
	return false;
}
bool KConfigManager::IsFloatType(KCMDataType type)
{
	switch (type)
	{
		case KCMDT_FLOAT32:
		case KCMDT_FLOAT64:
		{
			return true;
		}
	};
	return false;
}
bool KConfigManager::IsNumericType(KCMDataType type)
{
	return IsIntType(type) || IsFloatType(type);
}
bool KConfigManager::IsBoolType(KCMDataType type)
{
	return type == KCMDT_BOOL;
}
bool KConfigManager::IsStringType(KCMDataType type)
{
	switch (type)
	{
		case KCMDT_STRING:
		{
			return true;
		}
	};
	return false;
}

std::pair<int64_t, int64_t> KConfigManager::GetMinMaxSignedValue(KCMDataType type)
{
	switch (type)
	{
		case KCMDT_INT8:
		{
			return std::make_pair(MIN(int8_t), MAX(int8_t));
		}
		case KCMDT_INT16:
		{
			return std::make_pair(MIN(int16_t), MAX(int16_t));
		}
		case KCMDT_INT32:
		{
			return std::make_pair(MIN(int32_t), MAX(int32_t));
		}
		case KCMDT_INT64:
		{
			return std::make_pair(MIN(int64_t), MAX(int64_t));
		}
	}
	return std::make_pair(0, 0);
}
std::pair<int64_t, int64_t> KConfigManager::GetMinMaxUnsignedValue(KCMDataType type)
{
	switch (type)
	{
		case KCMDT_UINT8:
		{
			return std::make_pair(MIN(uint8_t), MAX(uint8_t));
		}
		case KCMDT_UINT16:
		{
			return std::make_pair(MIN(uint16_t), MAX(uint16_t));
		}
		case KCMDT_UINT32:
		{
			return std::make_pair(MIN(uint32_t), MAX(uint32_t));
		}
		case KCMDT_UINT64:
		{
			return std::make_pair(MIN(uint64_t), MAX(uint64_t));
		}
	}
	return std::make_pair(0, 0);
}
std::pair<double, double> KConfigManager::GetMinMaxFloatValue(KCMDataType type)
{
	switch (type)
	{
		case KCMDT_FLOAT32:
		{
			return std::make_pair(MIN(float), MAX(float));
		}
		case KCMDT_FLOAT64:
		{
			return std::make_pair(MIN(double), MAX(double));
		}
	}
	return std::make_pair(0, 0);
}

KCMDataType KConfigManager::GetTypeID(const wxString& name)
{
	if (ms_NameToTypeMap.count(name))
	{
		return ms_NameToTypeMap.at(name);
	}
	return KCMDT_UNKNOWN;
}
wxString KConfigManager::GetTypeName(KCMDataType type)
{
	auto tElement = std::find_if(ms_NameToTypeMap.cbegin(), ms_NameToTypeMap.cend(), [type](const NameToTypeMap::value_type& tElement)
	{
		return tElement.second == type;
	});
	if (tElement != ms_NameToTypeMap.cend())
	{
		return (*tElement).first;
	}
	else
	{
		return "unknown";
	}
}

KCMTypeDetector KConfigManager::GetTypeDetectorID(const wxString& name)
{
	if (name == "HungarianNotation")
	{
		return KCM_DETECTOR_HUNGARIAN_NOTATION;
	}
	else if (name == "DataAnalysis")
	{
		return KCM_DETECTOR_DATA_ANALYSIS;
	}
	return KCM_DETECTOR_INVALID;
}

KCMSampleValueArray KConfigManager::FF_GetVideoAdapterList(KCMConfigEntryStd* configEntry, KxXMLNode& node)
{
	KCMSampleValueArray outList;
	auto tAdaptersList = KxSystemSettings::EnumVideoAdapters();
	for (const auto& tAdapter: tAdaptersList)
	{
		outList.push_back(KCMSampleValue(configEntry, tAdapter.DeviceString));
	}
	return outList;
}
KCMSampleValueArray KConfigManager::FF_GetVideoModesList(KCMConfigEntryStd* configEntry, KxXMLNode& node)
{
	KCMSampleValueArray outList;

	KCMConfigEntryDV* pDVEntry = configEntry->ToDVEntry();
	if (pDVEntry)
	{
		auto tModesList = KxSystemSettings::EnumVideoModes(wxEmptyString);
		std::unordered_set<wxString> tHash;

		for (const auto& tMode: tModesList)
		{
			wxString sHashString = wxString::Format("%dx%d", (int)tMode.Width, (int)tMode.Height);
			if (!tHash.count(sHashString))
			{
				tHash.emplace(sHashString);

				wxString sWidth = pDVEntry->GetFormatter()((uint32_t)tMode.Width);
				wxString sHeight = pDVEntry->GetFormatter()((uint32_t)tMode.Height);
				wxString label = KAux::GetResolutionRatio(wxSize(tMode.Width, tMode.Height));
				wxString value = wxString::Format("%s%s%s", sWidth, pDVEntry->GetSeparator(), sHeight);

				if (label.IsEmpty())
				{
					label = T("ConfigManager.Samples.VideoMode.RatioUnknown");
				}
				outList.push_back(KCMSampleValue(pDVEntry, value, label));
			}
		}
	}
	return outList;
}
KCMSampleValueArray KConfigManager::FF_GetVirtualKeys(KCMConfigEntryStd* configEntry, KxXMLNode& node)
{
	KCMSampleValueArray tVirtualKeys;
	for (const auto& tKeyInfo: configEntry->GetFileEntry()->GetConfigManager()->GetVirtualKeys())
	{
		tVirtualKeys.push_back(KCMSampleValue(configEntry, std::to_wstring(tKeyInfo.first), tKeyInfo.second.second));
	}
	return tVirtualKeys;
}
KCMSampleValueArray KConfigManager::FF_FindFiles(KCMConfigEntryStd* configEntry, KxXMLNode& node)
{
	KxXMLNode tSampleValuesNode = node.GetFirstChildElement("SampleValues");

	KCMSampleValueArray tFoundFiles;
	wxString folder = V(tSampleValuesNode.GetAttribute("Folder"));
	wxString filter = KAux::StrOr(tSampleValuesNode.GetAttribute("Filter"), KxFile::NullFilter);
	bool bRecursiveSearch = tSampleValuesNode.GetAttributeBool("Recursive");
	KxStringVector files = KxFile(folder).Find(filter, KxFS_FILE, bRecursiveSearch);
	for (const wxString& s: files)
	{
		tFoundFiles.emplace_back(KCMSampleValue(configEntry, s.AfterLast('\\')));
	}
	return tFoundFiles;
}

void KConfigManager::LoadMainFile(KxXMLDocument& xml)
{
	LoadFormatOptions(xml);
	LoadAdditionalFile(xml);
}
void KConfigManager::LoadAdditionalFile(KxXMLDocument& xml, bool bAllowENB)
{
	InitTypeDetectors(xml);
	LoadCategories(xml);
	LoadFileRecords(xml, bAllowENB);
}

void KConfigManager::LoadFormatOptions(KxXMLDocument& xml)
{
	m_Formatter.Load(xml.QueryElement("Config/FormatOptions"));
}
void KConfigManager::InitTypeDetectors(KxXMLDocument& xml)
{
	m_Detector_HungarianNotation.Init(xml.QueryElement("Config/TypeDetection/HungarianNotation"));
}
void KConfigManager::LoadCategories(KxXMLDocument& xml)
{
	for (KxXMLNode node = xml.QueryElement("Config/Categories").GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
	{
		wxString id = node.GetValue();
		if (!id.IsEmpty())
		{
			wxString label;
			if (node.HasAttribute("Label"))
			{
				label = KApp::Get().ExpandVariables(node.GetAttribute("Label"));
			}
			else
			{
				label = T(wxString::Format("GameConfig.Categories.%s", id));
			}

			if (label.IsEmpty())
			{
				label = id;
			}
			m_Categories.emplace(std::make_pair(id, label));
		}
	}
}
void KConfigManager::LoadFileRecords(KxXMLDocument& xml, bool bAllowENB)
{
	// Add files from parameters XML
	KxXMLNode tDataNode = xml.QueryElement("Config/Data");
	for (KxXMLNode node = tDataNode.GetFirstChildElement("File"); node.IsOK(); node = node.GetNextSiblingElement("File"))
	{
		KCMFileEntry* fileEntry = m_Files.emplace_back(new KCMFileEntry(this, node, m_Formatter));
		fileEntry->Load();
	}

	// Add files not listed in XML
	for (size_t i = 0; i < GetGameConfig()->GetEntriesCount(); i++)
	{
		const KConfigManagerConfigEntry* pProfileEntry = GetGameConfig()->GetEntryAt(i);
		if (pProfileEntry->IsGameConfigID() && (bAllowENB || !pProfileEntry->IsENBID()))
		{
			auto tElement = std::find_if(m_Files.cbegin(), m_Files.cend(), [pProfileEntry](const KCMFileEntry* fileEntry)
			{
				return pProfileEntry->GetID() == fileEntry->GetID();
			});
			if (tElement == m_Files.cend())
			{
				KCMFileEntry* fileEntry = m_Files.emplace_back(new KCMFileEntry(this, pProfileEntry, m_Formatter));
				fileEntry->Load();
			}
		}
	}
}

void KConfigManager::LoadVirtualKeys()
{
	KxFileStream tXMLStream(GetConfigFile("VirtualKeys"), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
	KxXMLDocument xml(tXMLStream);

	KxXMLNode node = xml.QueryElement("VirtualKeys");
	for (node = node.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
	{
		unsigned long nKeyCode = WXK_NONE;
		wxString value = node.GetValue();
		if (value.Mid(2).ToCULong(&nKeyCode, 16) || value.ToCULong(&nKeyCode, 16))
		{
			wxString sVKID = node.GetAttribute("VKID");
			wxString name = KAux::StrOr(node.GetAttribute("Name"), sVKID, std::to_wstring(nKeyCode));

			bool bHasTranslation = false;
			wxString label = KxTranslation::GetString(wxString::Format("ConfigManager.VirtualKey.%s", sVKID), &bHasTranslation);
			if (bHasTranslation)
			{
				name = label;
			}

			m_VirtualKeys.emplace(std::make_pair((wxKeyCode)nKeyCode, std::make_pair(sVKID, name)));
		}
	}

	if (!m_VirtualKeys.count(WXK_NONE))
	{
		m_VirtualKeys.emplace(std::make_pair(WXK_NONE, std::make_pair("VK_NONE", T("ConfigManager.VirtualKey.VK_NONE"))));
	}
}

KConfigManager::KConfigManager(KWorkspace* workspace)
	:m_Workspace(workspace)
{
	LoadVirtualKeys();
}
KConfigManager::KConfigManager(KWorkspace* workspace, const wxString& templateID)
	:m_Workspace(workspace), m_FilePath(GetConfigFileForTemplate(templateID))
{
	LoadVirtualKeys();

	KxFileStream tXMLStream(m_FilePath, KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
	m_XML.Load(tXMLStream);

	LoadMainFile(m_XML);
}
KConfigManager::~KConfigManager()
{
	Clear();
	for (auto& v: m_AdditionalXML)
	{
		delete v.second;
	}
}

wxString KConfigManager::GetID() const
{
	return "KConfigManager";
}
wxString KConfigManager::GetName() const
{
	return T("ToolBar.ConfigManager");
}
wxString KConfigManager::GetVersion() const
{
	return "1.1";
}

void KConfigManager::AddFile(const wxString& fileName, bool bAllowENB)
{
	KxFileStream tXMLStream(fileName, KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
	m_AdditionalXML.push_back(std::make_pair(fileName, new KxXMLDocument(tXMLStream)));

	LoadAdditionalFile(*(m_AdditionalXML.back().second), bAllowENB);
}
void KConfigManager::AddENB()
{
	AddFile(GetConfigFile("ENB"), true);
}

KCMFileEntry* KConfigManager::GetEntry(KPGCFileID id) const
{
	auto tElement = std::find_if(m_Files.cbegin(), m_Files.cend(), [id](const KCMFileEntry* v)
	{
		return id == v->GetID();
	});
	if (tElement != m_Files.cend())
	{
		return (*tElement);
	}
	return NULL;
}
const KConfigManager::VirtualKeyMapInfo& KConfigManager::GetVirtualKeyInfo(wxKeyCode nKeyCode) const
{
	if (m_VirtualKeys.count(nKeyCode))
	{
		return m_VirtualKeys.at(nKeyCode);
	}
	return m_VirtualKeys.at(WXK_NONE);
}

void KConfigManager::Clear()
{
	m_Detector_HungarianNotation.Clear();
	m_Detector_DataAnalysis.Clear();
	m_Categories.clear();
	m_Formatter = KCMOptionsFormatter();

	for (KCMFileEntry* fileEntry: m_Files)
	{
		delete fileEntry;
	}
	m_Files.clear();
	
	for (auto& v: m_Providers)
	{
		delete v.second;
	}
	m_Providers.clear();
}
void KConfigManager::Reload()
{
	Clear();

	LoadMainFile(m_XML);
	for (auto& v: m_AdditionalXML)
	{
		LoadAdditionalFile(*v.second);
	}
}

KConfigManager::FillFunnctionType KConfigManager::OnQueryFillFunction(const wxString& name)
{
	if (name == "GetVideoAdapterList")
	{
		return FF_GetVideoAdapterList;
	}
	else if (name == "GetVideoModesList")
	{
		return FF_GetVideoModesList;
	}
	else if (name == "GetVirtualKeys")
	{
		return FF_GetVirtualKeys;
	}
	else if (name == "FindFiles")
	{
		return FF_FindFiles;
	}
	return NULL;
}
KCMIDataProvider* KConfigManager::OnQueryDataProvider(const KCMFileEntry* fileEntry)
{
	if (m_Providers.count(fileEntry->GetID()))
	{
		return m_Providers.at(fileEntry->GetID());
	}
	else
	{
		const KConfigManagerConfigEntry* pProfileEntry = fileEntry->GetProfileEntry();
		if (pProfileEntry)
		{
			switch (pProfileEntry->GetFormat())
			{
				case KPGC_FORMAT_INI:
				{
					KCMDataProviderINI* pDataProvider = NULL;
					if (pProfileEntry->IsFilePathRelative())
					{
						wxString path = KModManager::GetDispatcher().GetTargetPath(pProfileEntry->GetFilePath());
						if (!path.IsEmpty())
						{
							pDataProvider = new KCMDataProviderINI(path);
						}
					}
					else
					{
						pDataProvider = new KCMDataProviderINI(pProfileEntry->GetFilePath());
					}

					if (pDataProvider)
					{
						m_Providers.emplace(std::make_pair(fileEntry->GetID(), pDataProvider));
						return pDataProvider;
					}
				}
			};
		}
	}
	return NULL;
}
KCMValueTypeDetector* KConfigManager::OnQueryTypeDetector(KCMTypeDetector id)
{
	switch (id)
	{
		case KCM_DETECTOR_HUNGARIAN_NOTATION:
		{
			return &m_Detector_HungarianNotation;
		}
		case KCM_DETECTOR_DATA_ANALYSIS:
		{
			return &m_Detector_DataAnalysis;
		}
	};
	return NULL;
}

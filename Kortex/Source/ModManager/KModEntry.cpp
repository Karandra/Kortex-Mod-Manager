#include "stdafx.h"
#include "KModEntry.h"
#include "KModManager.h"
#include "Network/KNetwork.h"
#include "GameInstance/KGameInstance.h"
#include "PackageProject/KPackageProject.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxCrypto.h>
#include <KxFramework/KxXML.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxFileFinder.h>

wxString KModEntry::GetSignatureFromID(const wxString& id)
{
	auto utf8 = id.ToUTF8();
	wxMemoryInputStream stream(utf8.data(), utf8.length());
	return KxCrypto::MD5(stream);
}

int64_t KModEntry::GetWebSiteModID(const FixedWebSitesArray& array, KNetworkProviderID index)
{
	if (index > KNETWORK_PROVIDER_ID_INVALID && index < KNETWORK_PROVIDER_ID_MAX)
	{
		return array[index];
	}
	return -1;
}
bool KModEntry::HasWebSite(const FixedWebSitesArray& array, KNetworkProviderID index)
{
	return GetWebSiteModID(array, index) != KNETWORK_SITE_INVALID_MODID;
}
KLabeledValue KModEntry::GetWebSite(const FixedWebSitesArray& array, KNetworkProviderID index, const wxString& signature)
{
	int64_t modID = GetWebSiteModID(array, index);
	if (modID != KNETWORK_SITE_INVALID_MODID)
	{
		KNetworkProvider* site = KNetwork::GetInstance()->GetProvider(index);
		return KLabeledValue(site->GetModURL(modID, signature), site->GetName());
	}
	return KLabeledValue(wxEmptyString);
}
void KModEntry::SetWebSite(FixedWebSitesArray& array, KNetworkProviderID index, KNetworkModID modID)
{
	if (index > KNETWORK_PROVIDER_ID_INVALID && index < KNETWORK_PROVIDER_ID_MAX)
	{
		array[index] = modID;
	}
}
bool KModEntry::HasTag(KxStringVector& array, const wxString& value)
{
	return std::find(array.begin(), array.end(), value) != array.end();
}
bool KModEntry::ToggleTag(KxStringVector& array, const wxString& value, bool setTag)
{
	auto it = std::find(array.begin(), array.end(), value);
	if (setTag && it == array.cend())
	{
		array.push_back(value);
		return true;
	}
	else if (!setTag && it != array.cend())
	{
		array.erase(it);
		return true;
	}
	return false;
}

bool KModEntry::IsInstalledReal() const
{
	return KxFile(GetLocation(KMM_LOCATION_MOD_FILES)).IsFolderExist();
}

KModEntry::KModEntry()
{
}
KModEntry::~KModEntry()
{
}

void KModEntry::CreateFromID(const wxString& id)
{
	m_ID = id;
	m_Signature = GetSignatureFromID(id);

	CreateFromSignature(m_Signature);
}
void KModEntry::CreateFromSignature(const wxString& signature)
{
	m_Signature = signature;

	if (!m_Signature.IsEmpty())
	{
		KxFileStream xmlStream(GetLocation(KMM_LOCATION_MOD_INFO), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
		KxXMLDocument xml(xmlStream);
		if (xml.IsOK())
		{
			KxXMLNode tRoot = xml.GetFirstChildElement("Mod");
			if (m_Signature == tRoot.GetAttribute("Signature"))
			{
				m_ID = tRoot.GetFirstChildElement("ID").GetValue();
				m_Name = tRoot.GetFirstChildElement("Name").GetValue();

				// Check ID validity
				if (m_ID.IsEmpty())
				{
					if (!m_Name.IsEmpty())
					{
						m_ID = m_Name;
					}
					else
					{
						m_Signature.Clear();
						return;
					}
				}
				if (m_Name.IsEmpty())
				{
					m_Name = m_ID;
				}

				m_Version = tRoot.GetFirstChildElement("Version").GetValue();
				m_Author = tRoot.GetFirstChildElement("Author").GetValue();

				// Tags
				KxXMLNode tagsNode = tRoot.GetFirstChildElement("Tags");
				KAux::LoadStringArray(m_Tags, tagsNode);
				m_PriorityGroupTag = tagsNode.GetAttribute("PriorityGroup");

				// Sites
				KxXMLNode tWebSitesNode = tRoot.GetFirstChildElement("Sites");
				m_FixedWebSites[KNETWORK_PROVIDER_ID_TESALL] = tWebSitesNode.GetAttributeInt("TESALLID", -1);
				m_FixedWebSites[KNETWORK_PROVIDER_ID_NEXUS] = tWebSitesNode.GetAttributeInt("NexusID", -1);
				m_FixedWebSites[KNETWORK_PROVIDER_ID_LOVERSLAB] = tWebSitesNode.GetAttributeInt("LoversLabID", -1);

				for (KxXMLNode node = tWebSitesNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
				{
					m_WebSites.emplace_back(KLabeledValue(node.GetValue(), node.GetAttribute("Label")));
				}

				// Time
				KxXMLNode tTimeNode = tRoot.GetFirstChildElement("Time");
				if (tTimeNode.IsOK())
				{
					auto ParseTime = [this, &tTimeNode](const auto& name, KMETimeIndex index)
					{
						KxXMLNode node = tTimeNode.GetFirstChildElement(name);
						if (node.IsOK())
						{
							m_Time[index].ParseISOCombined(node.GetValue());
						}
					};
					ParseTime("Install", KME_TIME_INSTALL);
					ParseTime("Uninstall", KME_TIME_UNINSTALL);
				}

				// Package file
				m_InstallPackageFile = tRoot.GetFirstChildElement("InstallPackage").GetValue();

				// Linked mod config
				KxXMLNode tLinkedModNode = tRoot.GetFirstChildElement("LinkedMod");
				if (tLinkedModNode.IsOK())
				{
					m_LinkedModFilesPath = tLinkedModNode.GetAttribute("FolderPath");
				}
			}
		}
	}
}
void KModEntry::CreateFromProject(const KPackageProject& config)
{
	const KPackageProjectInfo& info = config.GetInfo();

	SetID(config.ComputeModID());
	m_Name = config.ComputeModName();
	m_Author = info.GetAuthor();
	m_Version = info.GetVersion();
	m_Description = info.GetDescription();
	m_IsDescriptionChanged = true;

	m_Tags = info.GetTags();
	m_FixedWebSites = info.GetFixedWebSites();
	m_WebSites = info.GetWebSites();

	// Remove any empty sites
	m_WebSites.erase(std::remove_if(m_WebSites.begin(), m_WebSites.end(), [](const KLabeledValue& value)
	{
		return !value.HasValue() && !value.HasLabel();
	}), m_WebSites.end());
}
void KModEntry::CreateAllFolders()
{
	KxFile(GetLocation(KMM_LOCATION_MOD_ROOT)).CreateFolder();

	if (!IsLinkedMod())
	{
		KxFile(GetLocation(KMM_LOCATION_MOD_FILES)).CreateFolder();
	}
}
bool KModEntry::Save()
{
	// Mod root is always needed here but other folders isn't
	KxFile(GetLocation(KMM_LOCATION_MOD_ROOT)).CreateFolder();

	KxFileStream stream(GetLocation(KMM_LOCATION_MOD_INFO), KxFS_ACCESS_WRITE, KxFS_DISP_CREATE_ALWAYS);
	if (stream.IsOk())
	{
		KxXMLDocument xml;
		auto SaveValueArray = [&xml](KxXMLNode& node, const KxStringVector& array, const wxString& name)
		{
			KxXMLNode arrayNode = node.NewElement(name);
			for (const wxString& value: array)
			{
				arrayNode.NewElement("Entry").SetValue(value);
			}
			return arrayNode;
		};
		auto SaveLabeledValueArray = [&xml](KxXMLNode& node, const KLabeledValueArray& array, const wxString& name)
		{
			KxXMLNode arrayNode = node.NewElement(name);
			for (const KLabeledValue& value: array)
			{
				KxXMLNode elementNode = arrayNode.NewElement("Entry");
				elementNode.SetValue(value.GetValue());
				if (value.HasLabel())
				{
					elementNode.SetAttribute("Label", value.GetLabel());
				}
			}
			return arrayNode;
		};
		auto SaveTime = [this, &xml](KxXMLNode& node, const wxString& name)
		{
			KxXMLNode arrayNode = node.NewElement(name);

			auto SetTime = [&arrayNode](wxDateTime& t, const char* sEntryName)
			{
				arrayNode.NewElement(sEntryName).SetValue(t.IsValid() ? t.FormatISOCombined() : wxEmptyString);
			};
			SetTime(m_Time[KME_TIME_INSTALL], "Install");
			SetTime(m_Time[KME_TIME_UNINSTALL], "Uninstall");

			return arrayNode;
		};
		auto SaveFixedSite = [this](KxXMLNode& node, const wxString& name, KNetworkProviderID index)
		{
			if (m_FixedWebSites[index] != -1)
			{
				node.SetAttribute(name, m_FixedWebSites[index]);
			}
		};

		// Signature
		KxXMLNode rootNode = xml.NewElement("Mod");
		rootNode.SetAttribute("Signature", m_Signature);

		// Generic info
		rootNode.NewElement("ID").SetValue(m_ID);
		rootNode.NewElement("Name").SetValue(GetName()); // Field 'm_Name' can be empty and GetName() returns 'm_ID' in this case
		rootNode.NewElement("Version").SetValue(m_Version);
		rootNode.NewElement("Author").SetValue(m_Author);

		KxXMLNode tagsNode = rootNode.NewElement("Tags");
		if (!m_PriorityGroupTag.IsEmpty())
		{
			tagsNode.SetAttribute("PriorityGroup", m_PriorityGroupTag);
		}
		KAux::SaveStringArray(m_Tags, tagsNode);

		// Sites
		KxXMLNode webSitesNode = SaveLabeledValueArray(rootNode, m_WebSites, "Sites");
		SaveFixedSite(webSitesNode, "NexusID", KNETWORK_PROVIDER_ID_NEXUS);
		SaveFixedSite(webSitesNode, "TESALLID", KNETWORK_PROVIDER_ID_TESALL);
		SaveFixedSite(webSitesNode, "LoversLabID", KNETWORK_PROVIDER_ID_LOVERSLAB);

		// Time and package
		SaveTime(rootNode, "Time");
		rootNode.NewElement("InstallPackage").SetValue(m_InstallPackageFile);

		// Description
		if (IsDescriptionChanged())
		{
			KxFileStream tDescriptionFile(GetLocation(KMM_LOCATION_MOD_DESCRIPTION), KxFS_ACCESS_WRITE, KxFS_DISP_CREATE_ALWAYS);
			tDescriptionFile.SetEnd();
			tDescriptionFile.WriteStringUTF8(m_Description);

			m_IsDescriptionChanged = false;
		}

		// Linked mod config
		if (IsLinkedMod())
		{
			KxXMLNode tLinkedModNode = rootNode.NewElement("LinkedMod");
			tLinkedModNode.SetAttribute("FolderPath", m_LinkedModFilesPath);
		}

		return xml.Save(stream);
	}
	return false;
}

bool KModEntry::IsOK() const
{
	return !m_ID.IsEmpty() && !m_Signature.IsEmpty();
}
void KModEntry::SetID(const wxString& id)
{
	m_ID = id;
	m_Signature = GetSignatureFromID(id);
}

const wxString& KModEntry::GetPriorityGroupTag() const
{
	return m_PriorityGroupTag;
}
void KModEntry::SetPriorityGroupTag(const wxString& value)
{
	m_PriorityGroupTag = value;
}

bool KModEntry::IsInstallPackageFileExist() const
{
	return KxFile(m_InstallPackageFile).IsFileExist();
}

const wxString& KModEntry::GetDescription() const
{
	if (m_Description.IsEmpty() && !IsDescriptionChanged())
	{
		KxFileStream stream(GetLocation(KMM_LOCATION_MOD_DESCRIPTION), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
		if (stream.IsOk())
		{
			m_Description = stream.ReadStringUTF8(stream.GetLength());
		}
	}
	return m_Description;
}
void KModEntry::SetDescription(const wxString& value)
{
	m_Description = value;
	m_IsDescriptionChanged = true;
}

int64_t KModEntry::GetWebSiteModID(KNetworkProviderID index) const
{
	return GetWebSiteModID(m_FixedWebSites, index);
}
bool KModEntry::HasWebSite(KNetworkProviderID index) const
{
	return HasWebSite(m_FixedWebSites, index);
}
KLabeledValue KModEntry::GetWebSite(KNetworkProviderID index) const
{
	return GetWebSite(m_FixedWebSites, index, m_Signature);
}
void KModEntry::SetWebSite(KNetworkProviderID index, KNetworkModID modID)
{
	SetWebSite(m_FixedWebSites, index, modID);
}

void KModEntry::ClearFileTree()
{
	m_FileTree.GetChildren().clear();
}
void KModEntry::UpdateFileTree()
{
	ClearFileTree();

	std::function<size_t(const wxString&, KFileTreeNode* treeNode, KFileTreeNode* parentNode)> Recurse;
	Recurse = [this, &Recurse](const wxString& path, KFileTreeNode* treeNode, KFileTreeNode* parentNode = NULL)
	{
		size_t itemsCount = 0;
		if (treeNode)
		{
			treeNode->GetChildren().reserve(Recurse(path, NULL, NULL));
		}

		KxFileFinder finder(path, wxS("*"));
		KxFileItem item = finder.FindNext();
		while (item.IsOK())
		{
			if (item.IsNormalItem())
			{
				itemsCount++;

				KFileTreeNode* node = NULL;
				if (treeNode)
				{
					node = &treeNode->GetChildren().emplace_back(*this, item, parentNode);
					node->ComputeHash();
				}

				if (node && item.IsDirectory())
				{
					Recurse(item.GetFullPath(), node, node);
				}
			}
			item = finder.FindNext();
		}
		return itemsCount;
	};
	Recurse(GetLocation(KMM_LOCATION_MOD_FILES), &m_FileTree, NULL);
}

bool KModEntry::IsEnabled() const
{
	return m_IsEnabled && IsInstalled();
}
void KModEntry::SetEnabled(bool value)
{
	m_IsEnabled = value;
}
bool KModEntry::IsInstalled() const
{
	return m_FileTree.HasChildren();
}

KImageEnum KModEntry::GetIcon() const
{
	return KIMG_NONE;
}
intptr_t KModEntry::GetPriority() const
{
	if (IsEnabled())
	{
		int priority = 0;
		for (const KModEntry* entry: KModManager::Get().GetEntries())
		{
			if (entry == this)
			{
				break;
			}

			if (entry->IsEnabled())
			{
				priority++;
			}
		}
		return priority;
	}
	return -1;
}
intptr_t KModEntry::GetOrderIndex() const
{
	// x2 reserve space for priority groups
	return 2 * KModManager::Get().GetModOrderIndex(this);
}
wxString KModEntry::GetLocation(KModManagerLocation index) const
{
	switch (index)
	{
		case KMM_LOCATION_MOD_ROOT:
		{
			return KModManager::GetLocation(KMM_LOCATION_MOD_ROOT, m_Signature);
		}
		case KMM_LOCATION_MOD_INFO:
		{
			return GetLocation(KMM_LOCATION_MOD_ROOT) + wxS("\\Info.xml");
		}
		case KMM_LOCATION_MOD_FILES:
		case KMM_LOCATION_MOD_FILES_DEFAULT:
		{
			if (IsLinkedMod() && index != KMM_LOCATION_MOD_FILES_DEFAULT)
			{
				return m_LinkedModFilesPath;
			}
			else
			{
				return GetLocation(KMM_LOCATION_MOD_ROOT) + wxS("\\ModFiles");
			}
		}
		case KMM_LOCATION_MOD_LOGO:
		{
			return GetLocation(KMM_LOCATION_MOD_ROOT) + wxS("\\Image.img");
		}
		case KMM_LOCATION_MOD_DESCRIPTION:
		{
			return GetLocation(KMM_LOCATION_MOD_ROOT) + wxS("\\Description.txt");
		}
	};
	return KModManager::GetLocation(index, m_Signature);
}

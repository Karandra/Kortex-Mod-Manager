#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/ModProvider.hpp>
#include <Kortex/GameInstance.hpp>
#include "PackageProject/KPackageProject.h"
#include "Utility/KAux.h"
#include <KxFramework/KxCrypto.h>
#include <KxFramework/KxXML.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxFileFinder.h>

namespace
{
	using namespace Kortex;

	template<class T> void LoadOldSite(ModSourceStore& store, KxXMLNode& node, const wxString& attributeName, const wxString& providerName)
	{
		ModID modID = node.GetAttributeInt(attributeName, ModID::GetInvalidValue());
		if (modID.HasValue())
		{
			auto AddName = [&providerName](ModSourceItem& item)
			{
				if (!item.HasName())
				{
					item.SetName(providerName);
				}
			};

			if (T* provider = T::GetInstance())
			{
				ModSourceItem& item = store.AssignWith(*provider, modID);
				AddName(item);
			}
			else
			{
				ModSourceItem& item = store.AssignWith(providerName, modID);
				AddName(item);
			}
		}
	}
	void LoadOldSites(ModSourceStore& store, KxXMLNode& sitesNode)
	{
		LoadOldSite<NetworkManager::NexusProvider>(store, sitesNode, "NexusID", "Nexus");
		LoadOldSite<NetworkManager::LoversLabProvider>(store, sitesNode, "LoversLabID", "LoversLab");
		LoadOldSite<NetworkManager::TESALLProvider>(store, sitesNode, "TESALLID", "TESALL");

		// Load any "free" sites
		for (KxXMLNode node = sitesNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			store.TryAddItem(ModSourceItem(node.GetAttribute("Label"), node.GetValue()));
		}
	}
}

namespace Kortex::ModManager
{
	bool BasicGameMod::IsInstalledReal() const
	{
		return KxFile(GetModFilesDir()).IsFolderExist();
	}

	bool BasicGameMod::IsOK() const
	{
		return !m_ID.IsEmpty() && !m_Signature.IsEmpty();
	}

	bool BasicGameMod::LoadUsingSignature(const wxString& signature)
	{
		m_Signature = signature;

		if (!m_Signature.IsEmpty())
		{
			KxFileStream xmlStream(GetInfoFile(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
			KxXMLDocument xml(xmlStream);
			if (xml.IsOK())
			{
				KxXMLNode rootNode = xml.GetFirstChildElement("Mod");
				if (m_Signature == rootNode.GetAttribute("Signature"))
				{
					m_ID = rootNode.GetFirstChildElement("ID").GetValue();
					m_Name = rootNode.GetFirstChildElement("Name").GetValue();

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
							return false;
						}
					}
					if (m_Name.IsEmpty())
					{
						m_Name = m_ID;
					}

					m_Version = rootNode.GetFirstChildElement("Version").GetValue();
					m_Author = rootNode.GetFirstChildElement("Author").GetValue();

					// Color
					KxXMLNode colorNode = rootNode.GetFirstChildElement("Color");
					if (colorNode.IsOK())
					{
						int r = colorNode.GetAttributeInt("R", -1);
						int g = colorNode.GetAttributeInt("G", -1);
						int b = colorNode.GetAttributeInt("B", -1);
						if (r >= 0 && g >= 0 && b >= 0)
						{
							m_Color.Set(r, g, b, 225);
						}
					}

					// Tags
					KxXMLNode tagsNode = rootNode.GetFirstChildElement("Tags");
					m_PriorityGroupTag = tagsNode.GetAttribute("PriorityGroup");

					m_TagStore.Clear();
					for (KxXMLNode node = tagsNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
					{
						m_TagStore.AddTag(node.GetValue());
					}

					// Sites
					KxXMLNode oldSitesNode = rootNode.GetFirstChildElement("Sites");
					if (oldSitesNode.IsOK())
					{
						LoadOldSites(m_ProviderStore, oldSitesNode);
					}
					else
					{
						m_ProviderStore.LoadAssign(rootNode.GetFirstChildElement("Provider"));
					}

					// Time
					KxXMLNode timeNode = rootNode.GetFirstChildElement("Time");
					if (timeNode.IsOK())
					{
						auto ParseTime = [this, &timeNode](const wxString& name, wxDateTime& value)
						{
							KxXMLNode node = timeNode.GetFirstChildElement(name);
							if (node.IsOK())
							{
								value.ParseISOCombined(node.GetValue());
							}
						};
						ParseTime("Install", m_TimeInstall);
						ParseTime("Uninstall", m_TimeUninstall);
					}

					// Package file
					m_PackageFile = rootNode.GetFirstChildElement("InstallPackage").GetValue();

					// Linked mod config
					KxXMLNode linkedModNode = rootNode.GetFirstChildElement("LinkedMod");
					if (linkedModNode.IsOK())
					{
						m_LinkLocation = linkedModNode.GetAttribute("FolderPath");
					}

					return true;
				}
			}
		}
		return false;
	}
	bool BasicGameMod::LoadUsingID(const wxString& id)
	{
		m_ID = id;
		m_Signature = GetSignatureFromID(id);

		return LoadUsingSignature(m_Signature);
	}
	bool BasicGameMod::CreateFromProject(const KPackageProject& config)
	{
		const KPackageProjectInfo& info = config.GetInfo();

		SetID(config.GetModID());
		m_Name = config.GetModName();
		m_Author = info.GetAuthor();
		m_Version = info.GetVersion();
		m_Description = info.GetDescription();
		m_IsDescriptionChanged = true;

		m_TagStore = info.GetTagStore();
		m_ProviderStore = info.GetProviderStore();
		return true;
	}
	
	void BasicGameMod::CreateAllFolders()
	{
		KxFile(GetRootDir()).CreateFolder();
		if (!IsLinkedMod())
		{
			KxFile(GetModFilesDir()).CreateFolder();
		}
	}
	bool BasicGameMod::Save()
	{
		// Mod root is always needed here but other folders isn't
		KxFile(GetRootDir()).CreateFolder();

		KxFileStream stream(GetInfoFile(), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways);
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
			auto SaveLabeledValueArray = [&xml](KxXMLNode& node, const KLabeledValue::Vector& array, const wxString& name)
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

			// Signature
			KxXMLNode rootNode = xml.NewElement("Mod");
			rootNode.SetAttribute("Signature", m_Signature);

			// Generic info
			rootNode.NewElement("ID").SetValue(m_ID);
			rootNode.NewElement("Name").SetValue(GetName()); // Field 'm_Name' can be empty and GetName() returns 'm_ID' in this case
			rootNode.NewElement("Version").SetValue(m_Version);
			rootNode.NewElement("Author").SetValue(m_Author);

			// Color
			if (m_Color.IsOk())
			{
				KxXMLNode colorNode = rootNode.NewElement("Color");
				colorNode.SetAttribute("R", m_Color.GetR());
				colorNode.SetAttribute("G", m_Color.GetG());
				colorNode.SetAttribute("B", m_Color.GetB());
			}

			// Tags
			KxXMLNode tagsNode = rootNode.NewElement("Tags");
			if (!m_PriorityGroupTag.IsEmpty())
			{
				tagsNode.SetAttribute("PriorityGroup", m_PriorityGroupTag);
			}
			m_TagStore.Visit([&tagsNode](const IModTag& tag)
			{
				tagsNode.NewElement("Entry").SetValue(tag.GetID());
				return true;
			});

			// Sites
			if (!m_ProviderStore.IsEmpty())
			{
				KxXMLNode providersNode = rootNode.NewElement("Provider");
				m_ProviderStore.Save(providersNode);
			}

			// Time
			KxXMLNode timeNode = rootNode.NewElement("Time");
			auto SaveTime = [this, &timeNode](const wxString& name, const wxDateTime& value)
			{
				if (value.IsValid())
				{
					timeNode.NewElement(name).SetValue(value.FormatISOCombined());
				}
			};
			SaveTime("Install", m_TimeInstall);
			SaveTime("Uninstall", m_TimeUninstall);

			// Package
			rootNode.NewElement("InstallPackage").SetValue(m_PackageFile);

			// Description
			if (IsDescriptionChanged())
			{
				KxFileStream descriptionStream(GetDescriptionFile(), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways);
				descriptionStream.WriteStringUTF8(m_Description);
				m_IsDescriptionChanged = false;
			}

			// Linked mod config
			if (IsLinkedMod())
			{
				KxXMLNode tLinkedModNode = rootNode.NewElement("LinkedMod");
				tLinkedModNode.SetAttribute("FolderPath", m_LinkLocation);
			}

			return xml.Save(stream);
		}
		return false;
	}
	
	wxString BasicGameMod::GetDescription() const
	{
		if (m_Description.IsEmpty() && !IsDescriptionChanged())
		{
			KxFileStream stream(GetDescriptionFile(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
			if (stream.IsOk())
			{
				m_Description = stream.ReadStringUTF8(stream.GetLength());
			}
		}
		return m_Description;
	}
	void BasicGameMod::SetDescription(const wxString& value)
	{
		m_Description = value;
		m_IsDescriptionChanged = true;
	}

	const FileTreeNode& BasicGameMod::GetFileTree() const
	{
		return m_FileTree;
	}
	void BasicGameMod::ClearFileTree()
	{
		m_FileTree.GetChildren().clear();
	}
	void BasicGameMod::UpdateFileTree()
	{
		ClearFileTree();

		auto BuildTreeBranch = [this](FileTreeNode::RefVector& directories, const wxString& path, FileTreeNode& treeNode, FileTreeNode* parentNode)
		{
			KxFileFinder finder(path, wxS("*"));
			KxFileItem item = finder.FindNext();
			while (item.IsOK())
			{
				if (item.IsNormalItem())
				{
					FileTreeNode& node = treeNode.GetChildren().emplace_back(*this, item, parentNode);
					node.ComputeHash();
				}
				item = finder.FindNext();
			}

			for (FileTreeNode& node: treeNode.GetChildren())
			{
				if (node.IsDirectory())
				{
					directories.emplace_back(&node);
				}
			}
		};

		// Build top level
		FileTreeNode::RefVector directories;
		BuildTreeBranch(directories, GetModFilesDir(), m_FileTree, nullptr);

		// Build subdirectories
		while (!directories.empty())
		{
			FileTreeNode::RefVector roundDirectories;
			roundDirectories.reserve(directories.size());

			for (FileTreeNode* node: directories)
			{
				BuildTreeBranch(roundDirectories, node->GetFullPath(), *node, node);
			}
			directories = std::move(roundDirectories);
		}
	}

	intptr_t BasicGameMod::GetPriority() const
	{
		intptr_t priority = 0;
		for (const auto& mod: IModManager::GetInstance()->GetMods())
		{
			if (mod.get() == this)
			{
				return priority;
			}
			priority++;
		}
		return -1;
	}
	intptr_t BasicGameMod::GetOrderIndex() const
	{
		return IGameMod::GetOrderIndex();
	}
	wxString BasicGameMod::GetModFilesDir() const
	{
		if (IsLinkedMod())
		{
			return m_LinkLocation;
		}
		return GetDefaultModFilesDir();
	}
}

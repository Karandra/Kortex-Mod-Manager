#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/GameInstance.hpp>
#include "Network/ModNetwork/Nexus.h"
#include "Network/ModNetwork/LoversLab.h"
#include "Network/ModNetwork/TESALL.h"
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

	template<class T> void LoadOldSite(ModSourceStore& store, KxXMLNode& node, const wxString& attributeName, const wxString& sourceName)
	{
		ModID modID = node.GetAttributeInt(attributeName, ModID::GetInvalidValue());
		if (modID.HasValue())
		{
			auto AddName = [&sourceName](ModSourceItem& item)
			{
				if (!item.HasName())
				{
					item.SetName(sourceName);
				}
			};

			if (T* modNetwork = T::GetInstance())
			{
				ModSourceItem& item = store.AssignWith(*modNetwork, modID);
				AddName(item);
			}
			else
			{
				ModSourceItem& item = store.AssignWith(sourceName, modID);
				AddName(item);
			}
		}
	}
	void LoadOldSites(ModSourceStore& store, KxXMLNode& sitesNode)
	{
		LoadOldSite<NetworkManager::NexusModNetwork>(store, sitesNode, "NexusID", "Nexus");
		LoadOldSite<NetworkManager::LoversLabModNetwork>(store, sitesNode, "LoversLabID", "LoversLab");
		LoadOldSite<NetworkManager::TESALLModNetwork>(store, sitesNode, "TESALLID", "TESALL");

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
						int a = colorNode.GetAttributeInt("A", -1);
						if (r >= 0 && g >= 0 && b >= 0 && a >= 0)
						{
							m_Color.Set(r, g, b, a);
						}
					}

					// Tags
					KxXMLNode tagsNode = rootNode.GetFirstChildElement("Tags");

					m_TagStore.Clear();
					for (KxXMLNode node = tagsNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
					{
						wxString tagID = node.GetValue();
						m_TagStore.AddTag(tagID);
						if (node.GetAttributeBool("Primary"))
						{
							m_TagStore.SetPrimaryTag(tagID);
						}
					}

					// TODO: remove on v2.0 release
					bool save = false;
					if (wxString priorityGroupTag = tagsNode.GetAttribute("PriorityGroup"); !priorityGroupTag.IsEmpty())
					{
						m_TagStore.SetPrimaryTag(priorityGroupTag);
						save = true;
					}

					// Sources
					if (KxXMLNode node = rootNode.GetFirstChildElement("Sites"); node.IsOK())
					{
						LoadOldSites(m_ModSourceStore, node);
					}
					else if (KxXMLNode node = rootNode.GetFirstChildElement("Provider"); node.IsOK())
					{
						m_ModSourceStore.LoadAssign(node);
					}
					else if (KxXMLNode node = rootNode.GetFirstChildElement("Source"); node.IsOK())
					{
						m_ModSourceStore.LoadAssign(node);
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

					if (save)
					{
						Save();
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
	bool BasicGameMod::CreateFromProject(const ModPackageProject& config)
	{
		const PackageProject::InfoSection& info = config.GetInfo();

		SetID(config.GetModID());
		m_Name = config.GetModName();
		m_Author = info.GetAuthor();
		m_Version = info.GetVersion();
		m_Description = info.GetDescription();
		m_IsDescriptionChanged = true;

		m_TagStore = info.GetTagStore();
		m_ModSourceStore = info.GetModSourceStore();
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
		CreateAllFolders();

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
				colorNode.SetAttribute("A", m_Color.GetA());
			}

			// Tags
			KxXMLNode tagsNode = rootNode.NewElement("Tags");

			const IModTag* primaryTag = m_TagStore.GetPrimaryTag();
			m_TagStore.Visit([&tagsNode, primaryTagID = primaryTag ? primaryTag->GetID() : wxEmptyString](const IModTag& tag)
			{
				KxXMLNode node = tagsNode.NewElement("Item");
				node.SetValue(tag.GetID());
				if (primaryTagID == tag.GetID())
				{
					node.SetAttribute("Primary", true);
				}
				return true;
			});

			// Source
			if (!m_ModSourceStore.IsEmpty())
			{
				KxXMLNode providersNode = rootNode.NewElement("Source");
				m_ModSourceStore.Save(providersNode);
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

	wxString BasicGameMod::GetModFilesDir() const
	{
		if (IsLinkedMod())
		{
			return m_LinkLocation;
		}
		return GetDefaultModFilesDir();
	}
}

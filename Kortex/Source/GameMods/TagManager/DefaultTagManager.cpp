#include "stdafx.h"
#include "DefaultTagManager.h"
#include "DefaultTag.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/PackageManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxXML.h>

namespace Kortex::Application::OName
{
	KortexDefOption(DefaultTags);
	KortexDefOption(UserTags);
}

namespace Kortex::ModTagManager
{
	void DefaultTagManager::OnProfileSelected(ProfileEvent& event)
	{
		if (IGameProfile* oldProfile = event.GetPreviousProfile())
		{
			SaveUserTags(*oldProfile);
			oldProfile->SaveConfig();
		}

		ClearUserTags();
		if (IGameProfile* newProfile = event.GetProfile())
		{
			// Load tags from newly selected profile
			LoadUserTags(*newProfile);
		}

		// Reload tags from mods
		if (IModManager* modManager = IModManager::GetInstance())
		{
			for (IGameMod* mod: modManager->GetMods())
			{
				LoadTagsFromMod(m_UserTags, *mod);
			}
		}
	}

	void DefaultTagManager::LoadTagsFrom(IModTag::Vector& items, const KxXMLNode& tagsNode)
	{
		const bool hasSE = IPackageManager::GetInstance()->HasComponent<PackageManager::IWithScriptExtender>();

		for (KxXMLNode node = tagsNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			if (!hasSE && node.GetAttributeBool(wxS("RequiresScriptExtender")))
			{
				continue;
			}

			wxString id = node.GetAttribute(wxS("ID"));
			if (!id.IsEmpty())
			{
				// If tag exist, it's probably from mod, remove it,
				// we are going to add more complete tag definition right now.
				if (IModTag* existingTag = FindTagByID(items, id))
				{
					RemoveTag(items, *existingTag);
				}

				// Name
				auto name = IModTag::GetTranslatedNameByID(id);
				if (!name)
				{
					name = node.GetFirstChildElement(wxS("Name")).GetValue(id);
				}

				auto tag = NewTag();
				tag->SetID(id);
				tag->SetName(*name);
				tag->SetExpanded(node.GetFirstChildElement(wxS("Expanded")).GetValueBool());
				
				INexusModTag* nexusTag = nullptr;
				if (tag->QueryInterface(nexusTag))
				{
					nexusTag->SetNexusID(node.GetFirstChildElement(wxS("NexusID")).GetValueInt(INexusModTag::InvalidNexusID));
				}

				// Color
				KxXMLNode colorNode = node.GetFirstChildElement("Color");
				if (colorNode.IsOK())
				{
					auto CheckColorValue = [](int value)
					{
						return value >= 0 && value <= 255;
					};

					int r = colorNode.GetAttributeInt(wxS("R"), -1);
					int g = colorNode.GetAttributeInt(wxS("G"), -1);
					int b = colorNode.GetAttributeInt(wxS("B"), -1);
					if (CheckColorValue(r) && CheckColorValue(g) && CheckColorValue(b))
					{
						tag->SetColor(KxColor(r, g, b, 200));
					}
				}

				if (tag->IsOK())
				{
					EmplaceTag(items, std::move(tag));
				}
			}
		}
	}
	void DefaultTagManager::SaveTagsTo(const IModTag::Vector& items, KxXMLNode& tagsNode) const
	{
		tagsNode.ClearNode();
		for (const auto& tag: items)
		{
			if (tag->IsOK())
			{
				KxXMLNode node = tagsNode.NewElement(wxS("Item"));

				// ID
				const wxString id = tag->GetID();
				node.SetAttribute(wxS("ID"), id);

				// Expanded
				node.NewElement(wxS("Expanded")).SetValue(tag->IsExpanded());

				// Name
				const wxString name = tag->GetName();
				if (!tag->IsDefaultTag() && !name.IsEmpty() && name != id)
				{
					node.NewElement(wxS("Name")).SetValue(name);
				}

				// Nexus ID
				INexusModTag* nexusTag = nullptr;
				if (tag->QueryInterface(nexusTag) && nexusTag->HasNexusID())
				{
					node.NewElement(wxS("NexusID")).SetValue(nexusTag->GetNexusID());
				}

				// Color
				KxColor color = tag->GetColor();
				if (color.IsOk())
				{
					KxXMLNode colorNode = node.NewElement(wxS("Color"));
					colorNode.SetAttribute(wxS("R"), color.GetR());
					colorNode.SetAttribute(wxS("G"), color.GetG());
					colorNode.SetAttribute(wxS("B"), color.GetB());
				}
			}
		}
	}

	void DefaultTagManager::LoadUserTags(const IGameProfile& profile)
	{
		LoadTagsFrom(m_UserTags, profile.GetProfileOption(Application::OName::UserTags).GetNode());
	}
	void DefaultTagManager::SaveUserTags(IGameProfile& profile) const
	{
		SaveTagsTo(m_UserTags, profile.GetProfileOption(Application::OName::UserTags).GetNode());
	}
	void DefaultTagManager::ClearUserTags()
	{
		m_UserTags.clear();
	}

	void DefaultTagManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		LoadTagsFrom(m_DefaultTags, managerNode.GetFirstChildElement(Application::OName::DefaultTags));
	}
	void DefaultTagManager::OnInit()
	{
	}
	void DefaultTagManager::OnExit()
	{
		if (IGameProfile* profile = IGameProfile::GetActive())
		{
			SaveUserTags(*profile);
			profile->SaveConfig();
		}
	}

	void DefaultTagManager::OnInitialModsLoading()
	{
		if (IGameProfile* profile = IGameProfile::GetActive())
		{
			LoadUserTags(*profile);
		}
		m_BroadcastReciever.Bind(ProfileEvent::EvtSelected, &DefaultTagManager::OnProfileSelected, this);
	}
	void DefaultTagManager::LoadTagsFromMod(IModTag::Vector& items, const IGameMod& mod)
	{
		mod.GetTagStore().Visit([this, &items](const IModTag& tag)
		{
			if (const IModTag* defaultTag = FindTagByID(GetDefaultTags(), tag.GetID()))
			{
				EmplaceTag(items, defaultTag->Clone());
			}
			else
			{
				EmplaceTagWith(items, tag.GetID(), tag.GetName());
			}
			return true;
		});
	}
	std::unique_ptr<IModTag> DefaultTagManager::NewTag()
	{
		return std::make_unique<DefaultTag>();
	}
}

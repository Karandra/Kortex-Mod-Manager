#include "stdafx.h"
#include "DefaultTagManager.h"
#include "DefaultTag.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include "PackageManager/KPackageManager.h"
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxXML.h>

namespace Kortex::Application::OName
{
	KortexDefOption(DefaultTags);
	KortexDefOption(UserTags);
}

namespace
{
	using namespace Kortex;
	using namespace Kortex::ModTagManager;

	std::unique_ptr<DefaultTag> NewDefaultTag()
	{
		return std::make_unique<DefaultTag>();
	}
}

namespace Kortex::ModTagManager
{
	void DefaultTagManager::LoadTagsFrom(IModTag::Vector& items, const KxXMLNode& tagsNode)
	{
		const bool hasSE = KPackageManager::GetInstance()->HasScriptExtender();

		for (KxXMLNode node = tagsNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			
			if (!hasSE && node.GetAttributeBool(wxS("RequiresScriptExtender")))
			{
				continue;
			}
			wxString id = node.GetAttribute(wxS("ID"));

			// If tag exist, it's probably from mod, remove it,
			// we are going to add more complete tag definition right now.
			if (IModTag* existingTag = FindTagByID(items, id))
			{
				RemoveTag(items, *existingTag);
			}

			wxString label = node.GetFirstChildElement(wxS("Name")).GetValue(id);
			auto labelTranslated = Translation::TryGetString(wxS("ModManager.Tag.") + label);
			if (labelTranslated)
			{
				label = labelTranslated.value();
			}

			auto tag = NewDefaultTag();
			tag->SetID(id);
			tag->SetName(label);
			tag->SetNexusID(node.GetAttributeInt(wxS("NexusID"), INexusModTag::InvalidNexusID));
			tag->MarkAsSystem(labelTranslated.has_value());

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
			EmplaceTag(items, std::move(tag));
		}
	}
	void DefaultTagManager::SaveTagsTo(const IModTag::Vector& items, KxXMLNode& tagsNode) const
	{
		tagsNode.ClearNode();
		for (const auto& tag: items)
		{
			KxXMLNode node = tagsNode.NewElement(wxS("Entry"));
			node.SetAttribute(wxS("ID"), tag->GetID());

			if (!tag->IsSystemTag())
			{
				node.NewElement(wxS("Name")).SetValue(tag->GetName());
			}

			INexusModTag* nexusTag = nullptr;
			if (tag->QueryInterface(nexusTag) && nexusTag->HasNexusID())
			{
				node.SetAttribute(wxS("NexusID"), nexusTag->GetNexusID());
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

	void DefaultTagManager::LoadUserTags()
	{
		LoadTagsFrom(m_UserTags, GetAInstanceOption(Application::OName::UserTags).GetNode());
	}
	void DefaultTagManager::SaveUserTags() const
	{
		SaveTagsTo(m_UserTags, GetAInstanceOption(Application::OName::UserTags).GetNode());
	}

	void DefaultTagManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		LoadTagsFrom(m_DefaultTags, managerNode.GetFirstChildElement(Application::OName::DefaultTags));
	}
	void DefaultTagManager::OnInit()
	{
		LoadUserTags();
		if (m_UserTags.empty())
		{
			for (const auto& tag: m_DefaultTags)
			{
				m_UserTags.emplace_back(tag->Clone());
			}
		}
	}
	void DefaultTagManager::OnExit()
	{
		SaveUserTags();
	}

	void DefaultTagManager::LoadTagsFromMod(IModTag::Vector& items, const IGameMod& mod)
	{
		mod.GetTagStore().Visit([this, &items](const IModTag& tag)
		{
			EmplaceTagWith(items, tag.GetID(), tag.GetName());
			return true;
		});
	}
	std::unique_ptr<IModTag> DefaultTagManager::NewTag()
	{
		return NewDefaultTag();
	}
}

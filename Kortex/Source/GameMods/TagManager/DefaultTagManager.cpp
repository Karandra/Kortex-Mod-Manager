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
	void DefaultTagManager::LoadTagsFrom(IModTag::Vector& items, const KxXMLNode& tagsNode, bool markAsSystem)
	{
		const bool hasSE = KPackageManager::GetInstance()->HasScriptExtender();

		for (KxXMLNode node = tagsNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			if (!hasSE && node.GetAttributeBool(wxS("RequiresScriptExtender")))
			{
				continue;
			}

			wxString id = node.GetAttribute("ID");
			if (!FindTagByID(id))
			{
				wxString label = node.GetAttribute("Name", id);

				auto labelTranslated = Translation::TryGetString(wxS("ModManager.Tag.") + label);
				if (labelTranslated)
				{
					label = labelTranslated.value();
				}

				auto tag = NewDefaultTag();
				tag->SetID(id);
				tag->SetName(label);
				tag->SetNexusID(node.GetAttributeInt(wxS("NexusID"), INexusModTag::InvalidNexusID));
				tag->MarkAsSystem(markAsSystem);

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

				items.emplace_back(std::move(tag));
			}
		}
	}
	void DefaultTagManager::SaveTagsTo(const IModTag::Vector& items, KxXMLNode& tagsNode) const
	{
		tagsNode.ClearNode();
		for (const auto& tag: items)
		{
			KxXMLNode node = tagsNode.NewElement(wxS("Entry"));
			node.SetAttribute(wxS("ID"), tag->GetID());
			node.SetAttribute(wxS("Name"), (tag->IsSystemTag() || tag->GetID() == tag->GetName() ? tag->GetID() : tag->GetName()));

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
		LoadTagsFrom(m_UserTags, GetAInstanceOption(Application::OName::UserTags).GetNode(), false);
	}
	void DefaultTagManager::SaveUserTags() const
	{
		SaveTagsTo(m_UserTags, GetAInstanceOption(Application::OName::UserTags).GetNode());
	}

	void DefaultTagManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		LoadTagsFrom(m_DefaultTags, managerNode.GetFirstChildElement(Application::OName::DefaultTags), true);
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

	void DefaultTagManager::LoadTagsFromMod(const IGameMod& mod)
	{
		mod.GetTagStore().Visit([this](const IModTag& tag)
		{
			EmplaceTagWith(tag.GetID(), tag.GetName());
			return true;
		});
	}
	std::unique_ptr<IModTag> DefaultTagManager::NewTag()
	{
		return NewDefaultTag();
	}
}

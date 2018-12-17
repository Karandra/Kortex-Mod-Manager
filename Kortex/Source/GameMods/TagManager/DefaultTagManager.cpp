#include "stdafx.h"
#include "DefaultTagManager.h"
#include "DefaultTag.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include "PackageManager/KPackageManager.h"
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxXML.h>

namespace Kortex::ModTagManager
{
	void DefaultTagManager::LoadTagsFromFile(const wxString& filePath, bool markAsSystem)
	{
		KxFileStream stream(filePath, KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
		KxXMLDocument xml(stream);

		const bool hasSE = KPackageManager::GetInstance()->HasScriptExtender();
		for (KxXMLNode node = xml.GetFirstChildElement("Tags").GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			if (!hasSE && node.GetAttributeBool("RequiresScriptExtender"))
			{
				continue;
			}

			wxString id = node.GetAttribute("ID");
			if (!FindTagByID(id))
			{
				wxString label = node.GetAttribute("Name", id);

				auto labelTranslated = Translation::TryGetString(IApplication::GetInstance()->GetTranslation(), wxS("ModManager.Tag.") + label);
				if (labelTranslated)
				{
					label = labelTranslated.value();
				}

				auto tag = std::make_unique<DefaultTag>(id, label, markAsSystem);
				tag->SetNexusID(node.GetAttributeInt("NexusID", INexusModTag::InvalidNexusID));

				// Color
				KxXMLNode colorNode = node.GetFirstChildElement("Color");
				if (colorNode.IsOK())
				{
					auto CheckColorValue = [](int value)
					{
						return value >= 0 && value <= 255;
					};

					int r = colorNode.GetAttributeInt("R", -1);
					int g = colorNode.GetAttributeInt("G", -1);
					int b = colorNode.GetAttributeInt("B", -1);
					if (CheckColorValue(r) && CheckColorValue(g) && CheckColorValue(b))
					{
						tag->SetColor(KxColor(r, g, b, 200));
					}
				}

				EmplaceTag(std::move(tag));
			}
		}
	}
	void DefaultTagManager::SaveTagsToFile(const wxString& filePath) const
	{
		KxXMLDocument xml;
		KxXMLNode rootNode = xml.NewElement("Tags");

		for (const auto& tag: m_Tags)
		{
			KxXMLNode node = rootNode.NewElement("Entry");
			node.SetAttribute("ID", tag->GetID());
			node.SetAttribute("Name", (tag->IsSystemTag() || tag->GetID() == tag->GetName() ? tag->GetID() : tag->GetName()));

			// Color
			if (tag->HasColor())
			{
				KxXMLNode colorNode = node.NewElement("Color");

				KxColor color = tag->GetColor();
				colorNode.SetAttribute("R", color.GetR());
				colorNode.SetAttribute("G", color.GetG());
				colorNode.SetAttribute("B", color.GetB());
			}
		}

		KxFileStream stream(filePath, KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
		xml.Save(stream);
	}

	void DefaultTagManager::LoadDefaultTags()
	{
		LoadTagsFromFile(IApplication::GetInstance()->GetDataFolder() + wxS("\\ModManager\\DefaultTags.xml"), true);
	}
	void DefaultTagManager::LoadUserTags()
	{
		LoadTagsFromFile(IGameInstance::GetActive()->GetModTagsFile(), false);
	}
	void DefaultTagManager::SaveUserTags() const
	{
		SaveTagsToFile(IGameInstance::GetActive()->GetModTagsFile());
	}

	void DefaultTagManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
	}
	void DefaultTagManager::OnInit()
	{
		LoadUserTags();
		if (HasTags())
		{
			LoadDefaultTags();
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
		return std::make_unique<DefaultTag>();
	}
}

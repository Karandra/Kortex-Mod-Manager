#include "stdafx.h"
#include "IConfigManager.h"
#include "ConfigManger/Item.h"
#include "ConfigManger/ItemGroup.h"
#include "ConfigManger/Definition.h"
#include <Kortex/Common/GameConfig.hpp>
#include <Kortex/Application.hpp>

namespace Kortex
{
	namespace ConfigManager::Internal
	{
		const SimpleManagerInfo TypeInfo("ConfigManager", "ConfigManager.Name");
	}

	wxString IConfigManager::GetDefinitionFileByID(const wxString& id)
	{
		return IApplication::GetInstance()->GetDataFolder() + wxS("\\ConfigDefinitions\\") + id + wxS(".xml");
	}

	void IConfigManager::OnInit()
	{
		RefStackTranslator& translator = GetTranslatorStack();
		translator.Push(IApplication::GetInstance()->GetTranslation());

		if (LoadTranslation(m_Translation, "ConfigManager"))
		{
			translator.Push(m_Translation);
		}
	}
	void IConfigManager::OnExit()
	{
	}
	void IConfigManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
	}

	bool IConfigManager::LoadTranslation(KxTranslation& translation, const wxString& component)
	{
		const IApplication* app = IApplication::GetInstance();
		const KxTranslation& appTranslation = app->GetTranslation();
		const wxString locale = appTranslation.GetLocale();

		LoadTranslationStatus status = app->TryLoadTranslation(translation, app->GetAvailableTranslations(), component, locale);
		return status == LoadTranslationStatus::Success;
	}

	IConfigManager::IConfigManager()
		:ManagerWithTypeInfo(GameConfigModule::GetInstance())
	{
	}

	wxString IConfigManager::TranslateItemLabel(const wxString& name, const wxString& perfix) const
	{
		if (!name.IsEmpty())
		{
			auto value = GetTranslator().TryGetString(perfix + wxS('.') + name);
			if (value)
			{
				return *value;
			}
			return name;
		}
		return {};
	}
	wxString IConfigManager::TranslateItemLabel(const KxXMLNode& itemNode, const wxString& name, const wxString& perfix, bool isAttribute) const
	{
		auto [text, isTranslated] = TranslateItemElement(itemNode, isAttribute, wxS("Label"));
		if (!isTranslated)
		{
			text = TranslateItemLabel(text, perfix);
			if (!text.IsEmpty())
			{
				return text;
			}
			return TranslateItemLabel(name, perfix);
		}
		return text;
	}
	std::pair<wxString, bool> IConfigManager::TranslateItemElement(const KxXMLNode& itemNode, bool isAttribute, const wxString& attributeName) const
	{
		wxString text = isAttribute ? itemNode.GetAttribute(attributeName) : itemNode.GetValue();
		if (!text.IsEmpty())
		{
			// Strip translation variable
			if (text.StartsWith(wxS("$T(")) && text.EndsWith(wxS(")")))
			{
				text.Remove(0, 3);
				text.RemoveLast(1);
			}

			auto value = GetTranslator().TryGetString(text);
			if (value)
			{
				return {*value, true};
			}
		}
		return {text, false};
	}
}

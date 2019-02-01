#include "stdafx.h"
#include "IConfigManager.h"
#include <Kortex/Common/GameConfig.hpp>
#include <Kortex/Application.hpp>

namespace Kortex
{
	namespace ConfigManager::Internal
	{
		const SimpleManagerInfo TypeInfo("ConfigManager", "ConfigManager.Name");
	}

	void IConfigManager::OnInit()
	{
	}
	void IConfigManager::OnExit()
	{
	}
	void IConfigManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		RefStackTranslator& translator = GetTranslatorStack();
		translator.Push(IApplication::GetInstance()->GetTranslation());

		if (LoadTranslation(m_Translation, "ConfigManager"))
		{
			translator.Push(m_Translation);
		}
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

	wxString IConfigManager::LoadItemLabel(const KxXMLNode& itemNode, const wxString& name, const wxString& perfix) const
	{
		const ITranslator& translator = GetTranslator();

		wxString label = itemNode.GetAttribute(wxS("Label"));
		if (!label.IsEmpty())
		{
			auto value = translator.TryGetString(label);
			if (value)
			{
				return *value;
			}
		}

		auto value = translator.TryGetString(perfix + wxS('.') + name);
		if (value)
		{
			return *value;
		}
		return name;
	}
}

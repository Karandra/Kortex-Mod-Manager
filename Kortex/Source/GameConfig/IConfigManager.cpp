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
}

#pragma once
#include "stdafx.h"
#include <KxFramework/KxIndexedEnum.h>

namespace Kortex::Application::OptionDatabase
{
	template<class TDerived, class TEnum, class TString>
	using IndexDefinition = KxIndexedEnum::Definition<TDerived, TEnum, TString, true>;
}

namespace Kortex::Application::OptionDatabase
{
	enum class Options
	{
		Game,
		Instance,
		Language,
		Workspace,
	};
	class OptionsDef: public IndexDefinition<OptionsDef, Options, wxString>
	{
		inline static const TItem m_Index[] =
		{
			{Options::Game, wxS("Game")},
			{Options::Instance, wxS("Instance")},
			{Options::Language, wxS("Language")},
			{Options::Workspace, wxS("Workspace")},
		};
	};
}

namespace Kortex::Application::OptionDatabase
{
	enum class Language
	{
		Locale,
	};
	class LanguageDef: public IndexDefinition<LanguageDef, Language, wxString>
	{
		inline static const TItem m_Index[] =
		{
			{Language::Locale, wxS("Locale")},
		};
	};

	enum class Instance
	{
		ID,
		Location,
	};
	class InstanceDef: public IndexDefinition<InstanceDef, Instance, wxString>
	{
		inline static const TItem m_Index[] =
		{
			{Instance::ID, wxS("ID")},
			{Instance::Location, wxS("Location")},
		};
	};

	enum class Game
	{
		ID,
	};
	class GameDef: public IndexDefinition<GameDef, Game, wxString>
	{
		inline static const TItem m_Index[] =
		{
			{Game::ID, wxS("ID")},
		};
	};
}

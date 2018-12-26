#include "stdafx.h"
#include "IGameInstance.h"
#include "IGameProfile.h"
#include "ActiveGameInstance.h"
#include "Util.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFileFinder.h>

namespace
{
	using namespace Kortex;

	std::unique_ptr<IGameInstance> ms_ActiveInstance;
	IGameInstance::Vector ms_InstanceTemplates;
}
using namespace Util;

//////////////////////////////////////////////////////////////////////////
namespace Kortex
{
	bool IGameInstance::IsValidInstanceID(const wxString& id)
	{
		// Restrict max ID length to 64 symbols
		if (!id.IsEmpty() && id.Length() <= 64)
		{
			return !KAux::HasForbiddenFileNameChars(id);
		}
		return false;
	}
	bool IGameInstance::IsValidProfileID(const wxString& id)
	{
		// Same rules
		return IsValidInstanceID(id);
	}

	IGameInstance* IGameInstance::CreateActive(const IGameInstance& instanceTemplate, const wxString& instanceID)
	{
		IGameInstance& instance = AssignActive(std::make_unique<GameInstance::ActiveGameInstance>(instanceTemplate, instanceID));
		if (instance.InitInstance())
		{
			return &instance;
		}

		DestroyActive();
		return nullptr;
	}

	wxString IGameInstance::GetTemplatesFolder()
	{
		return IApplication::GetInstance()->GetDataFolder() + wxS("\\InstanceTemplates");
	}
	wxString IGameInstance::GetUserTemplatesFolder()
	{
		return IApplication::GetInstance()->GetUserSettingsFolder() + wxS("\\InstanceTemplates");
	}
	void IGameInstance::LoadTemplates()
	{
		ms_InstanceTemplates.clear();
		FindInstanceTemplates(GetTemplatesFolder(), true);
		FindInstanceTemplates(GetUserTemplatesFolder(), false);

		SortByOrder(ms_InstanceTemplates);
	}

	size_t IGameInstance::GetTemplatesCount()
	{
		return ms_InstanceTemplates.size();
	}
	IGameInstance::Vector& IGameInstance::GetTemplates()
	{
		return ms_InstanceTemplates;
	}
	IGameInstance* IGameInstance::GetTemplate(const GameID& id)
	{
		return FindObjectInVector<IGameInstance, FindBy::GameID>(ms_InstanceTemplates, id);
	}
	bool IGameInstance::HasTemplate(const GameID& id)
	{
		return GetTemplate(id) != nullptr;
	}

	void IGameInstance::FindInstanceTemplates(const wxString& path, bool isSystem)
	{
		KxFileFinder finder(path, wxS("*.xml"));
		KxFileItem item = finder.FindNext();
		while (item.IsOK())
		{
			if (item.IsFile() && item.IsNormalItem())
			{
				IGameInstance& instance = *ms_InstanceTemplates.emplace_back(std::make_unique<GameInstance::DefaultGameInstance>(item.GetFullPath(), wxEmptyString, isSystem));
				if (!instance.InitInstance())
				{
					ms_InstanceTemplates.pop_back();
				}
			}
			item = finder.FindNext();
		};
	}

	IGameInstance* IGameInstance::GetActive()
	{
		return ms_ActiveInstance.get();
	}
	IGameInstance& IGameInstance::AssignActive(std::unique_ptr<IGameInstance> instance)
	{
		ms_ActiveInstance = std::move(instance);
		return *ms_ActiveInstance;
	}
	void IGameInstance::DestroyActive()
	{
		ms_ActiveInstance.reset();
	}

	wxString IGameInstance::GetActiveProfileID()
	{
		if (const IGameProfile* profile = GetActiveProfile())
		{
			return profile->GetID();
		}
		return wxEmptyString;
	}
	IGameProfile* IGameInstance::GetActiveProfile()
	{
		if (IGameInstance::GetActive())
		{
			GameInstance::ActiveGameInstance* instance = static_cast<GameInstance::ActiveGameInstance*>(IGameInstance::GetActive());
			return instance->GetActiveProfile();
		}
		return nullptr;
	}
	bool IGameInstance::IsActiveProfileID(const wxString& id)
	{
		if (const IGameProfile* profile = GetActiveProfile())
		{
			return KxComparator::IsEqual(profile->GetID(), id, true);
		}
		return false;
	}
}

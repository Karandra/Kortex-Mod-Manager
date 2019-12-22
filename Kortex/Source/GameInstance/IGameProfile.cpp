#include "stdafx.h"
#include <Kortex/PluginManager.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/Application.hpp>
#include "IGameInstance.h"
#include "IGameProfile.h"
#include "Utility/Common.h"
#include <KxFramework/KxFileStream.h>

namespace
{
	namespace Util
	{
		using namespace Kortex;

		wxString GetGlobalRelativePath(const wxString& folderName)
		{
			return IGameInstance::GetActive()->GetInstanceRelativePath(wxS("GlobalProfile")) + wxS('\\') + folderName;
		}
		wxString GetLocalPath(const wxString& id)
		{
			return IGameInstance::GetActive()->GetProfilesDir() + wxS('\\') + id;
		}
		wxString GetLocalRelativePath(const wxString& id, const wxString& name)
		{
			return GetLocalPath(id) + wxS('\\') + name;
		}

		bool CreateLocalFolder(const wxString& id, const wxString& name)
		{
			return KxFile(GetLocalRelativePath(id, name)).CreateFolder();
		}
	}
}

namespace Kortex
{
	IGameProfile* IGameProfile::GetActive()
	{
		if (IGameInstance* instance = IGameInstance::GetActive())
		{
			return instance->GetActiveProfile();
		}
		return nullptr;
	}

	void IGameProfile::UpdateVariablesUsingActive(IVariableTable& variables)
	{
		variables.SetVariable(Variables::KVAR_GLOBAL_SAVES_DIR, Util::GetGlobalRelativePath(GameInstance::FolderName::Saves));
		variables.SetVariable(Variables::KVAR_GLOBAL_CONFIG_DIR, Util::GetGlobalRelativePath(GameInstance::FolderName::Config));
	}
	wxString IGameProfile::ProcessID(const wxString& id)
	{
		return Utility::MakeSafeFileName(id);
	}
	bool IGameProfile::CreateLocalFolder(const wxString& id, const wxString& name)
	{
		return Util::CreateLocalFolder(id, name);
	}

	wxString IGameProfile::GetConfigFile() const
	{
		return Util::GetLocalRelativePath(GetID(), wxS("Profile.xml"));
	}
	wxString IGameProfile::GetProfileDir() const
	{
		return Util::GetLocalPath(GetID());
	}
	wxString IGameProfile::GetProfileRelativePath(const wxString& name) const
	{
		return Util::GetLocalRelativePath(GetID(), name);
	}
	wxString IGameProfile::GetSavesDir() const
	{
		if (IsLocalSavesEnabled())
		{
			return GetProfileRelativePath(GameInstance::FolderName::Saves);
		}
		else
		{
			return Util::GetGlobalRelativePath(GameInstance::FolderName::Saves);
		}
	}
	wxString IGameProfile::GetConfigDir() const
	{
		if (IsLocalSavesEnabled())
		{
			return GetProfileRelativePath(GameInstance::FolderName::Config);
		}
		else
		{
			return Util::GetGlobalRelativePath(GameInstance::FolderName::Config);
		}
	}
	wxString IGameProfile::GetOverwritesDir() const
	{
		return GetProfileRelativePath(GameInstance::FolderName::Overwrites);
	}
}

namespace Kortex::GameInstance
{
	ProfileMod::ProfileMod(const wxString& signature, bool active, intptr_t priority)
		:m_Signature(signature), m_Priority(priority >= 0 ? priority : std::numeric_limits<intptr_t>::max()), m_IsActive(active)
	{
	}
	ProfileMod::ProfileMod(const IGameMod& mod, bool active)
		:m_Signature(mod.GetSignature()), m_Priority(mod.GetPriority()),  m_IsActive(active)
	{
	}

	IGameMod* ProfileMod::GetMod() const
	{
		if (IModManager* manager = IModManager::GetInstance())
		{
			return manager->FindModBySignature(m_Signature);
		}
		return nullptr;
	}
}

namespace Kortex::GameInstance
{
	ProfilePlugin::ProfilePlugin(const IGamePlugin& plugin, bool active)
		:m_Name(plugin.GetName()), m_Priority(plugin.GetPriority()), m_IsActive(active)
	{
	}
	ProfilePlugin::ProfilePlugin(const wxString& name, bool enabled, intptr_t priority)
		: m_Name(name), m_Priority(priority >= 0 ? priority : std::numeric_limits<intptr_t>::max()), m_IsActive(enabled)
	{
	}

	IGamePlugin* ProfilePlugin::GetPlugin() const
	{
		if (IPluginManager* manager = IPluginManager::GetInstance())
		{
			return manager->FindPluginByName(m_Name);
		}
		return nullptr;
	}
}

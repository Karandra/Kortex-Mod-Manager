#include "stdafx.h"
#include "IGameInstance.h"
#include "IGameProfile.h"
#include "ActiveGameInstance.h"
#include "DefaultGameInstance.h"
#include <Kortex/Application.hpp>
#include "Utility/KBitmapSize.h"
#include "Util.h"
#include <KxFramework/KxShell.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxComparator.h>

namespace
{
	using namespace Kortex;

	std::unique_ptr<IGameInstance> ms_ActiveInstance;
	IGameInstance::Vector ms_InstanceTemplates;
	IGameInstance::Vector ms_Instances;
}

namespace Kortex::GameInstance
{
	class TemplateLoader
	{
		public:
			void FindInstanceTemplates(const wxString& path, bool isSystem)
		{
			KxFileFinder finder(path, wxS("*.xml"));
			for (KxFileItem item = finder.FindNext(); item.IsOK(); item = finder.FindNext())
			{
				if (item.IsFile() && item.IsNormalItem())
				{
					IGameInstance& instance = *ms_InstanceTemplates.emplace_back(std::make_unique<GameInstance::DefaultGameInstance>(item.GetFullPath(), wxEmptyString, isSystem));
					if (!instance.InitInstance())
					{
						ms_InstanceTemplates.pop_back();
					}
				}
			}
		}
	};
}

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
	
	wxBitmap IGameInstance::GetGenericIcon()
	{
		return wxBitmap(GetGenericIconLocation(), wxBITMAP_TYPE_ANY);
	}
	wxString IGameInstance::GetGenericIconLocation()
	{
		return GetTemplatesFolder() + wxS("\\Icons\\Generic.ico");
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
	
	IGameInstance::Vector& IGameInstance::GetShallowInstances()
	{
		return ms_Instances;
	}
	IGameInstance* IGameInstance::GetShallowInstance(const wxString& instanceID)
	{
		return Util::FindObjectInVector<IGameInstance, Util::FindBy::InstanceID>(ms_Instances, instanceID);
	}
	IGameInstance* IGameInstance::NewShallowInstance(const wxString& instanceID)
	{
		if (GetShallowInstance(instanceID) == nullptr)
		{
			IGameInstance& instance = *ms_Instances.emplace_back(std::make_unique<GameInstance::ConfigurableGameInstance>(instanceID));
			Util::SortByInstanceID(ms_Instances);

			return &instance;
		}
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

		GameInstance::TemplateLoader loader;
		loader.FindInstanceTemplates(GetTemplatesFolder(), true);
		loader.FindInstanceTemplates(GetUserTemplatesFolder(), false);

		Util::SortByOrder(ms_InstanceTemplates);
	}
	void IGameInstance::LoadInstances()
	{
		ms_Instances.clear();

		KxFileFinder finder(IApplication::GetInstance()->GetInstancesFolder(), wxS("*"));
		for (KxFileItem item = finder.FindNext(); item.IsOK(); item = finder.FindNext())
		{
			if (item.IsDirectory() && item.IsNormalItem())
			{
				// Allow loading of invalid instances. They should be available so
				// instance selection window can report that and not just silently ignore them.
				IGameInstance& instance = *ms_Instances.emplace_back(std::make_unique<GameInstance::ConfigurableGameInstance>(item.GetName()));
				instance.InitInstance();
			}
		}
		Util::SortByOrder(ms_Instances);
	}

	size_t IGameInstance::GetTemplatesCount()
	{
		return ms_InstanceTemplates.size();
	}
	IGameInstance::Vector& IGameInstance::GetTemplates()
	{
		return ms_InstanceTemplates;
	}

	bool IGameInstance::IsActiveInstance() const
	{
		return ms_ActiveInstance &&
			!IsTemplate() &&
			ms_ActiveInstance->GetGameID() == GetGameID() &&
			ms_ActiveInstance->GetInstanceID() == GetInstanceID();
	}

	IGameInstance* IGameInstance::GetTemplate(const GameID& id)
	{
		return Util::FindObjectInVector<IGameInstance, Util::FindBy::GameID>(ms_InstanceTemplates, id);
	}
	bool IGameInstance::HasTemplate(const GameID& id)
	{
		return GetTemplate(id) != nullptr;
	}

	wxBitmap IGameInstance::LoadIcon(const wxString& path) const
	{
		wxBitmap bitmap(path, wxBITMAP_TYPE_ANY);
		if (bitmap.IsOk())
		{
			KBitmapSize size;
			size.FromSystemIcon();
			if (bitmap.GetWidth() != size.GetWidth() || bitmap.GetHeight() != size.GetHeight())
			{
				bitmap = size.ScaleBitmapAspect(bitmap);
			}
		}
		else
		{
			KxFileItem item;
			item.SetName(".exe");
			item.SetNormalAttributes();

			bitmap = KxShell::GetFileIcon(item, false);
		}
		return bitmap;
	}
	wxString IGameInstance::GetDefaultIconLocation() const
	{
		return KxString::Format(wxS("%1\\Icons\\%2.ico"), IGameInstance::GetTemplatesFolder(), GetGameID());
	}

	const IGameInstance& IGameInstance::GetTemplate() const
	{
		return *GetTemplate(GetGameID());
	}
	IGameInstance& IGameInstance::GetTemplate()
	{
		return *GetTemplate(GetGameID());
	}

	bool IGameInstance::Deploy(const IGameInstance* baseInstance, uint32_t copyOptions)
	{
		if (!IsDeployed())
		{
			KxFile(GetModsDir()).CreateFolder();
			KxFile(GetProfilesDir()).CreateFolder();

			if (baseInstance)
			{
				if (copyOptions & GameInstance::CopyOptionsInstance::Config)
				{
					KxFile(baseInstance->GetConfigFile()).CopyFile(GetConfigFile(), false);
				}
			}
			return true;
		}
		return false;
	}
	bool IGameInstance::IsDeployed() const
	{
		return GetShallowInstance(GetInstanceID()) != nullptr && KxFile(GetInstanceDir()).IsFolderExist();
	}
	bool IGameInstance::WithdrawDeploy()
	{
		if (IsDeployed())
		{
			// Move to recycle bin
			KxFile path(GetInstanceDir());
			path.RemoveFolderTree(true, true);
			path.RemoveFolder(true);

			Vector::const_iterator it;
			if (Util::FindObjectInVector<IGameInstance, Util::FindBy::InstanceID>(ms_Instances, GetInstanceID(), &it))
			{
				ms_Instances.erase(it);
			}
			return true;
		}
		return false;
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

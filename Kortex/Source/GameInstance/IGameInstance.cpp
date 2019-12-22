#include "stdafx.h"
#include "IGameInstance.h"
#include "IGameProfile.h"
#include "ActiveGameInstance.h"
#include "DefaultGameInstance.h"
#include <Kortex/Application.hpp>
#include "Application/SystemApplication.h"
#include "Utility/BitmapSize.h"
#include "Utility/Common.h"
#include "Util.h"
#include <KxFramework/KxShell.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxComparator.h>

namespace
{
	using namespace Kortex;
	wxBitmap LoadIconFromFile(const wxString& path)
	{
		wxBitmap bitmap(path, wxBITMAP_TYPE_ANY);
		if (bitmap.IsOk())
		{
			Utility::BitmapSize size;
			size.FromSystemIcon();
			if (bitmap.GetWidth() != size.GetWidth() || bitmap.GetHeight() != size.GetHeight())
			{
				bitmap = size.ScaleMaintainRatio(bitmap);
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
						auto& templates = SystemApplication::GetInstance()->GetGameInstanceTemplates();

						auto instance = std::make_unique<GameInstance::DefaultGameInstance>(item.GetFullPath(), wxEmptyString, isSystem);
						IGameInstance& ref = *templates.emplace_back(std::move(instance));
						if (!ref.InitInstance())
						{
							templates.pop_back();
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
			return !Utility::HasForbiddenFileNameChars(id);
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
		return LoadIconFromFile(GetGenericIconLocation());
	}
	wxString IGameInstance::GetGenericIconLocation()
	{
		return IApplication::GetInstance()->GetDataFolder() + wxS("\\UI\\kortex-logo-icon.ico");
	}

	IGameInstance* IGameInstance::CreateActive(const IGameInstance& instanceTemplate, const wxString& instanceID)
	{
		AssignActive(std::move(std::make_unique<GameInstance::ActiveGameInstance>(instanceTemplate, instanceID)));

		if (IGameInstance* instance = GetActive(); instance && instance->InitInstance())
		{
			return instance;
		}
		else
		{
			AssignActive(nullptr);
			return nullptr;
		}
	}
	IGameInstance* IGameInstance::GetActive()
	{
		return SystemApplication::GetInstance()->GetActiveGameInstance();
	}
	void IGameInstance::AssignActive(std::unique_ptr<IGameInstance> instance)
	{
		SystemApplication::GetInstance()->AssignActiveGameInstance(std::move(instance));
	}
	
	IGameInstance::Vector& IGameInstance::GetShallowInstances()
	{
		return SystemApplication::GetInstance()->GetShallowGameInstances();
	}
	IGameInstance* IGameInstance::GetShallowInstance(const wxString& instanceID)
	{
		return Util::FindObjectInVector<IGameInstance, Util::FindBy::InstanceID>(GetShallowInstances(), instanceID);
	}
	IGameInstance* IGameInstance::NewShallowInstance(const wxString& instanceID, const GameID& gameID)
	{
		if (GetShallowInstance(instanceID) == nullptr)
		{
			const IGameInstance* instanceTemplate = GetTemplate(gameID);
			if (instanceTemplate)
			{
				auto instance = std::make_unique<GameInstance::ConfigurableGameInstance>(*instanceTemplate, instanceID);

				auto& shallowInstances = GetShallowInstances();
				IGameInstance& ref = *shallowInstances.emplace_back(std::move(instance));
				Util::SortByInstanceID(shallowInstances);

				ref.InitInstance();
				return &ref;
			}
		}
		return nullptr;
	}

	wxString IGameInstance::GetGameDefinitionsFolder()
	{
		return IApplication::GetInstance()->GetDataFolder() + wxS("\\GameDefinitions");
	}
	wxString IGameInstance::GetUserGameDefinitionsFolder()
	{
		return IApplication::GetInstance()->GetUserSettingsFolder() + wxS("\\GameDefinitions");
	}
	
	void IGameInstance::LoadTemplates()
	{
		auto& templates = GetTemplates();
		templates.clear();

		GameInstance::TemplateLoader loader;
		loader.FindInstanceTemplates(GetGameDefinitionsFolder(), true);
		loader.FindInstanceTemplates(GetUserGameDefinitionsFolder(), false);

		Util::SortByOrder(templates);
	}
	void IGameInstance::LoadInstances()
	{
		auto& shallowInstances = GetShallowInstances();
		shallowInstances.clear();

		KxFileFinder finder(IApplication::GetInstance()->GetInstancesFolder(), wxS("*"));
		for (KxFileItem item = finder.FindNext(); item.IsOK(); item = finder.FindNext())
		{
			if (item.IsDirectory() && item.IsNormalItem())
			{
				// Allow loading of invalid instances. They should be available so
				// instance selection window can report that and not just silently ignore them.
				IGameInstance& instance = *shallowInstances.emplace_back(std::make_unique<GameInstance::ConfigurableGameInstance>(item.GetName()));
				instance.InitInstance();
			}
		}
		Util::SortByOrder(shallowInstances);
	}

	size_t IGameInstance::GetTemplatesCount()
	{
		return GetTemplates().size();
	}
	IGameInstance::Vector& IGameInstance::GetTemplates()
	{
		return SystemApplication::GetInstance()->GetGameInstanceTemplates();
	}

	bool IGameInstance::IsActiveInstance() const
	{
		const IGameInstance* active = GetActive();
		return active && !IsTemplate() && active->GetGameID() == GetGameID() && active->GetInstanceID() == GetInstanceID();
	}

	IGameInstance* IGameInstance::GetTemplate(const GameID& id)
	{
		return Util::FindObjectInVector<IGameInstance, Util::FindBy::GameID>(GetTemplates(), id);
	}
	bool IGameInstance::HasTemplate(const GameID& id)
	{
		return GetTemplate(id) != nullptr;
	}

	wxBitmap IGameInstance::LoadIcon(const wxString& path) const
	{
		return LoadIconFromFile(path);
	}
	wxString IGameInstance::GetDefaultIconLocation() const
	{
		return KxString::Format(wxS("%1\\Icons\\%2.ico"), IGameInstance::GetGameDefinitionsFolder(), GetGameID());
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

			// Should never fail since 'NewShallowInstance' function creates object that implement this interface
			IConfigurableGameInstance* instance = nullptr;
			if (QueryInterface(instance))
			{
				instance->SaveConfig();
			}

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
			auto& shallowInstances = GetShallowInstances();
			if (Util::FindObjectInVector<IGameInstance, Util::FindBy::InstanceID>(shallowInstances, GetInstanceID(), &it))
			{
				shallowInstances.erase(it);
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

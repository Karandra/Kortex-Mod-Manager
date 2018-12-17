#pragma once
#include "stdafx.h"
#include "IEvent.h"

namespace Kortex
{
	class IGameMod;
}

namespace Kortex::ModManager
{
	class ModEvent: public IEvent
	{
		public:
			using RefVector = std::vector<IGameMod*>;

		private:
			IGameMod* m_Mod = nullptr;
			RefVector m_ModVector;

		public:
			ModEvent(wxEventType type = wxEVT_NULL)
				:IEvent(type)
			{
			}
			ModEvent(wxEventType type, IGameMod& mod)
				:IEvent(type), m_Mod(&mod)
			{
			}
			ModEvent(wxEventType type, RefVector mods)
				:IEvent(type), m_ModVector(mods)
			{
			}
			ModEvent(wxEventType type, const wxString& id)
				:IEvent(type)
			{
				wxNotifyEvent::SetString(id);
			}

			ModEvent* Clone() const override
			{
				return new ModEvent(*this);
			}

		public:
			bool HasMod() const
			{
				return m_Mod != nullptr;
			}
			IGameMod* GetMod() const
			{
				return m_Mod;
			}
			wxString GetModID() const;

			bool HasModArray() const
			{
				return !m_ModVector.empty();
			}
			const RefVector& GetModsArray() const
			{
				return m_ModVector;
			}
	};
}

//////////////////////////////////////////////////////////////////////////
namespace Kortex::Events
{
	wxDECLARE_EVENT(ModInstalling, ModManager::ModEvent);
	wxDECLARE_EVENT(ModInstalled, ModManager::ModEvent);

	wxDECLARE_EVENT(ModUnsinstalling, ModManager::ModEvent);
	wxDECLARE_EVENT(ModUninstalled, ModManager::ModEvent);

	wxDECLARE_EVENT(ModToggled, ModManager::ModEvent);
	wxDECLARE_EVENT(ModChanged, ModManager::ModEvent);
	wxDECLARE_EVENT(ModFilesChanged, ModManager::ModEvent);

	wxDECLARE_EVENT(ModsReordered, ModManager::ModEvent);

	wxDECLARE_EVENT(ModVirtualTreeInvalidated, ModManager::ModEvent);
}

#pragma once
#include "stdafx.h"

namespace Kortex
{
	namespace PluginManager
	{
		class IPluginReader;
		class StdContentEntry;
	}
	
	class IGameMod;
	class IPluginManager;
	class IGamePlugin: public RTTI::IInterface<IGamePlugin>
	{
		friend class IPluginManager;

		public:
			using Vector = std::vector<std::unique_ptr<IGamePlugin>>;
			using RefVector = std::vector<IGamePlugin*>;

		protected:
			virtual void OnRead(PluginManager::IPluginReader& reader) = 0;

		public:
			virtual ~IGamePlugin() = default;

		public:
			virtual bool IsOK() const = 0;

			virtual wxString GetName() const = 0;
			virtual wxString GetFullPath() const = 0;

			virtual bool IsActive() const = 0;
			virtual void SetActive(bool isActive) = 0;
			bool CanToggleActive() const;

			virtual const IGameMod* GetOwningMod() const = 0;

			virtual const PluginManager::StdContentEntry* GetStdContentEntry() const = 0;
			bool IsStdContent() const
			{
				return GetStdContentEntry() != nullptr;
			}
	
			intptr_t GetOrderIndex() const;
			intptr_t GetPriority() const;
			intptr_t GetDisplayPriority() const;

			bool HasDependentPlugins() const;
			RefVector GetDependentPlugins() const;
	};
}

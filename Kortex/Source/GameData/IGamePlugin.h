#pragma once
#include "stdafx.h"

namespace Kortex
{
	namespace PluginManager
	{
		class IPluginReader;
		class StdContentItem;
	}
	
	class IGameMod;
	class IPluginManager;
	class IGamePlugin: public KxRTTI::Interface<IGamePlugin>
	{
		KxDecalreIID(IGamePlugin, {0x977a3069, 0xf8e8, 0x4f7e, {0xb4, 0xd1, 0xe0, 0x94, 0xaa, 0x91, 0x22, 0xbc}});

		friend class IPluginManager;

		public:
			using Vector = std::vector<std::unique_ptr<IGamePlugin>>;
			using RefVector = std::vector<IGamePlugin*>;

		private:
			mutable bool m_IsDataRead = false;

		protected:
			virtual void OnRead(PluginManager::IPluginReader& reader) = 0;
			void ReadDataIfNeeded() const;

		public:
			virtual ~IGamePlugin() = default;

		public:
			virtual bool IsOK() const = 0;

			virtual wxString GetName() const = 0;
			virtual wxString GetFullPath() const = 0;

			virtual bool IsActive() const = 0;
			virtual void SetActive(bool isActive) = 0;
			bool CanToggleActive() const;

			const IGameMod* GetOwningMod() const;
			virtual const PluginManager::StdContentItem* GetStdContentEntry() const = 0;
			bool IsStdContent() const
			{
				return GetStdContentEntry() != nullptr;
			}
	
			intptr_t GetOrderIndex() const;
			intptr_t GetPriority() const;
			intptr_t GetDisplayOrder() const;

			bool HasDependentPlugins() const;
			RefVector GetDependentPlugins() const;
	};
}

#pragma once
#include "Framework.hpp"
#include "IGameDefinition.h"
#include <kxf/General/StaticVariablesCollection.h>
#include <kxf/FileSystem/NativeFileSystem.h>
#include <kxf/Serialization/XML.h>

namespace Kortex
{
	class DefaultGameDefinition: public kxf::RTTI::ImplementInterface<DefaultGameDefinition, IGameDefinition, IEditableGameDefinition>
	{
		friend class DefaultApplication;

		private:
			kxf::ScopedNativeFileSystem m_RootFS;
			kxf::ScopedNativeFileSystem m_ResourcesFS;
			kxf::ScopedNativeFileSystem m_GameFS;
			kxf::XMLDocument m_DefitionData;

			GameID m_GameID;
			kxf::String m_GameName;
			kxf::String m_GameNameShort;
			mutable std::unique_ptr<kxf::IImage2D> m_Icon;
			int m_SortOrder = -1;

			kxf::StaticVariablesCollection m_Variables;

		private:
			void MakeNull();
			kxf::FSPath GetDefinitionFileName() const;

			bool LoadDefinition(const kxf::IFileSystem& rootFileSystem);
			bool LoadDefinitionData();
			void SetupVariables(const kxf::XMLNode& variablesRoot);

		public:
			DefaultGameDefinition() = default;

		public:
			// IGameDefinition
			bool IsNull() const override
			{
				return m_GameID.IsNull() || m_DefitionData.IsNull();
			}

			kxf::IVariablesCollection& GetVariables() override
			{
				return m_Variables;
			}
			kxf::String ExpandVariables(const kxf::String& variables) const override;
			kxf::String ExpandVariablesLocally(const kxf::String& variables) const override;

			int GetSortOrder() const override
			{
				return m_SortOrder;
			}
			GameID GetGameID() const override
			{
				return m_GameID;
			}
			kxf::String GetGameName() const override
			{
				if (!m_GameName.IsEmpty())
				{
					return m_GameName;
				}
				else if (!m_GameNameShort.IsEmpty())
				{
					return m_GameNameShort;
				}
				return m_GameID;
			}
			kxf::String GetGameShortName() const override
			{
				if (!m_GameNameShort.IsEmpty())
				{
					return m_GameNameShort;
				}
				else if (!m_GameName.IsEmpty())
				{
					return m_GameName;
				}
				return m_GameID;
			}
			const kxf::IImage2D& GetIcon() const override;
			
			using IGameDefinition::GetFileSystem;
			kxf::IFileSystem& GetFileSystem(Location locationID) override;

			// IEditableGameDefinition
			kxf::XMLDocument& GetDefinitionData() override
			{
				return m_DefitionData;
			}
			const kxf::XMLDocument& GetDefinitionData() const override
			{
				return m_DefitionData;
			}
			bool SaveDefinitionData() override;
	};
}

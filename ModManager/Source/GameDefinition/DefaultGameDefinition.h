#pragma once
#include "Framework.hpp"
#include "IGameDefinition.h"
#include <kxf/General/StaticVariablesCollection.h>
#include <kxf/FileSystem/NativeFileSystem.h>
#include <kxf/Serialization/XML.h>

namespace Kortex
{
	class DefaultGameDefinition: public IGameDefinition
	{
		private:
			kxf::ScopedNativeFileSystem m_RootFS;
			kxf::ScopedNativeFileSystem m_ResourcesFS;
			kxf::ScopedNativeFileSystem m_GameFS;
			kxf::XMLDocument m_DefitionData;

			kxf::String m_Name;
			kxf::String m_GameName;
			kxf::String m_GameNameShort;
			mutable std::unique_ptr<kxf::IImage2D> m_Icon;
			int m_SortOrder = -1;

			kxf::StaticVariablesCollection m_Variables;

		private:
			void MakeNull();

			bool LoadDefinition();
			void SetupVariables(const kxf::XMLNode& variablesRoot);

		public:
			DefaultGameDefinition() = default;

		public:
			// IGameDefinition
			bool IsNull() const override
			{
				return m_Name.IsEmpty() || m_DefitionData.IsNull();
			}

			kxf::XMLDocument& GetDefinitionData() override
			{
				return m_DefitionData;
			}
			const kxf::XMLDocument& GetDefinitionData() const override
			{
				return m_DefitionData;
			}
			bool LoadDefinitionData(const kxf::IFileSystem& fileSystem) override;
			bool SaveDefinitionData() override;

			kxf::IVariablesCollection& GetVariables() override
			{
				return m_Variables;
			}
			const kxf::IVariablesCollection& GetVariables() const override
			{
				return m_Variables;
			}
			kxf::String ExpandVariables(const kxf::String& variables) const override
			{
				return m_Variables.Expand(variables);
			}

			int GetSortOrder() const override
			{
				return m_SortOrder;
			}
			kxf::String GetName() const override
			{
				return m_Name;
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
				return m_Name;
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
				return m_Name;
			}
			const kxf::IImage2D& GetIcon() const override;
			
			kxf::IFileSystem& GetFileSystem(Location locationID) override;
			const kxf::IFileSystem& GetFileSystem(Location locationID) const override
			{
				return const_cast<DefaultGameDefinition&>(*this).DefaultGameDefinition::GetFileSystem(locationID);
			}
	};
}

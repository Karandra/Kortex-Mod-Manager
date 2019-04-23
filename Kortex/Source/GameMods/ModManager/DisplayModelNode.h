#pragma once
#include "stdafx.h"
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex
{
	class IGameMod;
	class IModTag;
}
namespace Kortex::ModManager
{
	class DisplayModel;

	class PriorityGroup;
	class FixedGameMod;
}

namespace Kortex::ModManager
{
	enum class DisplayModelColumnID
	{
		Name,
		Color,
		Priority,
		Version,
		Author,
		Tags,

		DateInstall,
		DateUninstall,
		ModFolder,
		PackagePath,
		Signature,

		MAX,

		ModSource = -1,
	};
}

namespace Kortex::ModManager
{
	class DisplayModelModNode: public KxRTTI::IExtendInterface<DisplayModelModNode, KxDataView2::Node>
	{
		friend class DisplayModel;

		public:
			using ColumnID = DisplayModelColumnID;

		private:
			IGameMod* m_Mod = nullptr;
			std::vector<DisplayModelModNode> m_Children;

		private:
			void OnAttachNode();

		public:
			DisplayModelModNode(IGameMod& mod)
				:m_Mod(&mod)
			{
			}
			
		public:
			KxDataView2::Editor* GetEditor(const KxDataView2::Column& column) const override;
			wxAny GetEditorValue(const KxDataView2::Column& column) const override;

			wxAny GetValue(const KxDataView2::Column& column) const override;
			wxAny GetValue(const KxDataView2::Column& column, const PriorityGroup& priorityGroup) const;
			wxAny GetValue(const KxDataView2::Column& column, const FixedGameMod& fixedGameMod) const;
			bool SetValue(const wxAny& value, KxDataView2::Column& column);

			bool Compare(const KxDataView2::Node& other, const KxDataView2::Column& column) const override;
			bool Compare(const IGameMod& left, const IGameMod& right, const KxDataView2::Column& column) const;

			bool GetAttributes(KxDataView2::CellAttributes& attributes, const KxDataView2::CellState& cellState, const KxDataView2::Column& column) const;
			bool IsCategoryNode() const override;
			int GetRowHeight() const override;

		public:
			IGameMod& GetMod() const
			{
				return *m_Mod;
			}
			DisplayModelModNode& AddModNode(IGameMod& mod)
			{
				return m_Children.emplace_back(mod);
			}

			DisplayModel& GetDisplayModel() const;
	};
}

namespace Kortex::ModManager
{
	class DisplayModelTagNode: public KxRTTI::IExtendInterface<DisplayModelTagNode, KxDataView2::Node>
	{
		friend class DisplayModel;

		public:
			using ColumnID = DisplayModelColumnID;

		private:
			IModTag* m_Tag = nullptr;
			std::vector<DisplayModelModNode> m_Children;

		private:
			void OnAttachNode();

		public:
			DisplayModelTagNode(IModTag& tag)
				:m_Tag(&tag)
			{
			}
			
		public:
			KxDataView2::Editor* GetEditor(const KxDataView2::Column& column) const override;
			wxAny GetValue(const KxDataView2::Column& column) const override;
			
			bool Compare(const KxDataView2::Node& other, const KxDataView2::Column& column) const override;
			bool Compare(const IModTag& left, const IModTag& right, const KxDataView2::Column& column) const;

			bool GetAttributes(KxDataView2::CellAttributes& attributes, const KxDataView2::CellState& cellState, const KxDataView2::Column& column) const;
			bool IsCategoryNode() const override;

		public:
			IModTag& GetTag() const
			{
				return *m_Tag;
			}

			DisplayModelModNode& AddModNode(IGameMod& mod)
			{
				return m_Children.emplace_back(mod);
			}
			bool HasMods() const
			{
				return !m_Children.empty();
			}
	};
}

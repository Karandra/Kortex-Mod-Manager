#pragma once
#include "stdafx.h"
#include "GameMods/IModTag.h"

namespace Kortex::ModTagManager
{
	class DefaultTag: public RTTI::IImplementation<IModTag, INexusModTag>
	{
		public:
			static std::optional<wxString> TryGetTranslatedName(const wxString& id);

		private:
			wxString m_ID;
			wxString m_Name;
			KxColor m_Color;
			int m_NexusID = -1;
			bool m_IsExpanded = false;

		public:
			DefaultTag() = default;
			DefaultTag(const wxString& id, const wxString& name = wxEmptyString)
				:m_ID(id), m_Name(m_Name)
			{
			}

		public:
			std::unique_ptr<IModTag> Clone() const override
			{
				return std::make_unique<DefaultTag>(*this);
			}
			
			bool IsExpanded() const
			{
				return m_IsExpanded;
			}
			void SetExpanded(bool isExpanded) override
			{
				m_IsExpanded = isExpanded;
			}

			wxString GetID() const override
			{
				return m_ID;
			}
			void SetID(const wxString& id) override
			{
				m_ID = id;
			}

			wxString GetName() const override;
			void SetName(const wxString& name) override;

			KxColor GetColor() const override
			{
				return m_Color;
			}
			void SetColor(const KxColor& color) override
			{
				m_Color = color;
			}

			int GetNexusID() const override
			{
				return m_NexusID;
			}
			void SetNexusID(int value) override
			{
				m_NexusID = value;
			}
	};
}

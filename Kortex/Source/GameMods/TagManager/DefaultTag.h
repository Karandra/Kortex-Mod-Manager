#pragma once
#include "stdafx.h"
#include "GameMods/IModTag.h"

namespace Kortex::ModTagManager
{
	class DefaultTagManager;

	class DefaultTag: public RTTI::IImplementation<IModTag, INexusModTag>
	{
		friend class DefaultTagManager;

		private:
			wxString m_ID;
			wxString m_Name;
			KxColor m_Color;
			int m_NexusID = -1;
			bool m_IsSystemTag = false;

		protected:
			void MarkAsSystem(bool value)
			{
				m_IsSystemTag = value;
			}

		public:
			DefaultTag() = default;
			DefaultTag(const wxString& id, const wxString& name = wxEmptyString, bool isSystemTag = false)
				:m_ID(id), m_Name(m_Name), m_IsSystemTag(isSystemTag)
			{
			}

		public:
			std::unique_ptr<IModTag> Clone() const override
			{
				return std::make_unique<DefaultTag>(*this);
			}
			bool IsSystemTag() const override
			{
				return m_IsSystemTag;
			}
			
			wxString GetID() const override
			{
				return m_ID;
			}
			void SetID(const wxString& id) override
			{
				m_ID = id;
			}

			wxString GetName() const override
			{
				return m_Name.IsEmpty() ? m_ID : m_Name;
			}
			void SetName(const wxString& name) override
			{
				m_Name = name;
			}

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

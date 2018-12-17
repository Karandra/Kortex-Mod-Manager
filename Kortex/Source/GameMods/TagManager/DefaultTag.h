#pragma once
#include "stdafx.h"
#include "GameMods/IModTag.h"

namespace Kortex::ModTagManager
{
	class DefaultTag: public RTTI::IImplementation<IModTag, INexusModTag>
	{
		private:
			wxString m_ID;
			wxString m_Name;
			KxColor m_Color;
			int m_NexusID = -1;
			bool m_IsSystemTag = false;

		public:
			DefaultTag() = default;
			DefaultTag(const wxString& id, const wxString& name = wxEmptyString, bool isSystemTag = false)
				:m_ID(id), m_Name(m_Name), m_IsSystemTag(isSystemTag)
			{
			}

		public:
			bool IsSystemTag() const
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

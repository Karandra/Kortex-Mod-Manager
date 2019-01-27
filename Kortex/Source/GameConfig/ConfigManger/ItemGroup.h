#pragma once
#include "stdafx.h"
#include "Common.h"
#include "ISource.h"
#include "ItemOptions.h"
class KxXMLNode;

namespace Kortex::GameConfig
{
	class ItemGroup
	{
		private:
			wxString m_ID;
			SourceTypeValue m_SourceType = SourceType::None;
			std::unique_ptr<ISource> m_Source;
			ItemOptions m_Options;

		public:
			ItemGroup(const wxString& id, const KxXMLNode& groupNode, const ItemOptions& parentOptions);

		public:
			bool IsOK() const
			{
				return !m_ID.IsEmpty() && m_Source != nullptr;
			}
			wxString GetID() const
			{
				return m_ID;
			}
			const ItemOptions& GetOptions() const
			{
				return m_Options;
			}

			const ISource& GetSource() const
			{
				return *m_Source;
			}
			ISource& GetSource()
			{
				return *m_Source;
			}

			bool OnLoadInstance(const KxXMLNode& groupNode);
	};
}

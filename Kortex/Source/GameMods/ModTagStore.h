#pragma once
#include "stdafx.h"

namespace Kortex
{
	class IModTag;

	class ModTagStore
	{
		private:
			std::unordered_set<wxString> m_TagIDs;

		public:
			using Visitor = std::function<bool(IModTag&)>;
			using CVisitor = std::function<bool(const IModTag&)>;

		public:
			bool HasTag(const wxString& tag) const
			{
				return m_TagIDs.find(tag) != m_TagIDs.end();
			}
			bool HasTag(const IModTag& tag) const;

			void AddTag(const wxString& tag)
			{
				m_TagIDs.insert(tag);
			}
			void AddTag(const IModTag& tag);

			void RemoveTag(const wxString& tag)
			{
				m_TagIDs.erase(tag);
			}
			void RemoveTag(const IModTag& tag);

			void ToggleTag(const IModTag& tag, bool addTag);
			void ToggleTag(const wxString& tag, bool addTag)
			{
				addTag ? AddTag(tag) : RemoveTag(tag);
			}
			
			size_t GetSize() const
			{
				return m_TagIDs.size();
			}
			size_t IsEmpty() const
			{
				return m_TagIDs.empty();
			}
			void Clear()
			{
				m_TagIDs.clear();
			}

			void Visit(const Visitor& visitor);
			void Visit(const CVisitor& visitor) const;

			KxStringVector GetIDs() const;
			KxStringVector GetNames() const;
	};
}

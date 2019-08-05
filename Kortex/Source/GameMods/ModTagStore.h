#pragma once
#include "stdafx.h"

namespace Kortex
{
	class IModTag;

	class ModTagStore
	{
		private:
			std::unordered_set<wxString> m_TagIDs;
			wxString m_PrimaryTag;

		public:
			using Visitor = std::function<bool(IModTag&)>;
			using CVisitor = std::function<bool(const IModTag&)>;

		public:
			bool HasTag(const wxString& tagID) const;
			bool HasTag(const IModTag& tag) const;

			void AddTag(const wxString& tagID);
			void AddTag(const IModTag& tag);

			void RemoveTag(const wxString& tagID);
			void RemoveTag(const IModTag& tag);

			void ToggleTag(const wxString& tagID, bool addTag);
			void ToggleTag(const IModTag& tagID, bool addTag);
			
			size_t GetSize() const
			{
				return m_TagIDs.size();
			}
			size_t IsEmpty() const
			{
				return m_TagIDs.empty();
			}
			void Clear();

			void Visit(const Visitor& visitor);
			void Visit(const CVisitor& visitor) const;

			IModTag* GetPrimaryTag() const;
			wxString GetPrimaryTagID() const;
			bool SetPrimaryTag(const IModTag& tag);
			bool SetPrimaryTag(const wxString& tagID);
			bool ClearPrimaryTag();

			KxStringVector GetIDs() const;
			KxStringVector GetNames() const;
	};
}

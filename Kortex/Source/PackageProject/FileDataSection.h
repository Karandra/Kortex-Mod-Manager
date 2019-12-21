#pragma once
#include "stdafx.h"
#include "ProjectSection.h"
#include <Kx/RTTI.hpp>

namespace Kortex::PackageProject
{
	class FileItem: public KxRTTI::Interface<FileItem>
	{
		KxDecalreIID(FileItem, {0x88e3075, 0xe7d5, 0x44af, {0x8d, 0xeb, 0xd, 0x71, 0xd7, 0x3c, 0x46, 0x5}});

		public:
			using Vector = std::vector<std::unique_ptr<FileItem>>;
			using RefVector = std::vector<FileItem*>;

		private:
			wxString m_ID;
			wxString m_Source;
			wxString m_Destination;
			int32_t m_Priority = -1;
			
		public:
			FileItem();
			virtual ~FileItem() = default;

		public:
			const wxString& GetID() const
			{
				return m_ID.IsEmpty() ? m_Source : m_ID;
			}
			void SetID(const wxString& id)
			{
				m_ID = id;
			}
			void MakeUniqueID();
			
			const wxString& GetSource() const
			{
				return m_Source;
			}
			void SetSource(const wxString& value)
			{
				m_Source = value;
			}
			
			const wxString& GetDestination() const
			{
				return m_Destination;
			}
			void SetDestination(const wxString& value)
			{
				m_Destination = value;
			}
			
			bool IsDefaultPriority() const;
			int32_t GetPriority() const;
			void SetPriority(int32_t value);
	};
}

namespace Kortex::PackageProject
{
	class FolderItem: public KxRTTI::ExtendInterface<FolderItem, FileItem>
	{
		KxDecalreIID(FolderItem, {0x6c8ec67, 0x9913, 0x404e, {0x97, 0xd4, 0x3c, 0xd3, 0x83, 0xfc, 0xd3, 0xd7}});

		private:
			std::vector<FileItem> m_Files;
			
		public:
			FolderItem() = default;
			~FolderItem() = default;

		public:
			std::vector<FileItem>& GetFiles()
			{
				return m_Files;
			}
			const std::vector<FileItem>& GetFiles() const
			{
				return m_Files;
			}
			
			FileItem& AddFile(FileItem item = {})
			{
				return m_Files.emplace_back(std::move(item));
			}
	};
}

namespace Kortex::PackageProject
{
	class FileDataSection: public ProjectSection
	{
		public:
			static const int ms_RequiredFilesPriority = std::numeric_limits<int>::min();
			static const int ms_DefaultPriority = -1;
			static const int ms_MinUserPriority = -1;
			static const int ms_MaxUserPriority = std::numeric_limits<int16_t>::max();
	
		public:
			static bool IsPriorityValid(int32_t value);
			static int32_t CorrectPriority(int32_t value);
			static bool IsFileIDValid(const wxString& id);
	
		private:
			FileItem::Vector m_Items;
	
		public:
			FileDataSection(ModPackageProject& project);
			~FileDataSection() = default;
	
		public:
			FileItem::Vector& GetItems()
			{
				return m_Items;
			}
			const FileItem::Vector& GetItems() const
			{
				return m_Items;
			}
	
			FileItem& AddFile(std::unique_ptr<FileItem> item)
			{
				return *m_Items.emplace_back(std::move(item));
			}
			FolderItem& AddFolder(std::unique_ptr<FolderItem> item)
			{
				FolderItem& ref = *item;
				m_Items.emplace_back(std::move(item));
				return ref;
			}
			
			FileItem* FindItemWithID(const wxString& id, size_t* index = nullptr) const;
			bool HasItemWithID(const wxString& id, const FileItem* ignoreThis = nullptr) const
			{
				FileItem* entry = FindItemWithID(id);
				return entry != nullptr && entry != ignoreThis;
			}
			bool IsUnusedID(const wxString& id) const
			{
				return IsFileIDValid(id) && !HasItemWithID(id);
			}
	};
}

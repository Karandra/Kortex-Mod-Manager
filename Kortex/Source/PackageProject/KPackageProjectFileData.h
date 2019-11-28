#pragma once
#include "stdafx.h"
#include "KPackageProjectPart.h"

namespace Kortex::PackageProject
{
	class FolderItem;
}

namespace Kortex::PackageProject
{
	class FileItem
	{
		public:
			using Vector = std::vector<std::unique_ptr<FileItem>>;
			using RefVector = std::vector<FileItem*>;

		private:
			wxString m_ID;
			wxString m_Source;
			wxString m_Destination;
			int32_t m_Priority;
	
		public:
			FileItem();
			virtual ~FileItem();
	
		public:
			virtual FolderItem* ToFolderItem()
			{
				return nullptr;
			}
			const FolderItem* ToFolderItem() const
			{
				return const_cast<FileItem&>(*this).ToFolderItem();
			}
			virtual FileItem* ToFileItem()
			{
				return this;
			}
			const FileItem* ToFileItem() const
			{
				return const_cast<FileItem&>(*this).ToFileItem();
			}
	
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
	class FolderItemElement
	{
		public:
			using Vector = std::vector<FolderItemElement>;

		private:
			wxString m_Source;
			wxString m_Destination;
	
		public:
			FolderItemElement()
			{
			}
			~FolderItemElement()
			{
			}
	
		public:
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
	};
}

namespace Kortex::PackageProject
{
	class FolderItem: public FileItem
	{
		private:
			FolderItemElement::Vector m_Files;
	
		public:
			FolderItem();
			~FolderItem();
	
		public:
			FolderItem* ToFolderItem() override
			{
				return this;
			}
	
		public:
			FolderItemElement::Vector& GetFiles()
			{
				return m_Files;
			}
			const FolderItemElement::Vector& GetFiles() const
			{
				return m_Files;
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
			FileItem::Vector m_Data;
	
		public:
			FileDataSection(ModPackageProject& project);
			virtual ~FileDataSection();
	
		public:
			FileItem::Vector& GetData()
			{
				return m_Data;
			}
			const FileItem::Vector& GetData() const
			{
				return m_Data;
			}
	
			FileItem* AddFile(FileItem* entry)
			{
				m_Data.push_back(std::unique_ptr<FileItem>(entry));
				return entry;
			}
			FolderItem* AddFolder(FolderItem* entry)
			{
				m_Data.push_back(std::unique_ptr<FolderItem>(entry));
				return entry;
			}
			
			FileItem* FindEntryWithID(const wxString& id, size_t* index = nullptr) const;
			bool HasEntryWithID(const wxString& id, const FileItem* ignoreThis = nullptr) const
			{
				FileItem* entry = FindEntryWithID(id);
				return entry != nullptr && entry != ignoreThis;
			}
			bool CanUseThisIDForNewEntry(const wxString& id) const
			{
				return IsFileIDValid(id) && !HasEntryWithID(id);
			}
	};
}

#pragma once 
#include "stdafx.h"
#include <KxFramework/KxIArchive.h>
#include <KxFramework/KxArchiveEvent.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxFileItem.h>

namespace Kortex::BethesdaArchiveNS
{
	enum class PropertyBool
	{
		ArchiveFlags,
		ContentFlags
	};
	enum class PropertyInt
	{
		Version
	};

	enum class ArchiveFlags: uint32_t
	{
		None = 0,
		IncludeDirectoryNames = 1 << 0,
		IncludeFileNames = 1 << 1,
		DefaultCompressed = 1 << 2,
		RetainEirectoryNames = 1 << 3,
		RetainFileNames = 1 << 4,
		RetainFileOffsets = 1 << 5,
		XBox360 = 1 << 6,
		RetainStringsRuringStartup = 1 << 7,
		EmbedFileNames = 1 << 8,
		CodecXMem = 1 << 9,

		REQUIRED = IncludeDirectoryNames|IncludeFileNames,
		CHANGEABLE = None,
		UNSUPPORTED_OBLIVION = XBox360,
		UNSUPPORTED_SKYRIM = XBox360|CodecXMem,
	};
	enum class ContentFlags: uint32_t
	{
		None = 0,
		Meshes = 1 << 0,
		Textures = 1 << 1,
		Menus = 1 << 2,
		Sounds = 1 << 3,
		Voices = 1 << 4,
		Shaders = 1 << 5,
		Trees = 1 << 6,
		Fonts = 1 << 7,
		Misc = 1 << 8,
		CTL = 1 << 9,
	};

	enum class Version: uint32_t
	{
		Oblivion = 103,
		Skyrim = 104,
		SkyrimSE = 105,
	};
	enum class Status
	{
		Success = 0,

		FIRST_GENRIC = 100,
		Unknown, // Unknown error
		Unsupported, // Unsupported format, compression, flags, whatever
		NoSuchFile, // No file with requested ID
		SuchFfolder, // Same, but for folder
		Compression, // Compression error
		Cancelled, // Operation aborted by user
		NoFilesToRxtract,

		/* Header errors */
		FIRST_HEADER = 200,
		Stream, // Stream internal error
		Signature, // Bad signature (not "BSA\000")
		Structure, // Invalid structure (currently bsaHeader::Offset != sizeof(bsaHeader))
		Version, // Only 104
		ArchiveFlags, // See bsaArchiveFlags::BSA_UNSUPPORTED_FLAGS

		/* File structure errors */
		FIRST_STRUCTURE = 300,
		FolderRecords,
		FileRecords,
		FileNames,
		FileData,
	};

	//////////////////////////////////////////////////////////////////////////
	constexpr char Signature[] = "BSA\0";
	struct Header
	{
		char Signature[4] = {};
		uint32_t Version = 0;
		uint32_t Offset = 0; // Should be sizeof(Header) in valid archive
		uint32_t ArchiveFlags = (uint32_t)ArchiveFlags::None;
		uint32_t FoldersCount = 0;
		uint32_t FilesCount = 0;
		uint32_t TotalFolderNameLength = 0;
		uint32_t TotalFileNameLength = 0;
		uint32_t FileFlags = (uint32_t)ContentFlags::None;
	};
	struct DirectoryRecord
	{
		uint64_t NameHash = 0;
		uint32_t Count = 0;
		uint32_t Offset = 0;
	};
	struct FileRecord
	{
		uint64_t NameHash = 0;
		uint32_t Size = 0;
		uint32_t Offset = 0;

		bool IsCompressed(const Header& header) const
		{
			// This is definitely not the way BSA stores its "compressed" status for files
			// Because this doesn't work at all
			// 
			// const uint32_t nCompressedMask = 1 << 30;
			// return (Size & nCompressedMask) && !(tHeader.ArchiveFlags & BSA_DEFAULT_COMPRESSED);
			return header.ArchiveFlags & (uint32_t)ArchiveFlags::DefaultCompressed;
		}
		void SetCompressed(Header& header, bool bCompressed)
		{
			const uint32_t compressedMask = 1 << 30;
			if (header.ArchiveFlags & (uint32_t)ArchiveFlags::DefaultCompressed)
			{
				KxUtility::ModFlagRef(Size, compressedMask, !bCompressed);
			}
			else
			{
				KxUtility::ModFlagRef(Size, compressedMask, bCompressed);
			}
		}
	};
	struct FileData
	{
		int64_t RawDataOffset = 0;
		uint32_t OriginalSize = 0;
		uint32_t CompressedSize = 0;
		size_t FileIndex = 0;

		double GetRatio() const
		{
			if (OriginalSize != 0)
			{
				return (double)CompressedSize / OriginalSize;
			}
			return -1;
		}
	};

	template<class TRecord>
	struct NamedRecord
	{
		TRecord Record;
		wxString Name;

		NamedRecord(TRecord&& record)
			:Record(std::forward<TRecord>(record))
		{
		}
	};

	template<class TRecord, class TData>
	struct NamedRecordWithData
	{
		TRecord Record;
		TData Data;
		wxString Name;

		NamedRecordWithData(TRecord&& record)
			:Record(std::forward<TRecord>(record))
		{
		}
	};
}

namespace Kortex
{
	class BethesdaArchive:
		public wxEvtHandler,
		public KxArchive::IArchive,
		public KxArchive::IArchiveItems,
		public KxArchive::IArchiveSearch,
		public KxArchive::IArchiveExtraction
	{
		public:
			using FileIndex = KxArchive::FileIndex;
			using FileIndexVector = KxArchive::FileIndexVector;

		public:
			using ArchiveFlags = BethesdaArchiveNS::ArchiveFlags;
			using ContentFlags = BethesdaArchiveNS::ContentFlags;
			using Version = BethesdaArchiveNS::Version;
			using Status = BethesdaArchiveNS::Status;

			using Header = BethesdaArchiveNS::Header;
			using DirectoryRecord = BethesdaArchiveNS::DirectoryRecord;
			using FileRecord = BethesdaArchiveNS::FileRecord;
			using FileData = BethesdaArchiveNS::FileData;

			using DirectoryItem = BethesdaArchiveNS::NamedRecord<DirectoryRecord>;
			using FileItem = BethesdaArchiveNS::NamedRecordWithData<FileRecord, FileData>;

		public:
			static uint64_t HashFilePath(const wxString& sourcePath, bool isFolderPath, wxString* correctedPath = nullptr);

		private:
			KxFileStream m_Stream;
			Header m_Header;
			std::vector<DirectoryItem> m_DirectoryRecods;
			std::vector<FileItem> m_FileRecords;

			int64_t m_OriginalSize = -1;
			Status m_Status = Status::Unknown;
			bool m_IsHeaderOK = false;

		private:
			Status IsHeaderOK() const;
			bool IsFlagsSupported() const;

			bool ReadHeader();
			bool ReadDirectoryRecods();
			bool ReadFileRecords();
			bool ReadFileNamesRecords();
			bool ReadFileDataRecords();

			const DirectoryItem* GetDirectoryRecord(size_t index) const
			{
				if (index < m_DirectoryRecods.size())
				{
					return &m_DirectoryRecods[index];
				}
				return nullptr;
			}
			const FileItem* GetFileRecord(size_t index) const
			{
				if (index < m_FileRecords.size())
				{
					return &m_FileRecords[index];
				}
				return nullptr;
			}

			bool OpenArchive(const wxString& filePath);
			void CloseArchive();

		public:
			BethesdaArchive();
			BethesdaArchive(const wxString& filePath);
			virtual ~BethesdaArchive();

		public:
			Status GetStatus() const
			{
				return m_Status;
			}

			// Is archive can be opened even if some errors occurred?
			bool CanIgnoreErrors() const
			{
				return m_IsHeaderOK;
			}

			uint32_t GetArchiveFlags() const
			{
				return m_Header.ArchiveFlags;
			}
			uint32_t GetContentFlags() const
			{
				return m_Header.FileFlags;
			}
			uint32_t GetArchiveVersion() const
			{
				return m_Header.Version;
			}

			size_t GetDirectoriesCount() const
			{
				return m_Header.FoldersCount;
			}
			size_t GetFilesCount() const
			{
				return m_Header.FilesCount;
			}
			size_t GetFilesCountInDirectory(size_t folderIndex) const
			{
				if (const DirectoryItem* item = GetDirectoryRecord(folderIndex))
				{
					return item->Record.Count;
				}
				return (size_t)-1;
			}

			// KxIArchive
			bool IsOK() const override;
			bool Open(const wxString& filePath) override;
			void Close() override;
			wxString GetFilePath() const override;

			int64_t GetOriginalSize() const override;
			int64_t GetCompressedSize() const override;

		public:
			// IArchiveItems
			size_t GetItemCount() const override;
			KxFileItem GetItem(size_t fileIndex) const override;

		public:
			// KxIArchiveSearch
			void* FindFirstFile(const wxString& filter, KxFileItem& fileItem) const override;
			bool FindNextFile(void* handle, KxFileItem& item) const override;
			void FindClose(void* handle) const override;
	};
}

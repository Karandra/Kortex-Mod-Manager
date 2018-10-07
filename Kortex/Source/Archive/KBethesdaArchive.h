#pragma once 
#include "stdafx.h"
#include <KxFramework/KxIArchive.h>
#include <KxFramework/KxArchiveEvent.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxFileItem.h>

namespace KBethesdaArchiveNS
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
	static const char Signature[] = "BSA\000";
	struct Header
	{
		char Signature[4] = {'\000'};
		uint32_t Version = 0;
		uint32_t Offset = 0; // Should be sizeof(Header)
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

		float GetRatio() const
		{
			if (OriginalSize != 0)
			{
				return (float)CompressedSize/(float)OriginalSize;
			}
			return 0;
		}
	};

	template<class RecordT> struct NamedRecord
	{
		RecordT Record;
		wxString Name;

		NamedRecord(RecordT&& record)
			:Record(std::move(record))
		{
		}
	};
	template<class RecordT, class DataT> struct NamedRecordWithData
	{
		RecordT Record;
		DataT Data;
		wxString Name;

		NamedRecordWithData(RecordT&& record)
			:Record(std::move(record))
		{
		}
	};
}

class KBethesdaArchive:
	public wxEvtHandler,
	public KxIArchive,
	public KxIArchiveSearch,
	public KxIArchiveExtraction
{
	public:
		using IndexVector = KxIArchiveNS::IndexVector;
		using Buffer = KxIArchiveNS::Buffer;
		using BufferMap = KxIArchiveNS::BufferMap;

	public:
		using ArchiveFlags = KBethesdaArchiveNS::ArchiveFlags;
		using ContentFlags = KBethesdaArchiveNS::ContentFlags;
		using Version = KBethesdaArchiveNS::Version;
		using Status = KBethesdaArchiveNS::Status;

		using Header = KBethesdaArchiveNS::Header;
		using DirectoryRecord = KBethesdaArchiveNS::DirectoryRecord;
		using FileRecord = KBethesdaArchiveNS::FileRecord;
		using FileData = KBethesdaArchiveNS::FileData;

		using DirectoryItem = KBethesdaArchiveNS::NamedRecord<DirectoryRecord>;
		using FileItem = KBethesdaArchiveNS::NamedRecordWithData<FileRecord, FileData>;

	public:
		static uint64_t HashFilePath(const wxString& sourcePath, bool isFolderPath, wxString* correctedPath = NULL);

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
			return NULL;
		}
		const FileItem* GetFileRecord(size_t index) const
		{
			if (index < m_FileRecords.size())
			{
				return &m_FileRecords[index];
			}
			return NULL;
		}

		bool OpenArchive(const wxString& filePath);
		void CloseArchive();

	public:
		KBethesdaArchive();
		KBethesdaArchive(const wxString& filePath);
		virtual ~KBethesdaArchive();

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
		virtual bool IsOK() const override;
		virtual bool Open(const wxString& filePath) override;
		virtual void Close() override;
		virtual wxString GetFilePath() const override;

		virtual size_t GetItemCount() const override;
		virtual wxString GetItemName(size_t index) const override;
		virtual int64_t GetOriginalSize() const override;
		virtual int64_t GetCompressedSize() const override;

		// KxIArchiveSearch
		virtual void* FindFirstFile(const wxString& filter, KxFileItem& fileItem) const override;
		virtual bool FindNextFile(void* handle, KxFileItem& item) const override;
		virtual void FindClose(void* handle) const override;

		// KxIArchiveExtraction
	protected:
		virtual bool DoExtractAll(const wxString& directory) const override;
		virtual bool DoExtractToDirectory(const IndexVector& indexes, const wxString& directory) const override;
		virtual bool DoExtractToFiles(const IndexVector& indexes, const KxStringVector& filePaths) const override;
		virtual Buffer DoExtractToMemory(size_t index) const override;
		virtual BufferMap DoExtractToMemory(const IndexVector& indexes) const override;
};

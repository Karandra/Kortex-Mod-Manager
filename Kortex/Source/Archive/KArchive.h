#pragma once
#include "stdafx.h"
#include <KxFramework/KxArchiveEvent.h>
#undef FindFirstFile
#undef FindNextFile
class KArchiveImpl;

enum KArchiveFormat
{
	KARC_FORMAT_UNKNOWN = -1,
	KARC_FORMAT_7Z,
	KARC_FORMAT_ZIP,
	KARC_FORMAT_RAR,
	KARC_FORMAT_RAR5,
	KARC_FORMAT_GZIP,
	KARC_FORMAT_BZIP2,
	KARC_FORMAT_TAR,
	KARC_FORMAT_ISO,
	KARC_FORMAT_CAB,
	KARC_FORMAT_LZMA,
	KARC_FORMAT_LZMA86,
};
enum KArchiveMethod
{
	KARC_METHOD_LZMA,
	KARC_METHOD_LZMA2,
	KARC_METHOD_PPMD,
	KARC_METHOD_BZIP2,
};
typedef std::vector<uint8_t> KAcrhiveBuffer;
typedef std::unordered_map<size_t, KAcrhiveBuffer> KAcrhiveBufferMap;

class KArchive: public wxEvtHandler
{
	public:
		static wxString GetLibraryPath();
		static wxString GetLibraryVersion();
		static bool IsLibraryLoaded();
		static bool Init();
		static bool UnInit();

	private:
		KArchiveImpl* m_Impl = NULL;

	public:
		KArchive();
		KArchive(const wxString& filePath);
		virtual ~KArchive();

	public:
		bool IsOpened() const;
		bool Open(const wxString& filePath);
		void Close();

		const wxString& GetFilePath() const;
		wxString GetItemName(size_t index) const;
		size_t GetItemCount() const;
		int64_t GetOriginalSize() const;
		int64_t GetCompressedSize() const;
		float GetRatio() const;

		KArchiveFormat GetProperty_CompressionFormat() const;
		void SetProperty_CompressionFormat(KArchiveFormat value);

		int GetProperty_CompressionLevel() const;
		void SetProperty_CompressionLevel(int value);

		int GetProperty_DictionarySize() const;
		void SetProperty_DictionarySize(int value);

		KArchiveMethod GetProperty_CompressionMethod() const;
		void SetProperty_CompressionMethod(KArchiveMethod method);

		bool GetProperty_Solid() const;
		void SetProperty_Solid(bool bSolid);

		bool GetProperty_MultiThreaded() const;
		void SetProperty_MultiThreaded(bool bMT);

		size_t FindFirstFileIn(const wxString& folder, const wxString& pattern) const;
		size_t FindNextFileIn(size_t index, const wxString& folder, const wxString& pattern) const;
		size_t FindFirstFile(const wxString& pattern, bool fileNameOnly = true) const;
		size_t FindNextFile(size_t index, const wxString& pattern, bool fileNameOnly = true) const;

		bool ExtractAll(const wxString& folderPath) const;
		bool Extract(const wxString& folderPath, const KxUInt32Vector& indexes) const;
		bool Extract(const KxUInt32Vector& indexes, const KxStringVector& tFinalPaths) const;
		KAcrhiveBuffer Extract(uint32_t index) const;
		KAcrhiveBufferMap Extract(const KxUInt32Vector& indexes) const;

		bool CompressFiles(const wxString& directory, const wxString& sSearchFilter, bool recursive = true);
		bool CompressAllFiles(const wxString& directory, bool recursive = true);
		bool CompressDirectory(const wxString& directory, bool recursive = true);
		bool CompressSpecifiedFiles(const KxStringVector& sourceFiles, const KxStringVector& archivePaths);
		bool CompressFile(const wxString& filePath);
		bool CompressFile(const wxString& filePath, const wxString& archivePath);
};

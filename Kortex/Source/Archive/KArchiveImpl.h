#pragma once
#include "stdafx.h"
#include <7zpp/7zpp.h>
#include "KArchive.h"
#undef CreateEvent

class KAcrhiveImplNotifier: public SevenZip::ProgressNotifier
{
	using TCharType = SevenZip::TCharType;

	private:
		wxEvtHandler* m_EvtHandler;
		int64_t m_LastMajor = 0;
		int64_t m_TotalSize = 0;
		wxString m_CurrentFile;
		bool m_StopNeeded = false;

	private:
		void SetCurrentFile(const TCharType* filePath);
		KxArchiveEvent CreateEvent(wxEventType type = KxEVT_ARCHIVE) const;

	public:
		KAcrhiveImplNotifier(wxEvtHandler* eventHandler);

	public:
		virtual bool Stop() override;
		virtual void OnStartWithTotal(const TCharType* filePath, int64_t totalBytes) override;
		virtual void OnMajorProgress(const TCharType* filePath, int64_t bytesCompleted) override;
		virtual void OnMinorProgress(const TCharType* filePath, int64_t bytesCompleted, int64_t totalBytes) override;
		virtual void OnDone(const TCharType* filePath) override;
};

class KArchiveImpl
{
	using TCharType = SevenZip::TCharType;
	using TString = SevenZip::TString;
	using TStringVector = SevenZip::TStringVector;

	private:
		static SevenZip::SevenZipLibrary ms_Library;

	public:
		static wxString GetLibraryPath();
		static SevenZip::SevenZipLibrary& GetLibrary()
		{
			return ms_Library;
		}
		static bool IsLibraryLoaded()
		{
			return ms_Library.IsLoaded();
		}
		static bool Init();
		static bool UnInit();

	private:
		typedef SevenZip::CompressionFormat::_Enum FormatEnum;

		wxEvtHandler* m_EvtHandler;
		KAcrhiveImplNotifier m_Notifier;
		SevenZip::SevenZipArchive m_Archive;
		const wxString m_ArchiveFilePath;
		int64_t m_OriginalSize = -1;

	private:
		KArchiveFormat ConvertFormat(FormatEnum type) const;
		FormatEnum ConvertFormat(KArchiveFormat type) const;
		void InvalidateCache();
		TStringVector RepackWXStringVector(const KxStringVector& array) const;

		size_t FindFileAt(size_t index, const wxString& pattern, bool fileNameOnly = true);
		size_t FindFileAtIn(size_t index, const wxString& pattern, const wxString& folder);

	public:
		KArchiveImpl(wxEvtHandler* eventHandler, const wxString& filePath);
		virtual ~KArchiveImpl();

	public:
		const wxString& GetFilePath()
		{
			return m_ArchiveFilePath;
		}
		wxString GetItemName(size_t index);
		size_t GetItemCount()
		{
			return m_Archive.GetNumberOfItems();
		}
		int64_t GetOriginalSize();
		int64_t GetCompressedSize();
		float GetRatio();

		KArchiveFormat GetProperty_CompressionFormat()
		{
			KArchiveFormat format = ConvertFormat(m_Archive.GetProperty_CompressionFormat());

			// This library can't detect 7z archive for some reason, but can perfectly read other data from it.
			// So check for original size validity and set format manually.
			if (format == KARC_FORMAT_UNKNOWN && GetOriginalSize() != -1)
			{
				format = KARC_FORMAT_7Z;
			}
			return format;
		}
		void SetProperty_CompressionFormat(KArchiveFormat value)
		{
			m_Archive.SetProperty_CompressionFormat(ConvertFormat(value));
		}

		int GetProperty_CompressionLevel() const
		{
			return m_Archive.GetProperty_CompressionLevel();
		}
		void SetProperty_CompressionLevel(int value)
		{
			m_Archive.SetProperty_CompressionLevel(value);
		}

		int GetProperty_DictionarySize() const
		{
			return m_Archive.GetProperty_DictionarySize();
		}
		void SetProperty_DictionarySize(int value)
		{
			m_Archive.SetProperty_DictionarySize(value);
		}

		KArchiveMethod GetProperty_CompressionMethod() const
		{
			return static_cast<KArchiveMethod>(m_Archive.GetProperty_CompressionMethod());
		}
		void SetProperty_CompressionMethod(KArchiveMethod nMethod)
		{
			m_Archive.SetProperty_CompressionMethod(static_cast<SevenZip::CompressionMethod>(nMethod));
		}

		bool GetProperty_Solid() const
		{
			return m_Archive.GetProperty_Solid();
		}
		void SetProperty_Solid(bool bSolid)
		{
			m_Archive.SetProperty_Solid(bSolid);
		}

		bool GetProperty_MultiThreaded() const
		{
			return m_Archive.GetProperty_MultiThreaded();
		}
		void SetProperty_MultiThreaded(bool bMT)
		{
			m_Archive.SetProperty_MultiThreaded(bMT);
		}

		size_t FindFirstFileIn(const wxString& folder, const wxString& pattern);
		size_t FindNextFileIn(size_t index, const wxString& folder, const wxString& pattern);
		size_t FindFirstArcFile(const wxString& pattern, bool fileNameOnly = true);
		size_t FindNextArcFile(size_t index, const wxString& pattern, bool fileNameOnly = true);

		bool ExtractAll(const wxString& folderPath);
		bool Extract(const wxString& folderPath, const KxUInt32Vector& indexes);
		bool Extract(const KxUInt32Vector& indexes, const KxStringVector& tFinalPaths);
		KAcrhiveBuffer Extract(uint32_t index);
		KAcrhiveBufferMap Extract(const KxUInt32Vector& indexes);

		bool CompressFiles(const wxString& directory, const wxString& sSearchFilter, bool recursive = true);
		bool CompressAllFiles(const wxString& directory, bool recursive = true);
		bool CompressDirectory(const wxString& directory, bool recursive = true);
		bool CompressSpecifiedFiles(const KxStringVector& sourceFiles, const KxStringVector& archivePaths);
		bool CompressFile(const wxString& filePath);
		bool CompressFile(const wxString& filePath, const wxString& archivePath);
};

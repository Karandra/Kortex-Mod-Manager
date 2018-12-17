#pragma once
#include "stdafx.h"
#include <KxFramework/KxIArchive.h>
#include <KxFramework/KxArchiveEvent.h>
#include <KxFramework/KxFileItem.h>

namespace SevenZip
{
	class SevenZipArchive;
	class SevenZipLibrary;
}

namespace KArchiveNS
{
	enum class PropertyBool
	{
		Solid,
		MultiThreaded,
	};
	enum class PropertyInt
	{
		CompressionLevel,
		DictionarySize,
		Format,
		Method,
	};

	enum Method
	{
		Unknown = -1,

		LZMA,
		LZMA2,
		PPMd,
		BZip2,
	};
	enum class Format
	{
		Unknown = -1,
		SevenZip,
		Zip,
		RAR,
		RAR5,
		GZip,
		BZip2,
		Tar,
		ISO,
		CAB,
		LZMA,
		LZMA86,
	};
}

class KArchive:
	public wxEvtHandler,

	public KxIArchive,
	public KxIArchiveSearch,
	public KxIArchiveExtraction,
	public KxIArchiveCompression,

	public KxIArchivePropertiesBool<KArchiveNS::PropertyBool>,
	public KxIArchivePropertiesInt<KArchiveNS::PropertyInt>
{
	public:
		using IndexVector = KxIArchiveNS::IndexVector;
		using Buffer = KxIArchiveNS::Buffer;
		using BufferMap = KxIArchiveNS::BufferMap;

	public:
		static wxString GetLibraryPath();
		static wxString GetLibraryVersion();

		static bool IsLibraryLoaded();
		static bool Init();
		static bool UnInit();

	private:
		SevenZip::SevenZipArchive* m_Impl = nullptr;
		wxString m_FilePath;
		
		int64_t m_CompressedSize = -1;
		mutable int64_t m_OriginalSize = -1;

	private:
		void OpenArchive(const wxString& filePath);
		void CloseArchive();

	public:
		KArchive();
		KArchive(const wxString& filePath);
		virtual ~KArchive();

	public:
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

		// KxIArchiveCompression
	protected:
		virtual bool DoCompressFiles(const wxString& directory, const wxString& searchFilter, bool recursive) override;
		virtual bool DoCompressDirectory(const wxString& directory, bool recursive) override;
		virtual bool DoCompressSpecifiedFiles(const KxStringVector& sourcePaths, const KxStringVector& archivePaths) override;
		virtual bool DoCompressFile(const wxString& sourcePath) override;
		virtual bool DoCompressFile(const wxString& sourcePath, const wxString& archivePath) override;

	public:
		// KxIArchivePropertiesBool
		virtual bool GetPropertyBool(BoolProperties property) const override;
		virtual void SetPropertyBool(BoolProperties property, bool value) override;

		// KxIArchivePropertiesInt
		virtual int GetPropertyInt(IntProperties property) const override;
		virtual void SetPropertyInt(IntProperties property, int value) override;
};

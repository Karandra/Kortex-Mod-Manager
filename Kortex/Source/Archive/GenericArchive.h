#pragma once
#include "stdafx.h"
#include <KxFramework/KxArchive.h>
#include <KxFramework/KxArchiveEvent.h>
#include <KxFramework/KxFileItem.h>
#include "Common.h"

namespace SevenZip
{
	class Archive;
	class Library;
	class ProgressNotifier;
}

namespace Kortex
{
	class GenericArchive:
		public wxEvtHandler,

		public KxRTTI::ImplementInterface
		<
			GenericArchive,
			KxArchive::IArchive,
			KxArchive::IArchiveItems,
			KxArchive::IArchiveSearch,
			KxArchive::IArchiveExtraction,
			KxArchive::IArchiveCompression,
			KxArchive::IArchiveProperties
		>
	{
		public:
			using FileIndex = KxArchive::FileIndex;
			using FileIndexVector = KxArchive::FileIndexVector;

		public:
			static wxString GetLibraryPath();
			static wxString GetLibraryVersion();

			static bool IsLibraryLoaded();
			static bool Init();
			static bool UnInit();

		private:
			std::unique_ptr<SevenZip::Archive> m_Archive;
			std::unique_ptr<SevenZip::ProgressNotifier> m_Notifier;

			mutable std::optional<int64_t> m_OriginalSize;
			mutable std::optional<int64_t> m_CompressedSize;

		private:
			void OpenArchive(const wxString& filePath);
			void CloseArchive();

		protected:
			bool OnDynamicBind(wxDynamicEventTableEntry& entry) override;

		public:
			GenericArchive();
			GenericArchive(const wxString& filePath);
			GenericArchive(const GenericArchive&) = delete;
			GenericArchive(GenericArchive&& other);
			virtual ~GenericArchive();

		public:
			// IArchive
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
			// IArchiveSearch
			void* FindFirstFile(const wxString& filter, KxFileItem& fileItem) const override;
			bool FindNextFile(void* handle, KxFileItem& item) const override;
			void FindClose(void* handle) const override;

		public:
			// IArchiveExtraction
			bool Extract(KxArchive::IExtractionCallback& callback) const override;
			bool Extract(KxArchive::IExtractionCallback& callback, KxArchive::FileIndexView files) const override;

		public:
			// IArchiveCompression
			bool CompressDirectory(const wxString& directory, bool recursive) override;

			bool CompressFiles(const wxString& directory, const wxString& searchFilter, bool recursive) override;
			bool CompressSpecifiedFiles(const KxStringVector& sourcePaths, const KxStringVector& archivePaths) override;

			bool CompressFile(const wxString& sourcePath) override;
			bool CompressFile(const wxString& sourcePath, const wxString& archivePath) override;

		public:
			// IArchiveProperties
			std::optional<bool> GetPropertyBool(wxStringView property) const override;
			bool SetPropertyBool(wxStringView property, bool value) override;

			std::optional<int64_t> GetPropertyInt(wxStringView property) const override;
			bool SetPropertyInt(wxStringView property, int64_t value)  override;

			std::optional<double> GetPropertyFloat(wxStringView property) const override;
			bool SetPropertyFloat(wxStringView property, double value) override;

			std::optional<wxString> GetPropertyString(wxStringView property) const override;
			bool SetPropertyString(wxStringView property, wxStringView value) override;

		public:
			GenericArchive& operator=(const GenericArchive&) = delete;
			GenericArchive& operator=(GenericArchive&& other);
	};
}

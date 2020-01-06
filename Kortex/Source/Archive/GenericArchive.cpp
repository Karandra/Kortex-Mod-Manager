#include "stdafx.h"
#include "GenericArchive.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxComparator.h>

namespace
{
	template<class... Args> auto FormatMessage(Args&&... arg)
	{
		return ::FormatMessageW(std::forward<Args>(arg)...);
	}
	template<class... Args> auto GetFullPathName(Args&&... arg)
	{
		return ::GetFullPathNameW(std::forward<Args>(arg)...);
	}
	template<class... Args> auto MoveFile(Args&&... arg)
	{
		return ::MoveFileW(std::forward<Args>(arg)...);
	}
	template<class... Args> auto DeleteFile(Args&&... arg)
	{
		return ::DeleteFileW(std::forward<Args>(arg)...);
	}
	template<class... Args> auto FindFirstFile(Args&&... arg)
	{
		return ::FindFirstFileW(std::forward<Args>(arg)...);
	}
	template<class... Args> auto CreateEvent(Args&&... arg)
	{
		return ::CreateEventW(std::forward<Args>(arg)...);
	}
}

#pragma push_macro("NULL")
#undef NULL
#define NULL nullptr

#include <7zpp/7zpp.h>
#include <7zpp/7zppEx.h>
#include <7zip\Archive\IArchive.h>
#pragma comment(lib, "7zpp_u.lib")

#pragma pop_macro("NULL")

namespace
{
	SevenZip::Library g_ArchiveLibrary;

	SevenZip::TString ToTString(const wxString& value)
	{
		return SevenZip::TString(value.wc_str(), value.length());
	}
	SevenZip::TStringView ToTStringView(const wxString& value)
	{
		return SevenZip::TStringView(value.wc_str(), value.length());
	}

	wxString ToWxString(const SevenZip::TString& value)
	{
		return wxString(value.data(), value.length());
	}
	wxString ToWxString(const SevenZip::TStringView& value)
	{
		return wxString(value.data(), value.length());
	}

	SevenZip::TStringVector ToTStringVector(const KxStringVector& vector)
	{
		return KxUtility::ConvertVector<SevenZip::TString>(vector, [](const wxString& value)
		{
			return ToTString(value);
		});
	}

	wxDateTime ToDateTime(const FILETIME& fileTime)
	{
		SYSTEMTIME systemTime = {};
		if (::FileTimeToSystemTime(&fileTime, &systemTime))
		{
			return wxDateTime().SetFromMSWSysTime(systemTime);
		}
		return wxDefaultDateTime;
	}
	std::optional<wxSeekMode> SeekModeToWx(int seekMode)
	{
		switch (seekMode)
		{
			case SEEK_CUR:
			{
				return wxSeekMode::wxFromCurrent;
			}
			case SEEK_SET:
			{
				return wxSeekMode::wxFromStart;
			}
			case SEEK_END:
			{
				return wxSeekMode::wxFromEnd;
			}
		};
		return std::nullopt;
	}
	KxFileItem ToKxFileItem(const SevenZip::FileInfo& archiveItem, size_t fileIndex)
	{
		KxFileItem item;
		item.SetExtraData(fileIndex);

		wxString fullPath = ToWxString(archiveItem.FileName);
		fullPath.Replace(wxS('/'), wxS('\\'), true);
		item.SetFullPath(fullPath);

		item.SetFileSize(archiveItem.Size);
		item.SetCompressedFileSize(archiveItem.CompressedSize);

		item.SetCreationTime(ToDateTime(archiveItem.CreationTime));
		item.SetModificationTime(ToDateTime(archiveItem.LastWriteTime));
		item.SetLastAccessTime(ToDateTime(archiveItem.LastAccessTime));

		// TODO: check if archive attributes are really Windows file attributes.
		uint32_t attributes = archiveItem.Attributes;

		// At least directory attribute is guaranteed to be assigned correctly.
		if (archiveItem.IsDirectory)
		{
			attributes |= FILE_ATTRIBUTE_DIRECTORY;
		}
		if (archiveItem.CompressedSize < archiveItem.Size)
		{
			attributes |= FILE_ATTRIBUTE_COMPRESSED;
		}

		item.SetAttributes(attributes);

		return item;
	}

	struct SearchData
	{
		static constexpr size_t InvalidIndex = std::numeric_limits<size_t>::max();

		wxString m_SearchQuery;
		size_t m_NextIndex = InvalidIndex;

		SearchData(const wxString& filter, size_t index = InvalidIndex)
			:m_SearchQuery(filter), m_NextIndex(index)
		{
		}
	};
	bool SearchFiles(const SevenZip::FileInfo::Vector& files, const wxString& filter, KxFileItem& item, size_t startAt, size_t& nextIndex)
	{
		for (size_t i = startAt; i < files.size(); i++)
		{
			if (KxComparator::Matches(std::wstring_view(files[i].FileName), ToTStringView(filter)))
			{
				item = ToKxFileItem(files[i], i);

				nextIndex = i + 1;
				return true;
			}
		}

		nextIndex = SearchData::InvalidIndex;
		return false;
	}
}
namespace
{
	namespace FormatNS
	{
		using KAEnum = Kortex::Archive::Format;
		using SZEnum = SevenZip::CompressionFormat;

		KAEnum Convert(SZEnum type)
		{
			switch (type)
			{
				case SZEnum::SevenZip:
				{
					return KAEnum::SevenZip;
				}
				case SZEnum::Zip:
				{
					return KAEnum::Zip;
				}
				case SZEnum::Rar:
				{
					return KAEnum::RAR;
				}
				case SZEnum::Rar5:
				{
					return KAEnum::RAR5;
				}
				case SZEnum::GZip:
				{
					return KAEnum::GZip;
				}
				case SZEnum::BZip2:
				{
					return KAEnum::BZip2;
				}
				case SZEnum::Tar:
				{
					return KAEnum::Tar;
				}
				case SZEnum::Iso:
				{
					return KAEnum::ISO;
				}
				case SZEnum::Cab:
				{
					return KAEnum::CAB;
				}
				case SZEnum::Lzma:
				{
					return KAEnum::LZMA;
				}
				case SZEnum::Lzma86:
				{
					return KAEnum::LZMA86;
				}
			};
			return KAEnum::Unknown;
		}
		SZEnum Convert(KAEnum type)
		{
			switch (type)
			{
				case KAEnum::SevenZip:
				{
					return SZEnum::SevenZip;
				}
				case KAEnum::Zip:
				{
					return SZEnum::Zip;
				}
				case KAEnum::RAR:
				{
					return SZEnum::Rar;
				}
				case KAEnum::RAR5:
				{
					return SZEnum::Rar5;
				}
				case KAEnum::GZip:
				{
					return SZEnum::GZip;
				}
				case KAEnum::BZip2:
				{
					return SZEnum::BZip2;
				}
				case KAEnum::Tar:
				{
					return SZEnum::Tar;
				}
				case KAEnum::ISO:
				{
					return SZEnum::Iso;
				}
				case KAEnum::CAB:
				{
					return SZEnum::Cab;
				}
				case KAEnum::LZMA:
				{
					return SZEnum::Lzma;
				}
				case KAEnum::LZMA86:
				{
					return SZEnum::Lzma86;
				}
			}
			return SZEnum::Unknown;
		}
	}
	namespace MethodNS
	{
		using KAEnum = Kortex::Archive::Method;
		using SZEnum = SevenZip::CompressionMethod;

		KAEnum Convert(SZEnum type)
		{
			switch (type)
			{
				case SZEnum::LZMA:
				{
					return KAEnum::LZMA;
				}
				case SZEnum::LZMA2:
				{
					return KAEnum::LZMA2;
				}
				case SZEnum::PPMD:
				{
					return KAEnum::PPMd;
				}
				case SZEnum::BZIP2:
				{
					return KAEnum::BZip2;
				}
			};
			return KAEnum::Unknown;
		}
		SZEnum Convert(KAEnum type)
		{
			switch (type)
			{
				case KAEnum::LZMA:
				{
					return SZEnum::LZMA;
				}
				case KAEnum::LZMA2:
				{
					return SZEnum::LZMA2;
				}
				case KAEnum::PPMd:
				{
					return SZEnum::PPMD;
				}
				case KAEnum::BZip2:
				{
					return SZEnum::BZIP2;
				}
			}
			return SZEnum::Unknown;
		}
	}
}

namespace SevenZip
{
	class GenericArchiveNotifier: public ProgressNotifier
	{
		private:
			wxEvtHandler& m_EvtHandler;

			wxString m_CurrentStatus;
			int64_t m_BytesTotal = 0;
			int64_t m_BytesCompleted = 0;
			bool m_ShouldCancel = false;

		private:
			void SetCurrentStatus(TStringView status)
			{
				if (!status.empty())
				{
					m_CurrentStatus.assign(status.data(), status.length());
				}
			}
			KxArchiveEvent CreateEvent(wxEventType type = KxArchiveEvent::EvtProcess) const
			{
				KxArchiveEvent event(type);
				event.Allow();
				event.SetEventObject(&m_EvtHandler);
				event.SetCurrent(m_CurrentStatus);
				event.SetMinorProcessed(m_BytesCompleted);
				event.SetMinorTotal(m_BytesTotal);

				return event;
			}

		public:
			GenericArchiveNotifier(wxEvtHandler& evtHandler)
				:m_EvtHandler(evtHandler)
			{
			}

		public:
			bool ShouldCancel() override
			{
				return m_ShouldCancel;
			}
			
			void OnStart(TStringView status, int64_t bytesTotal) override
			{
				SetCurrentStatus(status);
				m_BytesTotal = bytesTotal;

				KxArchiveEvent event = CreateEvent();
				event.SetCurrent(m_CurrentStatus);
				m_EvtHandler.ProcessEvent(event);

				m_ShouldCancel = !event.IsAllowed();
			}
			void OnProgress(TStringView status, int64_t bytesCompleted) override
			{
				SetCurrentStatus(status);
				m_BytesCompleted = bytesCompleted;

				KxArchiveEvent event = CreateEvent();
				m_EvtHandler.ProcessEvent(event);

				m_ShouldCancel = !event.IsAllowed();
			}
			void OnEnd() override
			{
				KxArchiveEvent event = CreateEvent(KxArchiveEvent::EvtDone);
				m_EvtHandler.ProcessEvent(event);

				m_ShouldCancel = !event.IsAllowed();
			}
	};

	class OutStreamWrapper_wxOutputStream: public OutStreamWrapper
	{
		protected:
			wxOutputStream& m_Stream;

		public:
			HRESULT DoWrite(const void* data, uint32_t size, uint32_t& written) override
			{
				m_Stream.Write(data, size);
				written = m_Stream.LastWrite();

				return written == size ? S_OK : E_FAIL;
			}
			HRESULT DoSeek(int64_t offset, uint32_t seekMode, int64_t& newPosition) override
			{
				if (!m_Stream.IsSeekable())
				{
					return E_NOTIMPL;
				}

				if (auto seekModeWx = SeekModeToWx(seekMode))
				{
					newPosition = m_Stream.SeekO(offset, *seekModeWx);
					return m_Stream.IsOk() ? S_OK : E_FAIL;
				}
				return E_INVALIDARG;
			}
			HRESULT DoSetSize(int64_t size) override
			{
				if (m_Stream.IsKindOf(wxCLASSINFO(KxFileStream)))
				{
					return static_cast<KxFileStream&>(m_Stream).SetAllocationSize(size) ? S_OK : E_FAIL;
				}
				return E_NOTIMPL;
			}

		public:
			OutStreamWrapper_wxOutputStream(wxOutputStream& stream, ProgressNotifier* notifier = nullptr)
				:OutStreamWrapper(notifier), m_Stream(stream)
			{
			}
	};
}

namespace SevenZip
{
	class CallbackExtractorWrapper: public Callback::Extractor
	{
		private:
			KxArchive::IExtractionCallback& m_Callback;
			const KxArchive::IArchiveItems& m_ArchiveItems;

			KxDelegateOutputStream m_Stream;
			FileIndex m_FileIndex = InvalidFileIndex;

		private:
			bool ShouldCancel() const
			{
				return m_Notifier.ShouldCancel() || m_Callback.ShouldCancel();
			}

		public:
			CallbackExtractorWrapper(KxArchive::IExtractionCallback& callback, const KxArchive::IArchiveItems& archiveItems)
				:m_Callback(callback), m_ArchiveItems(archiveItems)
			{
			}

		public:
			STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode) override
			{
				m_Stream = nullptr;
				m_FileIndex = InvalidFileIndex;

				if (ShouldCancel())
				{
					return E_ABORT;
				}
				if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
				{
					return S_OK;
				}

				if (m_Stream = m_Callback.OnGetStream(index))
				{
					m_FileIndex = index;
					*outStream = CreateObject<OutStreamWrapper_wxOutputStream>(*m_Stream, *m_Notifier).Detach();

					if (m_Notifier)
					{
						if (KxFileItem fileItem = m_ArchiveItems.GetItem(index))
						{
							wxString path = fileItem.GetFullPath();
							m_Notifier.OnStart(ToTStringView(path), fileItem.GetFileSize());
						}
						else
						{
							wxFileOffset streamSize = m_Stream->GetLength();
							if (streamSize == wxInvalidOffset)
							{
								streamSize = m_Stream->TellO();
							}

							m_Notifier.OnStart({}, streamSize);
						}
					}
				}
				return ShouldCancel() ? E_ABORT : S_OK;
			}
			STDMETHOD(PrepareOperation)(Int32 askExtractMode) override
			{
				return ShouldCancel() ? E_ABORT : S_OK;
			}
			STDMETHOD(SetOperationResult)(Int32 resultEOperationResult) override
			{
				KxDelegateOutputStream stream = std::move(m_Stream);

				m_Notifier.OnEnd();
				if (ShouldCancel())
				{
					return E_ABORT;
				}
				if (stream && !m_Callback.OnOperationCompleted(m_FileIndex, *stream))
				{
					return E_FAIL;
				}
				return S_OK;
			}
	};
}

namespace Kortex
{
	wxString GenericArchive::GetLibraryPath()
	{
		wxString path = IApplication::GetInstance()->GetDataFolder();
		#if defined _WIN64
		path += wxS("\\7z x64.dll");
		#else
		path += wxS("\\7z.dll");
		#endif
		return path;
	}
	wxString GenericArchive::GetLibraryVersion()
	{
		return KxLibrary::GetVersionInfoFromFile(GetLibraryPath()).GetString("ProductVersion");
	}

	bool GenericArchive::IsLibraryLoaded()
	{
		return g_ArchiveLibrary.IsLoaded();
	}
	bool GenericArchive::Init()
	{
		g_ArchiveLibrary.Load(ToTStringView(GetLibraryPath()));
		return IsLibraryLoaded();
	}
	bool GenericArchive::UnInit()
	{
		if (IsLibraryLoaded())
		{
			g_ArchiveLibrary.Free();
			return true;
		}
		return false;
	}

	void GenericArchive::OpenArchive(const wxString& filePath)
	{
		CloseArchive();

		// Notifier will be created when first event handler will be bound
		m_Archive = std::make_unique<SevenZip::Archive>(g_ArchiveLibrary, ToTString(filePath), m_Notifier.get());
	}
	void GenericArchive::CloseArchive()
	{
		m_Archive = nullptr;
		m_Notifier = nullptr;

		m_OriginalSize = std::nullopt;
		m_CompressedSize = std::nullopt;
	}

	bool GenericArchive::OnDynamicBind(wxDynamicEventTableEntry& entry)
	{
		if (!m_Notifier)
		{
			m_Notifier = std::make_unique<SevenZip::GenericArchiveNotifier>(*this);
		}
		if (m_Archive)
		{
			m_Archive->SetNotifier(m_Notifier.get());
		}

		return wxEvtHandler::OnDynamicBind(entry);
	}

	GenericArchive::GenericArchive() = default;
	GenericArchive::GenericArchive(const wxString& filePath)
	{
		OpenArchive(filePath);
	}
	GenericArchive::GenericArchive(GenericArchive&& other)
	{
		*this = std::move(other);
	}
	GenericArchive::~GenericArchive()
	{
		CloseArchive();
	}

	// IArchive
	bool GenericArchive::IsOK() const
	{
		return m_Archive && m_Archive->IsLoaded();
	}
	bool GenericArchive::Open(const wxString& filePath)
	{
		OpenArchive(filePath);
		return IsOK();
	}
	void GenericArchive::Close()
	{
		CloseArchive();
	}
	wxString GenericArchive::GetFilePath() const
	{
		return ToWxString(m_Archive->GetProperty_FilePath());
	}

	int64_t GenericArchive::GetOriginalSize() const
	{
		if (!m_OriginalSize)
		{
			int64_t total = 0;
			for (const SevenZip::FileInfo& info: m_Archive->GetItems())
			{
				if (!info.IsDirectory && info.Size > 0)
				{
					total += info.Size;
				}
			}
			m_OriginalSize = total;
		}
		return *m_OriginalSize;
	}
	int64_t GenericArchive::GetCompressedSize() const
	{
		if (!m_CompressedSize)
		{
			int64_t total = 0;
			for (const SevenZip::FileInfo& info: m_Archive->GetItems())
			{
				if (!info.IsDirectory && info.CompressedSize > 0)
				{
					total += info.CompressedSize;
				}
			}
			m_CompressedSize = total;
		}
		return *m_CompressedSize;
	}
	
	// IArchiveItems
	size_t GenericArchive::GetItemCount() const
	{
		return m_Archive->GetItemCount();
	}
	KxFileItem GenericArchive::GetItem(size_t fileIndex) const
	{
		const auto& items = m_Archive->GetItems();
		if (fileIndex < items.size())
		{
			return ToKxFileItem(items[fileIndex], fileIndex);
		}
		return {};
	}

	// IArchiveSearch
	void* GenericArchive::FindFirstFile(const wxString& filter, KxFileItem& fileItem) const
	{
		fileItem = {};
		size_t nextIndex = SearchData::InvalidIndex;
		
		wxString filterCopy = filter;
		filterCopy.Replace(wxS('/'), wxS('\\'), true);
		if (SearchFiles(m_Archive->GetItems(), filterCopy, fileItem, 0, nextIndex))
		{
			return std::make_unique<SearchData>(filterCopy, nextIndex).release();
		}
		return nullptr;
	}
	bool GenericArchive::FindNextFile(void* handle, KxFileItem& item) const
	{
		item = {};
		if (handle)
		{
			SearchData& searchData = *reinterpret_cast<SearchData*>(handle);

			size_t nextIndex = SearchData::InvalidIndex;
			if (SearchFiles(m_Archive->GetItems(), searchData.m_SearchQuery, item, searchData.m_NextIndex, nextIndex))
			{
				searchData.m_NextIndex = nextIndex;
				return true;
			}
		}
		return false;
	}
	void GenericArchive::FindClose(void* handle) const
	{
		delete reinterpret_cast<SearchData*>(handle);
	}

	// IArchiveExtraction
	bool GenericArchive::Extract(KxArchive::IExtractionCallback& callback) const
	{
		auto extractor = SevenZip::CreateObject<SevenZip::CallbackExtractorWrapper>(callback, *this);
		return m_Archive->Extract(extractor.Detach());
	}
	bool GenericArchive::Extract(KxArchive::IExtractionCallback& callback, KxArchive::FileIndexView files) const
	{
		auto extractor = SevenZip::CreateObject<SevenZip::CallbackExtractorWrapper>(callback, *this);
		return m_Archive->Extract(extractor.Detach(), SevenZip::FileIndexView(files.data(), files.size()));
	}

	// IArchiveCompression
	bool GenericArchive::CompressDirectory(const wxString& directory, bool recursive)
	{
		return m_Archive->CompressDirectory(ToTString(directory), recursive);
	}
	
	bool GenericArchive::CompressFiles(const wxString& directory, const wxString& searchFilter, bool recursive)
	{
		return m_Archive->CompressFiles(ToTString(directory), ToTString(searchFilter), recursive);
	}
	bool GenericArchive::CompressSpecifiedFiles(const KxStringVector& sourcePaths, const KxStringVector& archivePaths)
	{
		return m_Archive->CompressSpecifiedFiles(ToTStringVector(sourcePaths), ToTStringVector(archivePaths));
	}
	
	bool GenericArchive::CompressFile(const wxString& sourcePath)
	{
		return m_Archive->CompressFile(ToTString(sourcePath));
	}
	bool GenericArchive::CompressFile(const wxString& sourcePath, const wxString& archivePath)
	{
		return m_Archive->CompressFile(ToTString(sourcePath), ToTString(archivePath));
	}

	// IBoolProperties
	bool GenericArchive::GetPropertyBool(const TBoolPropertyIndex& property) const
	{
		switch (property)
		{
			case TBoolPropertyIndex::Solid:
			{
				return m_Archive->GetProperty_Solid();
			}
			case TBoolPropertyIndex::MultiThreaded:
			{
				return m_Archive->GetProperty_MultiThreaded();
			}
		};
		return false;
	}
	void GenericArchive::SetPropertyBool(const TBoolPropertyIndex& property, bool value)
	{
		switch (property)
		{
			case TBoolPropertyIndex::Solid:
			{
				return m_Archive->SetProperty_Solid(value);
			}
			case TBoolPropertyIndex::MultiThreaded:
			{
				return m_Archive->SetProperty_MultiThreaded(value);
			}
		};
	}

	// IIntProperties
	int GenericArchive::GetPropertyInt(const TIntPropertyIndex& property) const
	{
		switch (property)
		{
			case TIntPropertyIndex::CompressionLevel:
			{
				return m_Archive->GetProperty_CompressionLevel();
			}
			case TIntPropertyIndex::DictionarySize:
			{
				return m_Archive->GetProperty_DictionarySize();
			}
			case TIntPropertyIndex::Format:
			{
				Archive::Format format = FormatNS::Convert(m_Archive->GetProperty_CompressionFormat());

				// This library can't detect 7z archive for some reason, but can perfectly read other data from it.
				// So check for original size validity and set format manually.
				if (format == Archive::Format::Unknown && GetOriginalSize() > 0)
				{
					format = Archive::Format::SevenZip;
				}
				return (int)format;
			}
			case TIntPropertyIndex::Method:
			{
				return (int)MethodNS::Convert(m_Archive->GetProperty_CompressionMethod());
			}
		};
		return -1;
	}
	void GenericArchive::SetPropertyInt(const TIntPropertyIndex& property, int value)
	{
		switch (property)
		{
			case TIntPropertyIndex::CompressionLevel:
			{
				return m_Archive->SetProperty_CompressionLevel(value);
			}
			case TIntPropertyIndex::DictionarySize:
			{
				return m_Archive->SetProperty_DictionarySize(value);
			}
			case TIntPropertyIndex::Format:
			{
				return m_Archive->SetProperty_CompressionFormat(FormatNS::Convert((Archive::Format)value));
			}
			case TIntPropertyIndex::Method:
			{
				return m_Archive->SetProperty_CompressionMethod(MethodNS::Convert((Archive::Method)value));
			}
		};
	}

	GenericArchive& GenericArchive::operator=(GenericArchive&& other)
	{
		m_Archive = std::move(other.m_Archive);
		m_Notifier = std::move(other.m_Notifier);

		m_OriginalSize = std::move(other.m_OriginalSize);
		m_CompressedSize = std::move(other.m_CompressedSize);
	}
}

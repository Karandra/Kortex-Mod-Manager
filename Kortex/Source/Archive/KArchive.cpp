#include "stdafx.h"
#include "KArchive.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxComparator.h>
#include <7zpp/7zpp.h>
#pragma comment(lib, "7zpp_u.lib")

namespace
{
	SevenZip::SevenZipLibrary ArchiveLibrary;

	SevenZip::TString ToTString(const wxString& value)
	{
		return SevenZip::TString(value.wc_str(), value.length());
	}
	SevenZip::TStringView ToTStringView(const wxString& value)
	{
		return SevenZip::TStringView(value.wc_str(), value.length());
	}

	const wxString ToWxString(const SevenZip::TString& value)
	{
		return wxString(value.data(), value.length());
	}
	const wxString ToWxString(const SevenZip::TStringView& value)
	{
		return wxString(value.data(), value.length());
	}

	SevenZip::TStringVector ToTStringVector(const KxStringVector& vector)
	{
		return KxUtility::RepackVector<wxString, SevenZip::TString>(vector, [](const wxString& value)
		{
			return ToTString(value);
		});
	}

	class AcrhiveNotifier: public SevenZip::ProgressNotifier
	{
		using TCharType = SevenZip::TCharType;

		private:
			wxEvtHandler* m_EvtHandler = NULL;
			int64_t m_LastMajor = 0;
			int64_t m_TotalSize = 0;
			wxString m_CurrentFile;
			bool m_ShouldStop = false;

		private:
			void SetCurrentFile(const TCharType* filePath)
			{
				if (filePath && *filePath != _T('\000'))
				{
					m_CurrentFile = filePath;
				}
			}
			KxArchiveEvent CreateEvent(wxEventType type = KxEVT_ARCHIVE) const
			{
				KxArchiveEvent event(type);
				event.Allow();
				event.SetEventObject(m_EvtHandler);
				event.SetCurrent(m_CurrentFile);
				event.SetMajorProcessed(m_LastMajor);
				event.SetMajorTotal(m_TotalSize);
				event.SetMinorProcessed(-2);
				event.SetMinorTotal(-2);

				return event;
			}

		public:
			AcrhiveNotifier(wxEvtHandler* eventHandler)
				:m_EvtHandler(eventHandler)
			{
			}

		public:
			virtual bool ShouldStop() override
			{
				return m_ShouldStop;
			}
			virtual void OnStartWithTotal(const TCharType* filePath, int64_t totalBytes) override
			{
				SetCurrentFile(filePath);
				m_TotalSize = totalBytes;

				KxArchiveEvent event = CreateEvent();
				event.SetCurrent(filePath);
				m_EvtHandler->ProcessEvent(event);
			}
			virtual void OnMajorProgress(const TCharType* filePath, int64_t bytesCompleted) override
			{
				SetCurrentFile(filePath);
				m_LastMajor = bytesCompleted;

				KxArchiveEvent event = CreateEvent();
				m_EvtHandler->ProcessEvent(event);
				m_ShouldStop = !event.IsAllowed();
			}
			virtual void OnMinorProgress(const TCharType* filePath, int64_t bytesCompleted, int64_t totalBytes) override
			{
				SetCurrentFile(filePath);

				KxArchiveEvent event = CreateEvent();
				event.SetMinorProcessed(bytesCompleted);
				event.SetMinorTotal(totalBytes);
				m_EvtHandler->ProcessEvent(event);

				m_ShouldStop = !event.IsAllowed();
			}
			virtual void OnDone(const TCharType* filePath) override
			{
				SetCurrentFile(filePath);

				KxArchiveEvent event = CreateEvent(KxEVT_ARCHIVE_DONE);
				event.SetMinorProcessed(m_TotalSize);
				m_EvtHandler->ProcessEvent(event);
			}
	};

	struct SearchData
	{
		wxString m_SearchQuery;
		size_t m_NextIndex = (size_t)-1;

		SearchData(const wxString& filter, size_t index = (size_t)-1)
			:m_SearchQuery(filter), m_NextIndex(index)
		{
		}
	};
	bool SearchFiles(const SevenZip::TStringVector& files, const wxString& filter, KxFileItem& item, size_t startAt, size_t& nextIndex)
	{
		for (size_t i = startAt; i < files.size(); i++)
		{
			const SevenZip::TString& path = files[i];
			if (KxComparator::Matches(std::wstring_view(path), ToTStringView(filter)))
			{
				// Extract directory from path
				size_t pos = path.rfind(L'\\');
				if (pos != path.npos)
				{
					SevenZip::TStringView name(path.data() + pos + 1);
					SevenZip::TStringView directory(path.data(), pos);

					item.SetName(ToWxString(name));
					item.SetSource(ToWxString(directory));
				}
				else
				{
					item.SetName(ToWxString(path));
				}

				item.SetNormalAttributes();
				item.SetExtraData(i);

				nextIndex = i + 1;
				return true;
			}
		}

		nextIndex = (size_t)-1;
		return false;
	}
}

namespace FormatNS
{
	using KAEnum = KArchiveNS::Format::_Enum;
	using SZEnum = SevenZip::CompressionFormat::_Enum;

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
	using KAEnum = KArchiveNS::Method::_Enum;
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

wxString KArchive::GetLibraryPath()
{
	wxString path = KApp::Get().GetDataFolder();
	#if defined _WIN64
	path += "\\7z x64.dll";
	#else
	path += "\\7z.dll";
	#endif
	return path;
}
wxString KArchive::GetLibraryVersion()
{
	return KxLibrary::GetVersionInfoFromFile(GetLibraryPath()).GetString("ProductVersion");
}

bool KArchive::IsLibraryLoaded()
{
	return ArchiveLibrary.IsLoaded();
}
bool KArchive::Init()
{
	ArchiveLibrary.Load(GetLibraryPath().wc_str());
	return IsLibraryLoaded();
}
bool KArchive::UnInit()
{
	if (IsLibraryLoaded())
	{
		ArchiveLibrary.Free();
		return true;
	}
	return false;
}

KArchive::KArchive()
{
}
KArchive::KArchive(const wxString& filePath)
{
	Open(filePath);
}
KArchive::~KArchive()
{
	Close();
}

// KxIArchive
bool KArchive::IsOK() const
{
	return m_Impl != NULL;
}
bool KArchive::Open(const wxString& filePath)
{
	Close();

	AcrhiveNotifier notifier(this);
	m_Impl = new SevenZip::SevenZipArchive(ArchiveLibrary, ToTString(filePath), &notifier);
	m_Impl->ReadInArchiveMetadata();

	m_FilePath = filePath;
	m_CompressedSize = KxFile(filePath).GetFileSize();
	return IsOK();
}
void KArchive::Close()
{
	delete m_Impl;

	m_FilePath.clear();
	m_CompressedSize = -1;
	m_OriginalSize = -1;
	m_Impl = NULL;
}
wxString KArchive::GetFilePath() const
{
	return m_FilePath;
}

size_t KArchive::GetItemCount() const
{
	return m_Impl->GetNumberOfItems();
}
wxString KArchive::GetItemName(size_t index) const
{
	return index < GetItemCount() ? m_Impl->GetItemsNames()[index] : wxEmptyString;
}
int64_t KArchive::GetOriginalSize() const
{
	if (m_OriginalSize == -1)
	{
		m_OriginalSize = 0;
		for (size_t size: m_Impl->GetOrigSizes())
		{
			m_OriginalSize += size;
		}
	}
	return m_OriginalSize;
}
int64_t KArchive::GetCompressedSize() const
{
	return m_CompressedSize;
}

// KxIArchiveSearch
void* KArchive::FindFirstFile(const wxString& filter, KxFileItem& fileItem) const
{
	fileItem = KxFileItem();
	size_t nextIndex = (size_t)-1;
	if (SearchFiles(m_Impl->GetItemsNames(), filter, fileItem, 0, nextIndex))
	{
		return new SearchData(filter, nextIndex);
	}
	return NULL;
}
bool KArchive::FindNextFile(void* handle, KxFileItem& item) const
{
	item = KxFileItem();
	SearchData* searchData = reinterpret_cast<SearchData*>(handle);

	size_t nextIndex = (size_t)-1;
	if (SearchFiles(m_Impl->GetItemsNames(), searchData->m_SearchQuery, item, searchData->m_NextIndex, nextIndex))
	{
		searchData->m_NextIndex = nextIndex;
		return true;
	}
	return false;
}
void KArchive::FindClose(void* handle) const
{
	SearchData* searchData = reinterpret_cast<SearchData*>(handle);
	delete searchData;
}

// KxIArchiveExtraction
bool KArchive::DoExtractAll(const wxString& directory) const
{
	AcrhiveNotifier notifier(const_cast<KArchive*>(this));
	return m_Impl->ExtractArchive(directory.wc_str(), &notifier);
}
bool KArchive::DoExtractToDirectory(const IndexVector& indexes, const wxString& directory) const
{
	AcrhiveNotifier notifier(const_cast<KArchive*>(this));
	return m_Impl->ExtractArchive(indexes, directory.wc_str(), &notifier);
}
bool KArchive::DoExtractToFiles(const IndexVector& indexes, const KxStringVector& filePaths) const
{
	AcrhiveNotifier notifier(const_cast<KArchive*>(this));
	return m_Impl->ExtractArchive(indexes, ToTStringVector(filePaths), &notifier);
}
KArchive::Buffer KArchive::DoExtractToMemory(size_t index) const
{
	KArchive::Buffer buffer;
	if (index < GetItemCount())
	{
		AcrhiveNotifier notifier(const_cast<KArchive*>(this));
		m_Impl->ExtractToMemory(index, buffer, &notifier);
	}
	return buffer;
}
KArchive::BufferMap KArchive::DoExtractToMemory(const IndexVector& indexes) const
{
	KArchive::BufferMap bufferMap;
	AcrhiveNotifier notifier(const_cast<KArchive*>(this));
	m_Impl->ExtractToMemory(indexes, bufferMap, &notifier);
	return bufferMap;
}

// KxIArchiveCompression
bool KArchive::DoCompressFiles(const wxString& directory, const wxString& searchFilter, bool recursive)
{
	AcrhiveNotifier notifier(const_cast<KArchive*>(this));
	return m_Impl->CompressFiles(ToTString(directory), ToTString(searchFilter), recursive, &notifier);
}
bool KArchive::DoCompressDirectory(const wxString& directory, bool recursive)
{
	AcrhiveNotifier notifier(const_cast<KArchive*>(this));
	return m_Impl->CompressDirectory(ToTString(directory), recursive, &notifier);
}
bool KArchive::DoCompressSpecifiedFiles(const KxStringVector& sourcePaths, const KxStringVector& archivePaths)
{
	AcrhiveNotifier notifier(const_cast<KArchive*>(this));
	return m_Impl->CompressSpecifiedFiles(ToTStringVector(sourcePaths), ToTStringVector(archivePaths), &notifier);
}
bool KArchive::DoCompressFile(const wxString& sourcePath)
{
	AcrhiveNotifier notifier(const_cast<KArchive*>(this));
	return m_Impl->CompressFile(ToTString(sourcePath), &notifier);
}
bool KArchive::DoCompressFile(const wxString& sourcePath, const wxString& archivePath)
{
	AcrhiveNotifier notifier(const_cast<KArchive*>(this));
	return m_Impl->CompressFile(ToTString(sourcePath), ToTString(archivePath), &notifier);
}

// KxIArchivePropertiesBool
bool KArchive::GetPropertyBool(BoolProperties property) const
{
	switch (property)
	{
		case BoolProperties::Solid:
		{
			return m_Impl->GetProperty_Solid();
		}
		case BoolProperties::MultiThreaded:
		{
			return m_Impl->GetProperty_MultiThreaded();
		}
	};
	return false;
}
void KArchive::SetPropertyBool(BoolProperties property, bool value)
{
	switch (property)
	{
		case BoolProperties::Solid:
		{
			return m_Impl->SetProperty_Solid(value);
		}
		case BoolProperties::MultiThreaded:
		{
			return m_Impl->SetProperty_MultiThreaded(value);
		}
	};
}

// KxIArchivePropertiesInt
int KArchive::GetPropertyInt(IntProperties property) const
{
	switch (property)
	{
		case IntProperties::CompressionLevel:
		{
			return m_Impl->GetProperty_CompressionLevel();
		}
		case IntProperties::DictionarySize:
		{
			return m_Impl->GetProperty_DictionarySize();
		}
		case IntProperties::Format:
		{
			KArchiveNS::Format::_Enum format = FormatNS::Convert(m_Impl->GetProperty_CompressionFormat());

			// This library can't detect 7z archive for some reason, but can perfectly read other data from it.
			// So check for original size validity and set format manually.
			if (format == KArchiveNS::Format::Unknown && GetOriginalSize() != -1)
			{
				format = KArchiveNS::Format::SevenZip;
			}
			return format;
		}
		case IntProperties::Method:
		{
			return (int)MethodNS::Convert(m_Impl->GetProperty_CompressionMethod());
		}
	};
	return -1;
}
void KArchive::SetPropertyInt(IntProperties property, int value)
{
	switch (property)
	{
		case IntProperties::CompressionLevel:
		{
			return m_Impl->SetProperty_CompressionLevel(value);
		}
		case IntProperties::DictionarySize:
		{
			return m_Impl->SetProperty_DictionarySize(value);
		}
		case IntProperties::Format:
		{
			return m_Impl->SetProperty_CompressionFormat(FormatNS::Convert((KArchiveNS::Format::_Enum)value));
		}
		case IntProperties::Method:
		{
			return m_Impl->SetProperty_CompressionMethod(MethodNS::Convert((KArchiveNS::Method::_Enum)value));
		}
	};
}

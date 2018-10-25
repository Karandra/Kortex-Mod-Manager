#include "stdafx.h"
#include "KBethesdaArchive.h"
#include <KxFramework/KxComparator.h>

namespace
{
	wxString ToWxString(const std::wstring_view& value)
	{
		return wxString(value.data(), value.length());
	}

	bool IsFileNamesEmbedded(const KBethesdaArchive::Header& header)
	{
		return header.Version == (uint32_t)KBethesdaArchive::Version::Skyrim && header.ArchiveFlags & (uint32_t)KBethesdaArchive::ArchiveFlags::EmbedFileNames;
	}
	void SkipBString(KxFileStream& stream)
	{
		stream.Skip(stream.ReadObject<uint8_t>());
	}

	struct SearchData
	{
		wxString m_SearchQuery;
		size_t m_NextIndex = (size_t)-1;

		SearchData(const wxString& filter, size_t index = (size_t)-1)
			:m_SearchQuery(filter), m_NextIndex(index)
		{
		}
	};
	bool SearchFiles(const std::vector<KBethesdaArchive::FileItem>& files, const wxString& filter, KxFileItem& item, size_t startAt, size_t& nextIndex)
	{
		for (size_t i = startAt; i < files.size(); i++)
		{
			const wxString& path = files[i].Name;
			if (KxComparator::Matches(path, filter))
			{
				// Extract directory from path
				size_t pos = path.rfind(L'\\');
				if (pos != path.npos)
				{
					std::wstring_view name(path.data() + pos + 1);
					std::wstring_view directory(path.data(), pos);

					item.SetName(ToWxString(name));
					item.SetSource(ToWxString(directory));
				}
				else
				{
					item.SetName(path);
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

/*
The hash cannot be calculated using / (forward slashes) or upper case characters. Forward slashes
must be changed to backslashes and upper case characters must be converted to lower case.
Folder hash values must not include leading or trailing slashes. While some folder names contain a file extension
(e.g. textures\actors\character\facegendata\facetint\skyrim.esm\), this must not be treated as a
file extension in the calculation. Some parts of the algorithm need to perform calculations on the extension
separately from the filename. The filename substring must be only the filename with no slashes or extension
(e.g. meshes\animbehaviors\doors\doortemplate01.hkx -> doortemplate01). The extension must include the '.'.

http://en.uesp.net/wiki/Tes4Mod:Hash_Calculation
*/
uint64_t KBethesdaArchive::HashFilePath(const wxString& sourcePath, bool isFolderPath, wxString* correctedPath)
{
	if (!sourcePath.IsEmpty())
	{
		wxString path = KxString::ToLower(sourcePath);
		path.Replace(wxS("/"), wxS("\\"));

		uint64_t hash1 = 0;
		uint64_t hash2 = 0;

		// If this is a file with an extension
		int extensionIndex = -1;
		bool hasExtension = false;
		if (!isFolderPath)
		{
			path = path.AfterLast(wxS('\\'));

			extensionIndex = path.Find(wxS('.'), true);
			hasExtension = extensionIndex != wxNOT_FOUND;
		}

		// Hash 1
		if (hasExtension)
		{
			for (size_t i = extensionIndex; i < path.Length(); i++)
			{
				hash1 = (hash1 * 0x1003f) + (wxChar)path[i];
			}

			// From here on, path must NOT include the file extension.
			path.Truncate(extensionIndex);
		}

		if (path.Length() > 2)
		{
			for (size_t i = 1; i < path.Length() - 2; i++)
			{
				hash2 = (hash2 * 0x1003f) + (wxChar)path[i];
			}
		}
		hash1 += hash2;
		hash2 = 0;

		// Move Hash 1 to the upper bits
		hash1 <<= 32;

		// Hash 2
		hash2 = (wxChar)path[path.Length() - 1];
		hash2 |= (path.Length() > 2) ? (wxChar)path[path.Length() - 2] << 8 : 0;
		hash2 |= path.Length() << 16;
		hash2 |= ((wxChar)path[0] << 24);

		if (hasExtension)
		{
			const int extensionLength = sizeof(uint32_t);
			char extensionString[extensionLength + 1] = {'\0'};
			for (size_t i = 0; i < extensionLength; i++)
			{
				size_t charIndex = extensionIndex + i;
				if (charIndex < sourcePath.Length())
				{
					extensionString[i] = (wxChar)sourcePath[charIndex];
				}
			}

			// Load these 4 bytes as integer
			switch (*(uint32_t*)extensionString)
			{
				// 2E 6B 66 00 == .kf\0
				case 0x00666B2E:
				{
					hash2 |= 0x80;
					break;
				}

				// .nif
				case 0x66696E2E:
				{
					hash2 |= 0x8000;
					break;
				}

				// .dds
				case 0x7364642E:
				{
					hash2 |= 0x8080;
					break;
				}

				// .wav
				case 0x7661772E:
				{
					hash2 |= 0x80000000;
					break;
				}
			};
		}

		KxUtility::SetIfNotNull(correctedPath, path);
		return hash1 + hash2;
	}

	KxUtility::SetIfNotNull(correctedPath, wxEmptyString);
	return 0;
}

KBethesdaArchive::Status KBethesdaArchive::IsHeaderOK() const
{
	Status status = Status::Success;
	if (std::memcmp(m_Header.Signature, KBethesdaArchiveNS::Signature, sizeof(m_Header.Signature)) == 0)
	{
		if (m_Header.Offset == sizeof(m_Header))
		{
			if (m_Header.Version == (uint32_t)Version::Oblivion || m_Header.Version == (uint32_t)Version::Skyrim)
			{
				if (!IsFlagsSupported())
				{
					status = Status::ArchiveFlags;
				}
			}
			else
			{
				status = Status::Version;
			}
		}
		else
		{
			status = Status::Structure;
		}
	}
	else
	{
		status = Status::Signature;
	}
	return status;
}
bool KBethesdaArchive::IsFlagsSupported() const
{
	// Flag 'ArchiveFlags::DefaultCompressed' itself is supported, changing it isn't
	if ((m_Header.ArchiveFlags & (uint32_t)ArchiveFlags::REQUIRED))
	{
		ArchiveFlags unsupportedFlags = ArchiveFlags::None;
		if (m_Header.Version == (uint32_t)Version::Oblivion)
		{
			unsupportedFlags = ArchiveFlags::UNSUPPORTED_OBLIVION;
		}
		else if (m_Header.Version == (uint32_t)Version::Skyrim)
		{
			unsupportedFlags = ArchiveFlags::UNSUPPORTED_SKYRIM;
		}
		return !(m_Header.ArchiveFlags & (uint32_t)unsupportedFlags);
	}
	return false;
}

bool KBethesdaArchive::ReadHeader()
{
	if (m_Stream.ReadObject(m_Header))
	{
		m_Status = IsHeaderOK();
		m_IsHeaderOK = m_Status == Status::Success;
		if (!m_IsHeaderOK)
		{
			m_Header = Header();
		}
		return m_IsHeaderOK;
	}
	else
	{
		m_Status = Status::Stream;
		return false;
	}
}
bool KBethesdaArchive::ReadDirectoryRecods()
{
	bool success = false;
	m_DirectoryRecods.reserve(m_Header.FoldersCount + 1);
	for (size_t i = 0; i < m_Header.FoldersCount; i++)
	{
		m_DirectoryRecods.emplace_back(m_Stream.ReadObject<DirectoryRecord>());
		if (m_Stream.LastReadSuccess())
		{
			m_Status = Status::FolderRecords;
			return false;
		}
	}
	return true;
}
bool KBethesdaArchive::ReadFileRecords()
{
	bool isSuccess = false;
	m_FileRecords.reserve(m_Header.FilesCount + 1);

	char folderNameBuffer[255] = {0};
	for (size_t i = 0; i < m_Header.FoldersCount; i++)
	{
		// Read and save folder name
		uint8_t length = m_Stream.ReadObject<uint8_t>();

		m_Stream.ReadBuffer(folderNameBuffer, length);
		m_DirectoryRecods[i].Name = wxString::FromUTF8(folderNameBuffer, length - 1);

		// Read file record
		for (size_t filesRecordIndex = 0; filesRecordIndex < m_DirectoryRecods[i].Record.Count; filesRecordIndex++)
		{
			m_FileRecords.emplace_back(m_Stream.ReadObject<FileRecord>());
			if (!m_Stream.LastReadSuccess())
			{
				isSuccess = false;
				break;
			}
		}

		if (!isSuccess)
		{
			m_Status = Status::FileRecords;
			return false;
		}
	}
	return true;
}
bool KBethesdaArchive::ReadFileNamesRecords()
{
	char folderNameBuffer[255] = {0};
	char* folderName = folderNameBuffer;

	for (size_t i = 0; i < m_Header.FilesCount; i++)
	{
		char c = NULL;
		do
		{
			if (!m_Stream.ReadObject(c))
			{
				m_Status = Status::FileNames;
				return false;
			}

			*folderName = c;
			folderName++;
		}
		while (c);

		m_FileRecords[i].Name = wxString::FromUTF8(folderNameBuffer);
	}
	return true;
}
bool KBethesdaArchive::ReadFileDataRecords()
{
	m_OriginalSize = m_Stream.Tell();

	bool isSuccess = true;
	for (size_t i = 0; i < m_Header.FilesCount; i++)
	{
		const FileRecord& fileRecord = m_FileRecords[i].Record;
		m_Stream.Seek(fileRecord.Offset, KxFileStream::SeekMode::FromStart);
		if (IsFileNamesEmbedded(m_Header))
		{
			SkipBString(m_Stream);
		}

		FileData& fileData = m_FileRecords[i].Data;
		fileData.FileIndex = i;
		fileData.RawDataOffset = fileRecord.Offset;
		fileData.OriginalSize = fileRecord.Size;
		fileData.CompressedSize = fileRecord.Size;
		if (fileRecord.IsCompressed(m_Header))
		{
			// Read original size
			fileData.OriginalSize = m_Stream.ReadObject<uint32_t>();
			m_OriginalSize += fileData.OriginalSize;
		}
		else
		{
			// This size is original size
			m_OriginalSize += fileRecord.Size;
		}

		if (!isSuccess)
		{
			m_Status = Status::FileData;
			return false;
		}
	}
	return true;
}

bool KBethesdaArchive::OpenArchive(const wxString& filePath)
{
	if (m_Stream.Open(filePath, KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read))
	{
		if (ReadHeader() && ReadDirectoryRecods() && ReadFileRecords() && ReadFileNamesRecords() && ReadFileDataRecords())
		{
			m_Status = Status::Success;
			return true;
		}
		return false;
	}

	m_Status = Status::Stream;
	return false;
}
void KBethesdaArchive::CloseArchive()
{
	m_Stream.Close();
	m_Header = Header();
	m_DirectoryRecods.clear();
	m_FileRecords.clear();
	m_OriginalSize = -1;
	m_OriginalSize = -1;
	m_Status = Status::Unknown;
	m_IsHeaderOK = false;
}

KBethesdaArchive::KBethesdaArchive()
{
}
KBethesdaArchive::KBethesdaArchive(const wxString& filePath)
{
	OpenArchive(filePath);
}
KBethesdaArchive::~KBethesdaArchive()
{
	CloseArchive();
}

// KxIArchive
bool KBethesdaArchive::IsOK() const
{
	return m_IsHeaderOK && m_Status == Status::Success;
}
bool KBethesdaArchive::Open(const wxString& filePath)
{
	Close();
	return OpenArchive(filePath);
}
void KBethesdaArchive::Close()
{
	CloseArchive();
}
wxString KBethesdaArchive::GetFilePath() const
{
	return m_Stream.GetFileName();
}

size_t KBethesdaArchive::GetItemCount() const
{
	return m_Header.FilesCount;
}
wxString KBethesdaArchive::GetItemName(size_t index) const
{
	return index < m_Header.FilesCount ? m_FileRecords[index].Name : wxEmptyString;
}
int64_t KBethesdaArchive::GetOriginalSize() const
{
	return m_OriginalSize;
}
int64_t KBethesdaArchive::GetCompressedSize() const
{
	return m_Stream.GetSize();
}

// KxIArchiveSearch
void* KBethesdaArchive::FindFirstFile(const wxString& filter, KxFileItem& fileItem) const
{
	fileItem = KxFileItem();
	size_t nextIndex = (size_t)-1;
	if (SearchFiles(m_FileRecords, filter, fileItem, 0, nextIndex))
	{
		return new SearchData(filter, nextIndex);
	}
	return NULL;
}
bool KBethesdaArchive::FindNextFile(void* handle, KxFileItem& item) const
{
	item = KxFileItem();
	SearchData* searchData = reinterpret_cast<SearchData*>(handle);

	size_t nextIndex = (size_t)-1;
	if (SearchFiles(m_FileRecords, searchData->m_SearchQuery, item, searchData->m_NextIndex, nextIndex))
	{
		searchData->m_NextIndex = nextIndex;
		return true;
	}
	return false;
}
void KBethesdaArchive::FindClose(void* handle) const
{
	SearchData* searchData = reinterpret_cast<SearchData*>(handle);
	delete searchData;
}

// KxIArchiveExtraction
bool KBethesdaArchive::DoExtractAll(const wxString& directory) const
{
	return false;
}
bool KBethesdaArchive::DoExtractToDirectory(const IndexVector& indexes, const wxString& directory) const
{
	return false;
}
bool KBethesdaArchive::DoExtractToFiles(const IndexVector& indexes, const KxStringVector& filePaths) const
{
	return false;
}
KBethesdaArchive::Buffer KBethesdaArchive::DoExtractToMemory(size_t index) const
{
	return Buffer();
}
KBethesdaArchive::BufferMap KBethesdaArchive::DoExtractToMemory(const IndexVector& indexes) const
{
	return BufferMap();
}

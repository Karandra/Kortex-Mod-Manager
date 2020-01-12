#include "stdafx.h"
#include "Common.h"
#include "GenericArchive.h"

namespace Kortex::Archive
{
	Format DetectFormat(const wxString& filePath)
	{
		GenericArchive archive(filePath);
		if (archive)
		{
			return archive.GetProperty<Format>(KxArchive::Property::Compression_Format).value_or(Format::Unknown);
		}
		return Format::Unknown;
	}

	wxString GetExtensionFromFormat(Format format)
	{
		switch (format)
		{
			case Format::SevenZip:
			{
				return _T("7z");
			}
			case Format::Zip:
			{
				return _T("zip");
			}
			case Format::RAR:
			case Format::RAR5:
			{
				return _T("rar");
			}
			case Format::GZip:
			{
				return _T("gz");
			}
			case Format::BZip2:
			{
				return _T("bz2");
			}
			case Format::Tar:
			{
				return _T("tar");
			}
			case Format::LZMA:
			{
				return _T("lzma");
			}
			case Format::LZMA86:
			{
				return _T("lzma86");
			}
			case Format::CAB:
			{
				return _T("cab");
			}
			case Format::ISO:
			{
				return _T("iso");
			}
		}
		return {};
	}
	wxString GetFormatName(Format format)
	{
		switch (format)
		{
			case Format::SevenZip:
			{
				return wxS("7-Zip");
			}
			case Format::Zip:
			{
				return wxS("ZIP");
			}
			case Format::RAR:
			{
				return wxS("RAR");
			}
			case Format::RAR5:
			{
				return wxS("RAR 5");
			}
			case Format::GZip:
			{
				return wxS("GZip");
			}
			case Format::BZip2:
			{
				return wxS("BZip2");
			}
			case Format::Tar:
			{
				return wxS("TAR");
			}
			case Format::LZMA:
			{
				return wxS("LZMA");
			}
			case Format::LZMA86:
			{
				return wxS("LZMA 86");
			}
			case Format::CAB:
			{
				return wxS("CAB");
			}
			case Format::ISO:
			{
				return wxS("ISO");
			}
		}
		return {};
	}
}

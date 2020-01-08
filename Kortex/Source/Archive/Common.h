#pragma once
#include "stdafx.h"
#include <KxFramework/KxArchive.h>
#include <KxFramework/KxArchiveEvent.h>
#include <KxFramework/KxFileItem.h>

namespace Kortex::Archive
{
	enum class Method
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

namespace Kortex::Archive::Property
{
	//KxArchiveDeclareUserProperty(Compression, Solid);
}

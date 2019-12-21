#pragma once
#include "stdafx.h"
#include "Common.h"
#include "DataType.h"

namespace Kortex::GameConfig
{
	class IFileSource: public KxRTTI::Interface<IFileSource>
	{
		KxDecalreIID(IFileSource, {0xf07744eb, 0x2d9f, 0x475f, {0x8b, 0xfb, 0xa0, 0xb7, 0xd5, 0xe0, 0xad, 0x9e}});

		protected:
			wxString ResolveFSLocation(const wxString& path) const;

		public:
			virtual wxString GetFileName() const = 0;
			virtual wxString GetFilePath() const = 0;

			wxString GetExpandedFileName() const;
			wxString GetResolvedFilePath() const;
	};
}

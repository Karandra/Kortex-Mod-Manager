#pragma once
#include "stdafx.h"
#include "Common.h"
#include "DataType.h"

namespace Kortex::GameConfig
{
	class IFileSource: public KxRTTI::Interface<IFileSource>
	{
		protected:
			wxString ResolveFSLocation(const wxString& path) const;

		public:
			virtual wxString GetFileName() const = 0;
			virtual wxString GetFilePath() const = 0;

			wxString GetExpandedFileName() const;
			wxString GetResolvedFilePath() const;
	};
}

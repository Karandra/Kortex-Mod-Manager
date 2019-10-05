#pragma once
#include "stdafx.h"
#include "IPluginReader.h"

namespace Kortex::PluginManager
{
	struct BethesdaPluginData
	{
		enum HeaderFlags: uint32_t
		{
			None = 0,
			Master = 1 << 0,
			Localized = 1 << 7,
			Light = 1 << 9,
			Ignored = 1 << 12,
		};

		KxStringVector m_RequiredPlugins;
		wxString m_Author;
		wxString m_Description;
		HeaderFlags m_HeaderFlags = HeaderFlags::None;
		uint32_t m_FormVersion = 0;
	};

	class IBethesdaPluginReader: public KxRTTI::ExtendInterface<IBethesdaPluginReader, IPluginReader>
	{
		public:
			using HeaderFlags = BethesdaPluginData::HeaderFlags;

		public:
			virtual BethesdaPluginData& GetData() = 0;
	};
}

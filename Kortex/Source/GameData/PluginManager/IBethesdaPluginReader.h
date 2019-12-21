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
		KxDecalreIID(IBethesdaPluginReader, {0x91a369c6, 0x9fc4, 0x499b, {0x86, 0xb3, 0xa4, 0xd, 0x31, 0x1d, 0x50, 0xb1}});

		public:
			using HeaderFlags = BethesdaPluginData::HeaderFlags;

		public:
			virtual BethesdaPluginData& GetData() = 0;
	};
}

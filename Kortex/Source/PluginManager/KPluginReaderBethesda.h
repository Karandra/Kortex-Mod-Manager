#pragma once
#include "stdafx.h"
#include "KPluginReader.h"

class KPluginReaderBethesda: public KPluginReader
{
	public:
		enum HeaderFlags: uint32_t
		{
			None = 0,
			Master = 1 << 0,
			Localized = 1 << 7,
			Light = 1 << 9,
			Ignored = 1 << 12,
		};

	protected:
		uint32_t m_HeaderFlags = HeaderFlags::None;
		KxStringVector m_RequiredPlugins;
		wxString m_Author;
		wxString m_Description;

	public:
		const KxStringVector& GetRequiredPlugins() const
		{
			return m_RequiredPlugins;
		}
		const wxString& GetAuthor() const
		{
			return m_Author;
		}
		const wxString& GetDescription() const
		{
			return m_Description;
		}

		bool IsLocalized() const
		{
			return m_HeaderFlags & HeaderFlags::Localized;
		}
		bool IsMaster() const
		{
			return m_HeaderFlags & HeaderFlags::Master;
		}
		bool IsLight() const
		{
			return m_HeaderFlags & HeaderFlags::Light;
		}
		bool IsNormal() const
		{
			return !IsMaster() && !IsLight();
		}
};

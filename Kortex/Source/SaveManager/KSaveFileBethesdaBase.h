#pragma once
#include "stdafx.h"
#include "KSaveFile.h"

class KSaveFileBethesdaBase: public KSaveFile
{
	public:
		using float32_t = float;
		using float64_t = double;

	protected:
		KLabeledValueArray m_BasicInfo;
		KxStringVector m_PluginsList;
		wxBitmap m_Bitmap;
		uint32_t m_SaveVersion = 0;

	public:
		virtual const KLabeledValueArray& GetBasicInfo() const override
		{
			return m_BasicInfo;
		}
		virtual const KxStringVector& GetPluginsList() const override
		{
			return m_PluginsList;
		}
		virtual const wxBitmap& GetBitmap() const override
		{
			return m_Bitmap;
		}

		uint32_t GetSaveVersion() const
		{
			return m_SaveVersion;
		}
};

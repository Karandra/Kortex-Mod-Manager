#pragma once
#include "stdafx.h"
#include "../IBethesdaGameSave.h"

namespace Kortex::SaveManager::BethesdaSave
{
	class BaseSave: public IBethesdaGameSave
	{
		public:
			using float32_t = float;
			using float64_t = double;

		protected:
			KLabeledValue::Vector m_BasicInfo;
			KxStringVector m_PluginsList;
			wxBitmap m_Bitmap;
			uint32_t m_SaveVersion = 0;

		public:
			virtual wxBitmap GetBitmap() const override
			{
				return m_Bitmap;
			}
			virtual KLabeledValue::Vector GetBasicInfo() const override
			{
				return m_BasicInfo;
			}

			virtual KxStringVector GetPlugins() const override
			{
				return m_PluginsList;
			}
			virtual size_t GetPluginsCount() const override
			{
				return m_PluginsList.size();
			}
			virtual bool HasPlugins() const override
			{
				return !m_PluginsList.empty();
			}

			virtual uint32_t GetVersion() const override
			{
				return m_SaveVersion;
			}
	};
}

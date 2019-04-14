#pragma once
#include "stdafx.h"
#include "GameData/SaveManager/IBethesdaGameSave.h"

namespace Kortex::SaveManager::BethesdaSave
{
	class BethesdaBasicSave: public IBethesdaGameSave
	{
		public:
			using float32_t = float;
			using float64_t = double;

		protected:
			SaveInfoPair::Vector m_BasicInfo;
			KxStringVector m_PluginsList;
			wxBitmap m_Bitmap;
			uint32_t m_SaveVersion = 0;

		protected:
			void SortBasicInfo();

		public:
			wxBitmap GetBitmap() const override
			{
				return m_Bitmap;
			}
			const InfoPairVector& GetBasicInfo() const override
			{
				return m_BasicInfo;
			}

			KxStringVector GetPlugins() const override
			{
				return m_PluginsList;
			}
			size_t GetPluginsCount() const override
			{
				return m_PluginsList.size();
			}
			bool HasPlugins() const override
			{
				return !m_PluginsList.empty();
			}

			uint32_t GetVersion() const override
			{
				return m_SaveVersion;
			}
			wxString GetDisplayName() const override;
	};
}

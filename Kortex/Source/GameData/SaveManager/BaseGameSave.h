#pragma once
#include "stdafx.h"
#include "GameData/IGameSave.h"

namespace Kortex::SaveManager
{
	class BaseGameSave: public IGameSave
	{
		private:
			KxFileItem m_Item;
			wxBitmap m_Thumb;
			bool m_FileRead = false;
			bool m_IsOK = false;

		protected:
			wxBitmap GetThumbBitmap() const override
			{
				return m_Thumb;
			}
			bool HasThumbBitmap() const override
			{
				return m_Thumb.IsOk();
			}
			void SetThumbBitmap(const wxBitmap& bitmap) override
			{
				m_Thumb = bitmap;
			}
			void ResetThumbBitmap() override
			{
				m_Thumb = wxNullBitmap;
			}

			virtual bool OnCreate(KxFileItem& fileItem) override
			{
				return fileItem.IsOK();
			}

		public:
			bool IsOK() const override;
			bool Create(const wxString& filePath) override;
			bool ReadFile() override;

			const KxFileItem& GetFileItem() const override
			{
				return m_Item;
			}
			KxFileItem& GetFileItem() override
			{
				return m_Item;
			}
	};
}

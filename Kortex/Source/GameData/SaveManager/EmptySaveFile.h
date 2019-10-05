#pragma once
#include "stdafx.h"
#include "BaseGameSave.h"

namespace Kortex::SaveManager
{
	class EmptySaveFile: public KxRTTI::ExtendInterface<EmptySaveFile, BaseGameSave>
	{
		private:
			InfoPairVector m_Info;

		protected:
			virtual bool OnRead(const KxFileItem& fileItem) override
			{
				return fileItem.IsOK();
			}

		public:
			virtual wxBitmap GetBitmap() const override
			{
				return wxNullBitmap;
			}
			virtual const InfoPairVector& GetBasicInfo() const override
			{
				return m_Info;
			}
			wxString GetDisplayName() const override
			{
				return {};
			}
	};
}

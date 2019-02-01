#pragma once
#include "stdafx.h"
#include "BaseGameSave.h"

namespace Kortex::SaveManager
{
	class EmptySaveFile: public RTTI::IExtendInterface<EmptySaveFile, BaseGameSave>
	{
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
			virtual KLabeledValue::Vector GetBasicInfo() const override
			{
				return {};
			}
	};
}

#pragma once
#include "stdafx.h"
#include "KSMSaveFile.h"

class KSMSaveFileBethesdaSkyrim: public KSMSaveFile
{
	private:
		KLabeledValueArray m_BasicInfo;
		KxStringVector m_PluginsList;
		wxBitmap m_Bitmap;

	protected:
		virtual bool DoReadData() override;

	public:
		KSMSaveFileBethesdaSkyrim(const wxString& filePath)
			:KSMSaveFile(filePath)
		{
		}

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
};

#pragma once
#include "stdafx.h"
#include "KSMSaveFile.h"

class KSMSaveFileBethesdaFallout4: public KSMSaveFile
{
	public:
		static wxImage ReadImageRGBA(const std::vector<unsigned char>& RGBAData, int width, int height);

	private:
		KLabeledValueArray m_BasicInfo;
		KxStringVector m_PluginsList;
		wxBitmap m_Bitmap;

	protected:
		virtual bool DoReadData() override;

	public:
		KSMSaveFileBethesdaFallout4(const wxString& filePath)
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

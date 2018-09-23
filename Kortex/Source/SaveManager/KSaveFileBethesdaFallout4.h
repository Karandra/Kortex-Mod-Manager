#pragma once
#include "stdafx.h"
#include "KSaveFile.h"

class KSaveFileBethesdaFallout4: public KSaveFile
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
		KSaveFileBethesdaFallout4(const wxString& filePath)
			:KSaveFile(filePath)
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

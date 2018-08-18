#pragma once
#include "stdafx.h"
#include "KLabeledValue.h"
#include <KxFramework/KxFileFinder.h>

class KSMSaveFile: public wxClientData
{
	private:
		KxFileFinderItem m_FileInfo;
		wxBitmap m_Thumb;
		bool m_IsDataRead = false;
		bool m_IsOK = false;

	protected:
		virtual bool DoReadData() = 0;

	public:
		KSMSaveFile(const wxString& filePath);
		virtual ~KSMSaveFile();

	public:
		bool IsOK() const
		{
			return m_IsOK;
		}
		void ReadData()
		{
			if (!m_IsDataRead)
			{
				m_IsOK = DoReadData();
				m_IsDataRead = true;
			}
		}

		virtual const KLabeledValueArray& GetBasicInfo() const = 0;
		virtual const KxStringVector& GetPluginsList() const = 0;
		virtual const wxBitmap& GetBitmap() const
		{
			return wxNullBitmap;
		}
		bool HasPluginsList() const
		{
			return !GetPluginsList().empty();
		}

		const KxFileFinderItem& GetFileInfo() const
		{
			return m_FileInfo;
		}
		KxFileFinderItem& GetFileInfo()
		{
			return m_FileInfo;
		}
		wxString GetFilePath() const
		{
			return m_FileInfo.GetFullPath();
		}
		
		const wxBitmap& GetThumbBitmap() const
		{
			return m_Thumb;
		}
		const wxBitmap& CreateThumbBitmap(int width, int height = -1);
		bool HasThumbBitmap() const
		{
			return m_Thumb.IsOk();
		}
		void ResetThumbBitmap()
		{
			m_Thumb = wxNullBitmap;
		}
};
typedef std::vector<std::unique_ptr<KSMSaveFile>> KSMSaveFileArray;

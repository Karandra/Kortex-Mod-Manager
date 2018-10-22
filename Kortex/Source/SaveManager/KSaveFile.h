#pragma once
#include "stdafx.h"
#include "KLabeledValue.h"
#include "KWithBitmap.h"
#include <KxFramework/KxFileFinder.h>

class KSaveFile
{
	public:
		using Vector = std::vector<std::unique_ptr<KSaveFile>>;
		using RefVector = std::vector<KSaveFile*>;
		using CRefVector = std::vector<const KSaveFile*>;

	public:
		static wxImage ReadImageRGB(const KxUInt8Vector& rgbData, int width, int height, int alphaOverride = -1, bool isStaticData = false);
		static wxImage ReadImageRGBA(const KxUInt8Vector& rgbaData, int width, int height, int alphaOverride = -1);

	private:
		KxFileItem m_FileInfo;
		wxBitmap m_Thumb;
		bool m_IsDataRead = false;
		bool m_IsOK = false;

	protected:
		virtual bool DoInitializeSaveData() = 0;

	protected:
		KSaveFile() = default;

	public:
		virtual ~KSaveFile() = default;

	public:
		bool IsOK() const
		{
			return m_IsOK;
		}
		bool Create(const wxString& filePath);
		void InitializeSaveData()
		{
			if (!m_IsDataRead)
			{
				m_IsOK = DoInitializeSaveData();
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

		const KxFileItem& GetFileInfo() const
		{
			return m_FileInfo;
		}
		KxFileItem& GetFileInfo()
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
		bool HasThumbBitmap() const
		{
			return m_Thumb.IsOk();
		}
		void SetThumbBitmap(const wxBitmap& bitmap)
		{
			m_Thumb = bitmap;
		}
		void ResetThumbBitmap()
		{
			m_Thumb = wxNullBitmap;
		}
};

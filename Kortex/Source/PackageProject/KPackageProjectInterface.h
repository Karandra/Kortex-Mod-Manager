#pragma once
#include "stdafx.h"
#include "KPackageProjectPart.h"
#include "KLabeledValue.h"
#include "KWithBitmap.h"
#include <KxFramework/KxColor.h>

class KPPIImageEntry: public KWithBitmap
{
	private:
		wxString m_Path;
		wxString m_Description;
		bool m_IsVisiable = true;
		bool m_FadeEnabled = false;
		wxSize m_Size = wxDefaultSize;

	public:
		KPPIImageEntry(const wxString& path = wxEmptyString, const wxString& description = wxEmptyString, bool isVisible = true);
		~KPPIImageEntry();

	public:
		bool HasPath() const
		{
			return !m_Path.IsEmpty();
		}
		const wxString& GetPath() const
		{
			return m_Path;
		}
		void SetPath(const wxString& value)
		{
			m_Path = value;
		}

		bool HasDescription() const
		{
			return !m_Description.IsEmpty();
		}
		const wxString& GetDescriptionRaw() const
		{
			return m_Description;
		}
		wxString GetDescription() const
		{
			return HasDescription() ? GetDescriptionRaw() : GetPath().AfterLast('\\');
		}
		void SetDescription(const wxString& label)
		{
			m_Description = label;
		}

		bool IsVisible() const
		{
			return m_IsVisiable;
		}
		void SetVisible(bool value)
		{
			m_IsVisiable = value;
		}

		bool IsFadeEnabled() const
		{
			return m_FadeEnabled;
		}
		void SetFadeEnabled(bool value)
		{
			m_FadeEnabled = value;
		}

		wxSize GetSize() const
		{
			return m_Size;
		}
		void SetSize(const wxSize& tSize)
		{
			m_Size = tSize;
		}
};
typedef std::vector<KPPIImageEntry> KPPIImageEntryArray;

//////////////////////////////////////////////////////////////////////////
class KPPITitleConfig
{
	public:
		static const wxAlignment ms_InvalidAlignment = wxALIGN_INVALID;

	private:
		wxAlignment m_Alignment = ms_InvalidAlignment;
		KxColor m_Color = wxNullColour;

	public:
		KPPITitleConfig(wxAlignment alignment = ms_InvalidAlignment, const KxColor& color = wxNullColour)
			:m_Alignment(alignment), m_Color(color)
		{
		}

	public:
		bool IsOK() const
		{
			return HasAlignment() && HasColor();
		}

		bool HasAlignment() const
		{
			return m_Alignment != ms_InvalidAlignment;
		}
		wxAlignment GetAlignment() const
		{
			return HasAlignment() ? m_Alignment : wxALIGN_NOT;
		}
		void SetAlignment(wxAlignment value)
		{
			m_Alignment = value;
		}

		bool HasColor() const
		{
			return m_Color.IsOk();
		}
		KxColor GetColor() const
		{
			return m_Color;
		}
		void SetColor(const KxColor& value)
		{
			m_Color = value;
		}
};

//////////////////////////////////////////////////////////////////////////
class KPackageProjectInterface: public KPackageProjectPart
{
	private:
		wxString m_MainImage;
		wxString m_HeaderImage;
		KPPIImageEntryArray m_Images;
		KPPITitleConfig m_TitleConfig;

	public:
		KPackageProjectInterface(KPackageProject& project);
		virtual ~KPackageProjectInterface();

	public:
		const KPPIImageEntryArray& GetImages() const
		{
			return m_Images;
		}
		KPPIImageEntryArray& GetImages()
		{
			return m_Images;
		}
		
		const wxString& GetMainImage() const
		{
			return m_MainImage;
		}
		void SetMainImage(const wxString& path)
		{
			m_MainImage = path;
		}

		const wxString& GetHeaderImage() const
		{
			return m_HeaderImage;
		}
		void SetHeaderImage(const wxString& path)
		{
			m_HeaderImage = path;
		}

		const KPPIImageEntry* FindEntryWithValue(const wxString& path) const;
		KPPIImageEntry* FindEntryWithValue(const wxString& path);
		
		const KPPIImageEntry* GetMainImageEntry() const
		{
			return FindEntryWithValue(m_MainImage);
		}
		KPPIImageEntry* GetMainImageEntry()
		{
			return FindEntryWithValue(m_MainImage);
		}

		const KPPIImageEntry* GetHeaderImageEntry() const
		{
			return FindEntryWithValue(m_HeaderImage);
		}
		KPPIImageEntry* GetHeaderImageEntry()
		{
			return FindEntryWithValue(m_HeaderImage);
		}

		const KPPITitleConfig& GetTitleConfig() const
		{
			return m_TitleConfig;
		}
		KPPITitleConfig& GetTitleConfig()
		{
			return m_TitleConfig;
		}
};

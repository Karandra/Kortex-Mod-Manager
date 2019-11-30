#pragma once
#include "stdafx.h"
#include "ProjectSection.h"
#include "Utility/KLabeledValue.h"
#include "Utility/KWithBitmap.h"
#include <KxFramework/KxColor.h>

namespace Kortex::PackageProject
{
	class ImageItem: public KWithBitmap
	{
		public:
			using Vector = std::vector<ImageItem>;

		private:
			wxString m_Path;
			wxString m_Description;
			bool m_IsVisiable = true;
			bool m_FadeEnabled = false;
			wxSize m_Size = wxDefaultSize;
	
		public:
			ImageItem(const wxString& path = wxEmptyString, const wxString& description = wxEmptyString, bool isVisible = true);
			~ImageItem();
	
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
}

namespace Kortex::PackageProject
{
	class TitleConfig
	{
		public:
			static const wxAlignment ms_InvalidAlignment = wxALIGN_INVALID;
			
		private:
			wxAlignment m_Alignment = ms_InvalidAlignment;
			KxColor m_Color = wxNullColour;
	
		public:
			TitleConfig(wxAlignment alignment = ms_InvalidAlignment, const KxColor& color = wxNullColour)
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
}

namespace Kortex::PackageProject
{
	class InterfaceSection: public ProjectSection
	{
		private:
			wxString m_MainImage;
			wxString m_HeaderImage;
			ImageItem::Vector m_Images;
			TitleConfig m_TitleConfig;
			
		public:
			InterfaceSection(ModPackageProject& project);
			~InterfaceSection();
	
		public:
			const ImageItem::Vector& GetImages() const
			{
				return m_Images;
			}
			ImageItem::Vector& GetImages()
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
	
			const ImageItem* FindImageByPath(const wxString& path) const;
			ImageItem* FindImageByPath(const wxString& path);
			
			const ImageItem* GetMainItem() const
			{
				return FindImageByPath(m_MainImage);
			}
			ImageItem* GetMainItem()
			{
				return FindImageByPath(m_MainImage);
			}
	
			const ImageItem* GetHeaderItem() const
			{
				return FindImageByPath(m_HeaderImage);
			}
			ImageItem* GetHeaderItem()
			{
				return FindImageByPath(m_HeaderImage);
			}
	
			const TitleConfig& GetTitleConfig() const
			{
				return m_TitleConfig;
			}
			TitleConfig& GetTitleConfig()
			{
				return m_TitleConfig;
			}
	};
}

#pragma once;
#include "stdafx.h"
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxStdDialog.h>
#include <KxFramework/KxSlider.h>
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxSplitterWindow.h>
#include <Kx/EventSystem/Event.h>
#include "Archive/KArchive.h"

namespace Kortex::UI
{
	class KImageViewerEvent: public wxNotifyEvent
	{
		public:
			enum Type
			{
				FilePath,
				BitmapPtr,
				InputStream,
			};
			
		public:
			KxEVENT_MEMBER(KImageViewerEvent, Next);
			KxEVENT_MEMBER(KImageViewerEvent, Previous);

		private:
			std::variant<wxString, wxBitmap, const KArchive::Buffer*> m_Data;
			bool m_HasPrev = false;
			bool m_HasNext = false;
			
		private:
			wxString GetString() const = delete;
			void SetString(const wxString& s) = delete;
			
		public:
			KImageViewerEvent() = default;
			KImageViewerEvent(wxEventType type, const wxBitmap& bitmap)
				:wxNotifyEvent(type)
			{
				SetBitmap(bitmap);
			}
			KImageViewerEvent(wxEventType type, const wxString& filePath)
				:wxNotifyEvent(type)
			{
				SetFilePath(filePath);
			}
			KImageViewerEvent(wxEventType type, const KArchive::Buffer& buffer)
				:wxNotifyEvent(type)
			{
				SetInputStream(buffer);
			}

		public:
			KImageViewerEvent* Clone() const override
			{
				return new KImageViewerEvent(*this);
			}
			Type GetType() const
			{
				return (Type)m_Data.index();
			}
			
			bool HasPrev() const
			{
				return m_HasPrev;
			}
			bool HasNext() const
			{
				return m_HasNext;
			}
			void SetHasPrevNext(bool bPrev, bool bNext)
			{
				m_HasPrev = bPrev;
				m_HasNext = bNext;
			}
			
			wxString GetDescription() const
			{
				return wxNotifyEvent::GetString();
			}
			void SetDescription(const wxString& s)
			{
				wxNotifyEvent::SetString(s);
			}
			
			bool HasBitmap() const;
			wxBitmap GetBitmap() const;
			void SetBitmap(const wxBitmap& bitmap);
	
			bool IsAnimationFile() const;
			bool HasFilePath() const;
			wxString GetFilePath() const;
			void SetFilePath(const wxString& filePath);
			
			bool IsInputStream() const;
			wxMemoryInputStream GetInputSteram();
			void SetInputStream(const KArchive::Buffer& buffer);
	};
}

namespace Kortex::UI
{
	class KImageViewerDialog: public KxStdDialog
	{
		private:
			KxSplitterWindow* m_Splitter = nullptr;
			KxImageView* m_ImageView = nullptr;
			KxHTMLWindow* m_Description = nullptr;
			KxAuiToolBar* m_ToolBar = nullptr;
			KxAuiToolBarItem* m_Backward = nullptr;
			KxAuiToolBarItem* m_Forward = nullptr;
			KxSlider* m_ScaleSlider = nullptr;
			wxString m_FilePath;
	
			wxColourPickerCtrl* m_ColorBGCtrl = nullptr;
			wxColourPickerCtrl* m_ColorFGCtrl = nullptr;
	
		private:
			int GetViewSizerProportion() const override
			{
				return 1;
			}
			wxOrientation GetViewSizerOrientation() const override
			{
				return wxVERTICAL;
			}
			wxOrientation GetViewLabelSizerOrientation() const override
			{
				return wxHORIZONTAL;
			}
			bool IsEnterAllowed(wxKeyEvent& event, wxWindowID* id = nullptr) const override
			{
				return true;
			}
			wxWindow* GetDialogMainCtrl() const override
			{
				return m_Splitter;
			}
			void ResetState()
			{
				m_ImageView->SetBitmap(wxNullBitmap);
			}
			bool OnDynamicBind(wxDynamicEventTableEntry& entry) override;
	
			void OnLoadFromDisk(const wxString& filePath);
			void OnNavigation(wxAuiToolBarEvent& event);
			void OnAcceptNavigation(KImageViewerEvent& event);
			void OnScaleChanged(wxCommandEvent& event);
			void OnSaveImage(wxCommandEvent& event);
			void OnChangeColor(wxColourPickerEvent& event);
	
		public:
			KImageViewerDialog(wxWindow* parent, const wxString& caption = wxEmptyString);
			~KImageViewerDialog();
	
		public:
			bool Create(wxWindow* parent, const wxString& caption = wxEmptyString);
			void Navigate(KImageViewerEvent& event)
			{
				OnAcceptNavigation(event);
			}
	};
}

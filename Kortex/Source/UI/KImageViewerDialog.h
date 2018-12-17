#pragma once;
#include "stdafx.h"
#include "KImageProvider.h"
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxStdDialog.h>
#include <KxFramework/KxSlider.h>
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxHTMLWindow.h>
#include "Archive/KArchive.h"
#include <KxFramework/KxSplitterWindow.h>

class KImageViewerEvent: public wxNotifyEvent
{
	public:
		enum Type
		{
			FilePath,
			BitmapPtr,
			InputStream,
		};

	private:
		std::variant<wxString, const wxBitmap*, const KArchive::Buffer*> m_Data;
		bool m_HasPrev = false;
		bool m_HasNext = false;

	private:
		wxString GetString() const = delete;
		void SetString(const wxString& s) = delete;

	public:
		KImageViewerEvent() {}
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
		virtual ~KImageViewerEvent()
		{
		}

		virtual KImageViewerEvent* Clone() const override
		{
			return new KImageViewerEvent(*this);
		}

	public:
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
		const wxBitmap& GetBitmap() const;
		void SetBitmap(const wxBitmap& bitmap);

		bool IsAnimationFile() const;
		bool HasFilePath() const;
		const wxString& GetFilePath() const;
		void SetFilePath(const wxString& filePath);

		bool IsInputStream() const;
		wxMemoryInputStream GetInputSteram();
		void SetInputStream(const KArchive::Buffer& buffer);
};

wxDECLARE_EVENT(KEVT_IMAGEVIEWER_NEXT_IMAGE, KImageViewerEvent);
wxDECLARE_EVENT(KEVT_IMAGEVIEWER_PREV_IMAGE, KImageViewerEvent);

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
		virtual int GetViewSizerProportion() const override
		{
			return 1;
		}
		virtual wxOrientation GetViewSizerOrientation() const override
		{
			return wxVERTICAL;
		}
		virtual wxOrientation GetViewLabelSizerOrientation() const override
		{
			return wxHORIZONTAL;
		}
		virtual bool IsEnterAllowed(wxKeyEvent& event, wxWindowID* id = nullptr) const override
		{
			return true;
		}
		virtual wxWindow* GetDialogMainCtrl() const override
		{
			return m_Splitter;
		}
		virtual void ResetState()
		{
			m_ImageView->SetBitmap(wxNullBitmap);
		}
		virtual bool OnDynamicBind(wxDynamicEventTableEntry& entry) override;

		void OnLoadFromDisk(const wxString& filePath);
		void OnNavigation(wxAuiToolBarEvent& event);
		void OnAcceptNavigation(KImageViewerEvent& event);
		void OnScaleChanged(wxCommandEvent& event);
		void OnSaveImage(wxCommandEvent& event);
		void OnChangeColor(wxColourPickerEvent& event);

	public:
		bool Create(wxWindow* parent, const wxString& caption = wxEmptyString);
		KImageViewerDialog(wxWindow* parent, const wxString& caption = wxEmptyString);
		virtual ~KImageViewerDialog();

	public:
		void Navigate(KImageViewerEvent& event)
		{
			OnAcceptNavigation(event);
		}
};

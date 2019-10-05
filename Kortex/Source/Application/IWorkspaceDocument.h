#pragma once
#include "stdafx.h"
#include <Kx/EventSystem/Event.h>
#include <Kx/RTTI.hpp>

namespace Kortex
{
	class IWorkspaceDocument: public KxRTTI::Interface<IWorkspaceDocument>
	{
		friend class KxIObject;

		public:
			KxEVENT_MEMBER(wxNotifyEvent, Changed);
			KxEVENT_MEMBER(wxNotifyEvent, Saved);
			KxEVENT_MEMBER(wxNotifyEvent, Discarded);

		protected:
			virtual wxString GetSaveConfirmationCaption() const;
			virtual wxString GetSaveConfirmationMessage() const;

		public:
			virtual ~IWorkspaceDocument() = default;

		public:
			virtual KxStandardID AskForSave(bool canCancel = true);
			virtual bool HasUnsavedChanges() const = 0;
			virtual void SaveChanges() = 0;
			virtual void DiscardChanges() = 0;
	};
}

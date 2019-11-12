#include "stdafx.h"
#include "IWorkspaceDocument.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxString.h>

namespace Kortex
{
	wxString IWorkspaceDocument::GetSaveConfirmationCaption() const
	{
		const IWorkspace* workspace = QueryInterface<IWorkspace>();
		return wxString::Format(wxS("%s - %s"), workspace->GetName(), KxString::ToLower(KTr("Controller.SaveChanges.Caption")));
	}
	wxString IWorkspaceDocument::GetSaveConfirmationMessage() const
	{
		return KTr("Controller.SaveChanges.Message");
	}

	KxStandardID IWorkspaceDocument::AskForSave(bool canCancel)
	{
		IWorkspace* workspace = QueryInterface<IWorkspace>();
		if (workspace && HasUnsavedChanges())
		{
			KxTaskDialog dialog(&workspace->GetWindow(), KxID_NONE, GetSaveConfirmationCaption(), GetSaveConfirmationMessage(), canCancel ? KxBTN_CANCEL : KxBTN_NONE);
			dialog.SetMainIcon(KxICON_WARNING);
			dialog.AddButton(KxID_YES, KTr("Controller.SaveChanges.Save"));
			dialog.AddButton(KxID_NO, KTr("Controller.SaveChanges.Discard"));

			wxWindowID ret = dialog.ShowModal();
			if (ret == KxID_YES)
			{
				SaveChanges();
			}
			else if (ret == KxID_NO)
			{
				DiscardChanges();
			}
			else
			{
				return KxID_CANCEL;
			}
		}
		return KxID_OK;
	}
}

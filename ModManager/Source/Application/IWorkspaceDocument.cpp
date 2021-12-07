#include "pch.hpp"
#include "IWorkspaceDocument.h"
#include "IWorkspace.h"
#include "IApplication.h"
#include "Localization.h"
#include <kxf/UI/Dialogs/TaskDialog.h>
#include <kxf/UI/IWidget.h>

namespace Kortex
{
	kxf::String IWorkspaceDocument::GetSaveConfirmationCaption() const
	{
		auto workspace = QueryInterface<IWorkspace>();
		return kxf::Format(kxS("{} – {}"), workspace->GetName(), Localize("WorkspaceDocument.SaveChanges.Caption"));
	}
	kxf::String IWorkspaceDocument::GetSaveConfirmationMessage() const
	{
		return Localize("WorkspaceDocument.SaveChanges.Message");
	}

	kxf::StdID IWorkspaceDocument::AskForSave(bool canCancel)
	{
		using namespace kxf;

		auto workspace = QueryInterface<IWorkspace>();
		if (workspace && HasUnsavedChanges())
		{
			UI::TaskDialog dialog(workspace->GetWidget().GetWxWindow(), wxID_NONE, GetSaveConfirmationCaption(), GetSaveConfirmationMessage(), canCancel ? StdButton::Cancel : StdButton::None);
			dialog.SetMainIcon(StdIcon::Warning);
			dialog.AddButton(StdID::Yes, Localize("Controller.SaveChanges.Save"));
			dialog.AddButton(StdID::No, Localize("Controller.SaveChanges.Discard"));

			WidgetID ret = dialog.ShowModal();
			if (ret == StdID::Yes)
			{
				SaveChanges();
			}
			else if (ret == StdID::No)
			{
				DiscardChanges();
			}
			else
			{
				return StdID::Cancel;
			}
		}
		return StdID::OK;
	}
}

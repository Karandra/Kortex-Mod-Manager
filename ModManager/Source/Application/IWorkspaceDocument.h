#pragma once
#include "Framework.hpp"

namespace Kortex
{
	class KORTEX_API IWorkspaceDocument: public kxf::RTTI::Interface<IWorkspaceDocument>
	{
		KxRTTI_DeclareIID(IWorkspaceDocument, {0xc775ca40, 0xfcd2, 0x4fdf, {0xb0, 0x15, 0xc9, 0x5b, 0x3d, 0x13, 0x54, 0x2e}});

		public:
			void OnSaved()
			{
				QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IWorkspaceDocument::OnSaved);
			}
			void OnChanged()
			{
				QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IWorkspaceDocument::OnChanged);
			}
			void OnDiscarded()
			{
				QueryInterface<kxf::IEvtHandler>()->ProcessSignal(&IWorkspaceDocument::OnDiscarded);
			}

		protected:
			virtual kxf::String GetSaveConfirmationCaption() const;
			virtual kxf::String GetSaveConfirmationMessage() const;

		public:
			virtual ~IWorkspaceDocument() = default;

		public:
			virtual kxf::StdID AskForSave(bool canCancel = true);
			virtual bool HasUnsavedChanges() const = 0;
			virtual void SaveChanges() = 0;
			virtual void DiscardChanges() = 0;
	};
}

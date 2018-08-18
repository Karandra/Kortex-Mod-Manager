#pragma once
#include "stdafx.h"
#include "ConfigManager/KCMController.h"
class KxTreeList;
class KCMConfigEntryPath;
class KCMConfigEntryStd;
class KConfigManager;
class KSettingsWorkspace;

class KSettingsWindowController: public KCMController
{
	private:
		KSettingsWorkspace* m_Workspace = NULL;

	protected:
		virtual KxTreeListItem CreateUnknownItemsRoot() override;

	public:
		KSettingsWindowController(KSettingsWorkspace* workspace, KxTreeList* view);
		virtual ~KSettingsWindowController();

	private:
		virtual KxStringVector OnFormatEntryToView(KCMConfigEntryPath* pathEntry) override;
		virtual KxStringVector OnFormatEntryToView(KCMConfigEntryStd* stdEntry) override;

	public:
		virtual int GetValueNameColumn() const override
		{
			return 0;
		}
		virtual int GetValueDataColumn() const override
		{
			return 1;
		}
		virtual wxWindow* GetParentWindow() override;

		virtual bool ShouldExpandTopLevelNodes() const override
		{
			return true;
		}
		virtual bool IsUnknownEntriesVisible() const override
		{
			return false;
		}
		virtual bool IsENBAllowed() const override
		{
			return false;
		}
		virtual bool IsEditable() const override
		{
			return true;
		}
		virtual bool IsCreateEntryAllowed() const override
		{
			return false;
		}
		virtual bool IsRemoveEntryAllowed() const override
		{
			return false;
		}
		virtual bool IsRemovePathAllowed() const override
		{
			return false;
		}
};

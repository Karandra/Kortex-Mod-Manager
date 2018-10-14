#pragma once
#include "stdafx.h"
#include "GameInstance/KGameID.h"
class KOperationWithProgressDialogBase;

class KModManagerImport
{
	public:
		enum Type
		{
			ModOrganizer,
			NexusModManager,
			Vortex,
		};
		
		static std::unique_ptr<KModManagerImport> Create(Type type);
		static void ShowImportDialog(Type type, wxWindow* window);

	private:
		wxString m_ProfileToImport;
		bool m_ShouldSkipExistingMods = false;

	protected:
		wxString GetProfileMatchingMessage(KxIconType* icon = NULL) const;
		
		const wxString& GetProfileToImport() const
		{
			return m_ProfileToImport;
		}
		bool ShouldSkipExistingMods() const
		{
			return m_ShouldSkipExistingMods;
		}
		void SkipExistingMods(bool skip = true)
		{
			m_ShouldSkipExistingMods = skip;
		}

	public:
		virtual ~KModManagerImport();

	public:
		void SetProfileToImport(const wxString& name)
		{
			m_ProfileToImport = name;
		}

	public:
		// Sets mod manager data directory. For MO this is the 'instance' folder.
		virtual void SetDirectory(const wxString& path) = 0;

		// Implement actual import here.
		virtual void Import(KOperationWithProgressDialogBase* context) = 0;

		// Check if data can be imported at all.
		virtual bool CanImport() const = 0;

		// Retrieve target profile. Invalid ID is file, this just means
		// that data can be imported into any game.
		virtual KGameID GetTargetProfileID() const = 0;

		// Return mod manager name.
		virtual wxString GetModManagerName() const = 0;

		// Any additional info.
		virtual wxString GetAdditionalInfo() const
		{
			return wxEmptyString;
		}

		// Get current used mod list or, if the mod manager doesn't support this
		// return empty string.
		virtual wxString GetCurrentModList() const
		{
			return wxEmptyString;
		}

		// If target mod-manager supports profiles implement this function.
		virtual KxStringVector GetProfilesList() const
		{
			return KxStringVector();
		}
};

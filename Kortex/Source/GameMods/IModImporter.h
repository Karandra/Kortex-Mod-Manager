#pragma once
#include "stdafx.h"
#include "GameInstance/GameID.h"
class KOperationWithProgressDialogBase;

namespace Kortex
{
	class IModImporter
	{
		public:
			enum class Type
			{
				ModOrganizer,
				NexusModManager,
				Vortex,
			};
		
			static std::unique_ptr<IModImporter> CreateImporter(Type type);
			static void PerformImport(Type type, wxWindow* window);

		private:
			wxString m_SelectedProfile;
			bool m_ShouldSkipExistingMods = false;

		protected:
			wxString GetProfileMatchingMessage(KxIconType* icon = nullptr) const;
		
			const wxString& GetSelectedProfile() const
			{
				return m_SelectedProfile;
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
			virtual ~IModImporter();

		public:
			// Sets mod manager data directory. For MO this is the instance folder.
			virtual void SetDirectory(const wxString& path) = 0;

			// Implement actual import here.
			virtual void Import(KOperationWithProgressDialogBase* context) = 0;

			// Check if data can be imported at all.
			virtual bool CanImport() const = 0;

			// Retrieve target profile. Value of 'GameIDs::NullGameID' is fine, it just means
			// that data can be imported into any game.
			virtual GameID GetTargetGameID() const = 0;

			// Return mod manager name.
			virtual wxString GetModManagerName() const = 0;

			// Any additional info.
			virtual wxString GetAdditionalInfo() const
			{
				return wxEmptyString;
			}

			// Get currently used profile or, if the mod manager doesn't support this, return empty string.
			virtual wxString GetCurrentProfile() const
			{
				return wxEmptyString;
			}

			// If target mod-manager supports profiles implement this function.
			virtual KxStringVector GetAvailableProfiles() const
			{
				return KxStringVector();
			}
	};
}

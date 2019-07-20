#pragma once
#include "stdafx.h"
#include "GameData/ISaveManager.h"
#include "Utility/KLabeledValue.h"

namespace Kortex::SaveManager
{
	class Config
	{
		private:
			KLabeledValue::Vector m_FileFilters;
			wxString m_SaveImplementation;
			wxString m_Location;

			wxString m_PrimarySaveExt;
			wxString m_SecondarySaveExt;

		public:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& node);

		public:
			wxString GetSaveImplementation() const;
			wxString GetLocation() const;
		
			bool HasFileFilter() const
			{
				return !m_FileFilters.empty();
			}
			const KLabeledValue::Vector& GetFileFilters() const
			{
				return m_FileFilters;
			}

			bool HasMultiFileSaveConfig() const
			{
				return !m_SecondarySaveExt.IsEmpty();
			}
			const wxString& GetPrimarySaveExtension() const
			{
				return m_PrimarySaveExt;
			}
			const wxString& GetSecondarySaveExtension() const
			{
				return m_SecondarySaveExt;
			}
	};
}

namespace Kortex::SaveManager
{
	namespace Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	}

	class DefaultSaveManager: public ISaveManager
	{
		private:
			BroadcastReciever m_BroadcastReciever;
			Config m_Config;

		private:
			KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;
			void OnSavesLocationChanged(BroadcastEvent& event);

		protected:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			void OnInit() override;
			void OnExit() override;

		public:
			const Config& GetConfig() const override
			{
				return m_Config;
			}
			KWorkspace* GetWorkspace() const override;
			std::unique_ptr<IGameSave> NewSave() const override;
	};
}

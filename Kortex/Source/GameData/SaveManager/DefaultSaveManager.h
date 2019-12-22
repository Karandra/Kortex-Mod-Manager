#pragma once
#include "stdafx.h"
#include "GameData/ISaveManager.h"
#include "Application/IWorkspace.h"
#include "Utility/LabeledValue.h"
#include "Utility/BitmapSize.h"

namespace Kortex::SaveManager
{
	class Config
	{
		private:
			Utility::LabeledValue::Vector m_FileFilters;
			wxString m_SaveImplementation;
			wxString m_Location;
			Utility::BitmapSize m_BitmapSize;

			wxString m_PrimarySaveExt;
			wxString m_SecondarySaveExt;

		public:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& node);

		public:
			wxString GetSaveImplementation() const;
			wxString GetLocation() const;
			Utility::BitmapSize GetBitmapSize() const
			{
				return m_BitmapSize;
			}

			bool HasFileFilter() const
			{
				return !m_FileFilters.empty();
			}
			const Utility::LabeledValue::Vector& GetFileFilters() const
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
			KxStringVector m_ActiveFilters;

		private:
			void CreateWorkspaces() override;
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
			IWorkspace::RefVector EnumWorkspaces() const override;
			std::unique_ptr<IGameSave> NewSave() const override;

			void UpdateActiveFilters(const KxStringVector& filters) override;
	};
}

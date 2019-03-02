#pragma once
#include "stdafx.h"
#include "GameConfig/IConfigManager.h"
#include "GameConfig/ConfigManger/DisplayModel.h"
#include "ConfigManager.h"
#include <KxFramework/KxStdDialog.h>

namespace Kortex::Application::Settings
{
	class Window: public KxStdDialog
	{
		private:
			ConfigManager m_Manager;
			GameConfig::DisplayModel m_DisplayModel;

		private:
			virtual int GetViewSizerProportion() const override
			{
				return 1;
			}
			virtual wxOrientation GetViewLabelSizerOrientation() const override
			{
				return wxVERTICAL;
			}
			virtual bool IsEnterAllowed(wxKeyEvent& event, wxWindowID* id) const override
			{
				return true;
			}
			virtual wxWindow* GetDialogMainCtrl() const override
			{
				return m_DisplayModel.GetView();
			}

			void OnCloseWindow(wxCloseEvent& event);
			void OnPrepareUninstall(wxCommandEvent& event);

		public:
			Window(wxWindow* parent = nullptr);
			~Window();
	};
}

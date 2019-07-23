#include "stdafx.h"
#include "DisplayModel.h"
#include "DisplayModelNode.h"
#include "OptionStore.h"
#include "Utility/KAux.h"
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include <KxFramework/KxDualInputDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxComparator.h>

namespace Kortex::NetworkManager::NXMHandler
{
	wxAny DisplayModelNode::GetValue(const KxDataView2::Column& column) const
	{
		using namespace KxDataView2;
		using ColumnID = DisplayModel::ColumnID;

		switch (column.GetID<ColumnID>())
		{
			case ColumnID::NexusID:
			{
				return m_NexusID;
			}
			case ColumnID::Game:
			{
				return m_Instance.GetGameName();
			}
			case ColumnID::Target:
			{
				if (auto value = m_Options.GetOption<OptionStore::Instance>(m_NexusID))
				{
					return value->ID;
				}
				else if (auto value = m_Options.GetOption<OptionStore::Command>(m_NexusID))
				{
					if (!value->Arguments.IsEmpty())
					{
						return KxString::Format(wxS("\"%1\" %2"), value->Executable, value->Arguments);
					}
					return value->Executable;
				}
				return KAux::MakeNoneLabel();
			}
		};
		return {};
	}
	wxAny DisplayModelNode::GetEditorValue(const KxDataView2::Column& column) const
	{
		using namespace KxDataView2;
		using ColumnID = DisplayModel::ColumnID;
		using TargetOption = DisplayModel::TargetOption;

		if (column.GetID<ColumnID>() == ColumnID::Target)
		{
			if (auto value = m_Options.GetOption<OptionStore::Instance>(m_NexusID))
			{
				return value->ID;
			}
			else if (auto value = m_Options.GetOption<OptionStore::Command>(m_NexusID))
			{
				return (int)TargetOption::ExternalProgram;
			}
			return (int)TargetOption::None;
		}
		return {};
	}
	bool DisplayModelNode::SetValue(KxDataView2::Column& column, const wxAny& value)
	{
		using namespace KxDataView2;
		using ColumnID = DisplayModel::ColumnID;
		using TargetOption = DisplayModel::TargetOption;

		if (column.GetID<ColumnID>() == ColumnID::Target)
		{
			if (int index = -1; value.CheckType<int>() && value.GetAs(&index))
			{
				if (index == (int)TargetOption::None)
				{
					m_Options.RemoveOption(m_NexusID);
					return true;
				}
				else if (index == (int)TargetOption::ExternalProgram)
				{
					BroadcastProcessor::Get().CallAfter([this]()
					{
						OpenExecutableDialog();
					});
				}
				else if (index >= (int)TargetOption::MAX)
				{
					const auto& instances = IGameInstance::GetShallowInstances();
					index -= (int)TargetOption::MAX;

					if (index < instances.size())
					{
						const IGameInstance& targetInstance = *instances[index];
						m_Options.SetOption(m_NexusID, OptionStore::Instance{targetInstance.GetInstanceID()});
						return true;
					}
				}
			}
		}
		return false;
	}

	bool DisplayModelNode::Compare(const KxDataView2::Node& other, const KxDataView2::Column& column) const
	{
		return KxComparator::IsLess(GetValue(column).As<wxString>(), other.GetValue(column).As<wxString>());
	}
	bool DisplayModelNode::IsEnabled(const KxDataView2::Column& column) const
	{
		using ColumnID = DisplayModel::ColumnID;
		return column.GetID<ColumnID>() == ColumnID::Target;
	}

	void DisplayModelNode::OpenExecutableDialog()
	{
		KxDualInputDialog dialog(GetView(),
								  KxID_NONE,
								  KTr("NetworkManager.NXMHandler.ConfigureExecutable"),
								  wxDefaultPosition,
								  wxDefaultSize,
								  KxBTN_OK|KxBTN_CANCEL
		);
		dialog.SetInitialSize(wxSize(640, wxDefaultCoord));

		dialog.SetHint1(KTr("Generic.FileLocation"));
		dialog.SetHint2(KTr("Generic.Arguments"));
		if (auto value = m_Options.GetOption<OptionStore::Command>(m_NexusID))
		{
			dialog.SetValue1(value->Executable);
			dialog.SetValue2(value->Arguments);
		}
		else
		{
			dialog.SetValue2(wxS("\"%1\""));
		}

		dialog.AddButton(KxID_SELECT_FILE, {}, true).GetControl()->Bind(wxEVT_BUTTON, [&dialog](wxCommandEvent& event)
		{
			KxFileBrowseDialog fileDialog(&dialog, KxID_NONE, KxFBD_OPEN);
			fileDialog.AddFilter("*.exe", KTr("FileFilter.Programs"));
			fileDialog.AddFilter("*", KTr("FileFilter.AllFiles"));
			fileDialog.SetFolder(dialog.GetValue1());

			if (fileDialog.ShowModal() == KxID_OK)
			{
				dialog.SetValue1(fileDialog.GetResult());
			}
		});

		dialog.Center();
		if (dialog.ShowModal() == KxID_OK)
		{
			OptionStore::Command command;
			command.Executable = dialog.GetValue1();
			command.Arguments = dialog.GetValue2();
			m_Options.SetOption(m_NexusID, std::move(command));

			Refresh();
		}
	}

	DisplayModelNode::DisplayModelNode(OptionStore& options, IGameInstance& instance)
		:m_Options(options), m_Instance(instance)
	{
		m_NexusID = m_Instance.GetVariables().GetVariable(wxS("NexusDomainName")).AsString();
	}
}

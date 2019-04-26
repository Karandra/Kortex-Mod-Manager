#include "stdafx.h"
#include "DisplayModelNode.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/ModTagManager.hpp>
#include <Kortex/Events.hpp>
#include <KxFramework/KxComparator.h>

namespace Kortex::ModManager
{
	void DisplayModelModNode::OnAttachNode()
	{
		AttachChildren(m_Children);
	}

	bool DisplayModelModNode::IsEnabled(const KxDataView2::Column& column) const
	{
		return Workspace::GetInstance()->IsChangingModsAllowed();
	}
	KxDataView2::Editor* DisplayModelModNode::GetEditor(const KxDataView2::Column& column) const
	{
		if (m_Mod->QueryInterface<FixedGameMod>() || m_Mod->QueryInterface<PriorityGroup>())
		{
			return nullptr;
		}
		else
		{
			switch (column.GetID<ColumnID>())
			{
				case ColumnID::Priority:
				{
					auto editor = column.GetEditor();
					auto& validator = static_cast<wxIntegerValidator<intptr_t>&>(editor->GetValidatorRef());
					validator.SetMin(0);
					validator.SetMax(IModManager::GetInstance()->GetMods().size() - 1);

					return editor;
				}
			};
			return column.GetEditor();
		}
	}
	wxAny DisplayModelModNode::GetEditorValue(const KxDataView2::Column& column) const
	{
		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Name:
			{
				return m_Mod->GetName();
			}
			case ColumnID::Version:
			{
				return m_Mod->GetVersion().ToString();
			}
		};
		return GetValue(column);
	}
	
	wxAny DisplayModelModNode::GetValue(const KxDataView2::Column& column) const
	{
		if (const PriorityGroup* priorityGroup = m_Mod->QueryInterface<PriorityGroup>())
		{
			return GetValue(column, *priorityGroup);
		}
		else if (const FixedGameMod* fixedMod = m_Mod->QueryInterface<FixedGameMod>())
		{
			return GetValue(column, *fixedMod);
		}
		else
		{
			switch (column.GetID<ColumnID>())
			{
				case ColumnID::Name:
				{
					KxDataView2::BitmapTextToggleValue valueData(m_Mod->IsActive(), KxDataView2::ToggleType::CheckBox);
					if (m_Mod->GetName() != m_Mod->GetID())
					{
						valueData.SetText(KxString::Format(wxS("%1 (%2)"), m_Mod->GetName(), m_Mod->GetID()));
					}
					else
					{
						valueData.SetText(m_Mod->GetName());
					}

					valueData.SetBitmap(KGetBitmap(m_Mod->GetIcon()));
					return valueData;
				}
				case ColumnID::Priority:
				{
					return m_Mod->GetPriority();
				}
				case ColumnID::Version:
				{
					const KxVersion& version = m_Mod->GetVersion();
					KxDataView2::BitmapTextValue valueData(version);
					if (version.IsOK())
					{
						switch (version.GetType())
						{
							case KxVERSION_DATETIME:
							{
								valueData.SetBitmap(KGetBitmap(KIMG_CALENDAR_DAY));
								break;
							}
							case KxVERSION_INTEGER:
							{
								valueData.SetBitmap(KGetBitmap(KIMG_NOTIFICATION_COUNTER_42));
								break;
							}
						};
					}
					return valueData;
				}
				case ColumnID::Author:
				{
					return m_Mod->GetAuthor();
				}
				case ColumnID::ModSource:
				{
					if (const IModNetwork* modNetwork = static_cast<const IModNetwork*>(column.GetClientData()))
					{
						const ModSourceItem* item = m_Mod->GetModSourceStore().GetItem(*modNetwork);
						if (NetworkModInfo modInfo; item && item->TryGetModInfo(modInfo))
						{
							return modInfo.ToString();
						}
					}
					break;
				}
				case ColumnID::Tags:
				{
					return DisplayModel::FormatTagList(*m_Mod);
				}
				case ColumnID::DateInstall:
				{
					return KAux::FormatDateTime(m_Mod->GetInstallTime());
				}
				case ColumnID::DateUninstall:
				{
					return KAux::FormatDateTime(m_Mod->GetUninstallTime());
				}
				case ColumnID::ModFolder:
				{
					if (m_Mod->IsLinkedMod())
					{
						return m_Mod->GetModFilesDir();
					}
					return m_Mod->GetRootDir();
				}
				case ColumnID::PackagePath:
				{
					return m_Mod->GetPackageFile();
				}
				case ColumnID::Signature:
				{
					return m_Mod->GetSignature();
				}
			};
		}
		return {};
	}
	wxAny DisplayModelModNode::GetValue(const KxDataView2::Column& column, const PriorityGroup& priorityGroup) const
	{
		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Name:
			{
				const IModTag* tag = priorityGroup.GetTag();
				if (tag && priorityGroup.IsBegin())
				{
					return KxDataView2::BitmapTextToggleValue(tag->GetName(), KGetBitmap(m_Mod->GetIcon()));
				}
				break;
			}
		};
		return {};
	}
	wxAny DisplayModelModNode::GetValue(const KxDataView2::Column& column, const FixedGameMod& fixedGameMod) const
	{
		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Name:
			{
				return KxDataView2::BitmapTextToggleValue(m_Mod->GetName(), KGetBitmap(m_Mod->GetIcon()));
			}
			case ColumnID::Priority:
			{
				return m_Mod->GetDisplayOrder();
			}
			case ColumnID::ModFolder:
			{
				return m_Mod->GetRootDir();
			}
		};
		return {};
	}
	bool DisplayModelModNode::SetValue(const wxAny& value, KxDataView2::Column& column)
	{
		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Name:
			{
				if (value.CheckType<wxString>())
				{
					m_Mod->SetName(value.As<wxString>());
					IEvent::MakeSend<ModEvent>(Events::ModChanged, *m_Mod);
				}
				else
				{
					m_Mod->SetActive(value.As<bool>());
					IEvent::MakeSend<ModEvent>(Events::ModToggled, *m_Mod);
				}

				m_Mod->Save();
				IModManager::GetInstance()->Save();
				return true;
			}
			case ColumnID::Priority:
			{
				int priority = -1;
				if (value.GetAs(&priority) && IModManager::GetInstance()->ChangeModPriority(*m_Mod, priority))
				{
					IModManager::GetInstance()->Save();
					IEvent::CallAfter([this]()
					{
						GetDisplayModel().LoadView();
						GetDisplayModel().SelectMod(m_Mod);
					});
					return true;
				}
				return false;
			}
			case ColumnID::Version:
			{
				wxString newVersion = value.As<wxString>();
				if (newVersion != m_Mod->GetVersion())
				{
					m_Mod->SetVersion(newVersion);
					m_Mod->Save();

					IEvent::MakeSend<ModEvent>(Events::ModChanged, *m_Mod);
					return true;
				}
				return false;
			}
			case ColumnID::Author:
			{
				wxString author = value.As<wxString>();
				if (author != m_Mod->GetAuthor())
				{
					m_Mod->SetAuthor(author);
					m_Mod->Save();

					IEvent::MakeSend<ModEvent>(Events::ModChanged, *m_Mod);
					return true;
				}
				break;
			}
		};
		return false;
	}

	bool DisplayModelModNode::Compare(const KxDataView2::Node& other, const KxDataView2::Column& column) const
	{
		const IGameMod& otherMod = static_cast<const DisplayModelModNode&>(other).GetMod();
		return Compare(*m_Mod, otherMod, column);
	}
	bool DisplayModelModNode::Compare(const IGameMod& left, const IGameMod& right, const KxDataView2::Column& column) const
	{
		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Name:
			{
				return KxComparator::IsLess(left.GetName(), right.GetName());
			}
			case ColumnID::Priority:
			{
				return left.GetDisplayOrder() < right.GetDisplayOrder();
			}
			case ColumnID::Version:
			{
				return left.GetVersion() < right.GetVersion();
			}
			case ColumnID::Author:
			{
				return KxComparator::IsLess(left.GetAuthor(), right.GetAuthor());
			}
			case ColumnID::Tags:
			{
				return KxComparator::IsLess(DisplayModel::FormatTagList(left), DisplayModel::FormatTagList(right));
			}
			case ColumnID::ModSource:
			{
				const IModNetwork* modNetwork = static_cast<const IModNetwork*>(column.GetClientData());
				if (modNetwork)
				{
					const ModSourceItem* item1 = left.GetModSourceStore().GetItem(*modNetwork);
					const ModSourceItem* item2 = right.GetModSourceStore().GetItem(*modNetwork);

					return item1 && item2 && (item1->GetModInfo().GetModID().GetValue() < item2->GetModInfo().GetModID().GetValue());
				}
				return false;
			}
			case ColumnID::DateInstall:
			{
				return left.GetInstallTime() < right.GetInstallTime();
			}
			case ColumnID::DateUninstall:
			{
				return left.GetUninstallTime() < right.GetUninstallTime();
			}
			case ColumnID::ModFolder:
			{
				return KxComparator::IsLess(left.GetModFilesDir(), right.GetModFilesDir());
			}
			case ColumnID::PackagePath:
			{
				return KxComparator::IsLess(left.GetPackageFile(), right.GetPackageFile());
			}
			case ColumnID::Signature:
			{
				return KxComparator::IsLess(left.GetSignature(), right.GetSignature());
			}
		};
		return false;
	}

	bool DisplayModelModNode::GetAttributes(KxDataView2::CellAttributes& attributes, const KxDataView2::CellState& cellState, const KxDataView2::Column& column) const
	{
		const FixedGameMod* fixed = m_Mod->QueryInterface<FixedGameMod>();
		const PriorityGroup* priorityGroup = m_Mod->QueryInterface<PriorityGroup>();
		const ColumnID columnID = column.GetID<ColumnID>();

		if (!fixed && !priorityGroup)
		{
			attributes.SetEnabled(m_Mod->IsInstalled() && IsEnabled(column));
		}
		if (columnID == ColumnID::Name && fixed && !priorityGroup)
		{
			attributes.SetItalic();
		}
		if (columnID == ColumnID::Color && !priorityGroup)
		{
			attributes.SetBackgroundColor(m_Mod->GetColor());
		}
		if (!fixed && !priorityGroup && (columnID == ColumnID::Name || columnID == ColumnID::ModSource))
		{
			attributes.SetUnderlined(cellState.IsHotTracked() && column.IsHotTracked());
		}
		if (priorityGroup)
		{
			DisplayModel& displayModel = GetDisplayModel();

			KxColor color = m_Mod->GetColor();
			if (color.IsOk())
			{
				attributes.SetForegroundColor(color.GetContrastColor(GetView()));
			}
			else
			{
				attributes.SetForegroundColor(displayModel.m_PriortyGroupColor);
			}

			attributes.SetBackgroundColor(color);
			attributes.SetBold(displayModel.m_BoldPriorityGroupLabels);
			attributes.SetAlignment(displayModel.m_PriorityGroupLabelAlignment);
		}
		return !attributes.IsDefault();
	}
	bool DisplayModelModNode::IsCategoryNode() const
	{
		//return m_Mod->QueryInterface<PriorityGroup>() != nullptr;
		return false;
	}
	int DisplayModelModNode::GetRowHeight() const
	{
		const DisplayModel& displayModel = GetDisplayModel();
		if (m_Mod->QueryInterface<PriorityGroup>())
		{
			return displayModel.m_PriorityGroupRowHeight;
		}
		return -1;
	}

	DisplayModel& DisplayModelModNode::GetDisplayModel() const
	{
		return *static_cast<DisplayModel*>(GetModel());
	}
}

namespace Kortex::ModManager
{
	void DisplayModelTagNode::OnAttachNode()
	{
		AttachChildren(m_Children);
	}

	KxDataView2::Editor* DisplayModelTagNode::GetEditor(const KxDataView2::Column& column) const
	{
		return nullptr;
	}
	wxAny DisplayModelTagNode::GetValue(const KxDataView2::Column& column) const
	{
		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Name:
			{
				return KxString::Format(wxS("%1 (%2)"), m_Tag->GetName(), m_Children.size());
			}
		};
		return {};
	}
	
	bool DisplayModelTagNode::Compare(const KxDataView2::Node& other, const KxDataView2::Column& column) const
	{
		const IModTag& otherTag = static_cast<const DisplayModelTagNode&>(other).GetTag();
		return Compare(*m_Tag, otherTag, column);
	}
	bool DisplayModelTagNode::Compare(const IModTag& left, const IModTag& right, const KxDataView2::Column& column) const
	{
		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Name:
			case ColumnID::Priority:
			{
				return KxComparator::IsLess(left.GetName(), right.GetName());
			}
		};
		return false;
	}

	bool DisplayModelTagNode::GetAttributes(KxDataView2::CellAttributes& attributes, const KxDataView2::CellState& cellState, const KxDataView2::Column& column) const
	{
		if (IsExpanded() || column.GetID<ColumnID>() == ColumnID::Name)
		{
			attributes.SetHeaderBackgound();
			return true;
		}
		return false;
	}
	bool DisplayModelTagNode::IsCategoryNode() const
	{
		return false;
	}
}

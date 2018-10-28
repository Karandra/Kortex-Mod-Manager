#include "stdafx.h"
#include "KProgramEntry.h"
#include "KProgramManager.h"
#include "GameInstance/KInstanceManagement.h"
#include "ModManager/KModManager.h"
#include "ModManager/KDispatcher.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxXML.h>

namespace
{
	bool IsRelative(const wxString& path)
	{
		return !(path.length() >= 2 && path[1] == wxS(':'));
	}
	wxString ResolvePath(const wxString& path, bool toVirtualDir, bool* isRelative = NULL)
	{
		if (!path.IsEmpty())
		{
			wxString pathExp = KVarExp(path);
			if (IsRelative(pathExp))
			{
				KxUtility::SetIfNotNull(isRelative, true);
				if (toVirtualDir)
				{
					if (KGameInstance* instnace = KGameInstance::GetActive())
					{
						return instnace->GetVirtualGameDir() + wxS('\\') + pathExp;
					}
				}
				else
				{
					if (KDispatcher* dispatcher = KDispatcher::GetInstance())
					{
						const KFileTreeNode* node = dispatcher->ResolveLocation(pathExp);
						if (node)
						{
							return node->GetFullPath();
						}
					}
				}
				return wxEmptyString;
			}

			KxUtility::SetIfNotNull(isRelative, false);
			return pathExp;
		}
		else
		{
			KxUtility::SetIfNotNull(isRelative, true);
			return wxEmptyString;
		}
	}
}

KProgramEntry::KProgramEntry()
{
}
KProgramEntry::KProgramEntry(const KxXMLNode& node)
{
	m_ShowInMainMenu = node.GetAttributeBool("ShowInMainMenu");

	m_Name = node.GetFirstChildElement("Name").GetValue();
	m_Executable = node.GetFirstChildElement("Executable").GetValue();
	m_IconPath = node.GetFirstChildElement("Icon").GetValue();
	m_Arguments = node.GetFirstChildElement("Arguments").GetValue();
	m_WorkingDirectory = node.GetFirstChildElement("WorkingDirectory").GetValue();
}

bool KProgramEntry::IsRequiresVFS() const
{
	bool isRelative = false;
	ResolvePath(m_Executable, true, &isRelative);
	return isRelative;
}
bool KProgramEntry::CanRunNow() const
{
	if (KModManager* manager = KModManager::GetInstance())
	{
		if (!IsRequiresVFS() || manager->IsVFSMounted())
		{
			return KxFile(ResolvePath(m_Executable, true)).IsFileExist();
		}
	}
	return false;
}

wxString KProgramEntry::GetName() const
{
	if (m_Name.IsEmpty())
	{
		return KVarExp(m_Executable).AfterLast(wxS('\\'));
	}
	return KVarExp(m_Name);
}
wxString KProgramEntry::GetIconPath() const
{
	wxString path = ResolvePath(m_IconPath, false);
	if (path.IsEmpty())
	{
		path = GetExecutableReal();
	}
	return path;
}
wxString KProgramEntry::GetExecutable() const
{
	return ResolvePath(m_Executable, true);
}
wxString KProgramEntry::GetExecutableReal() const
{
	return ResolvePath(m_Executable, false);
}
wxString KProgramEntry::GetArguments() const
{
	return KVarExp(m_Arguments);
}
wxString KProgramEntry::GetWorkingDirectory() const
{
	return ResolvePath(m_WorkingDirectory, true);
}

void KProgramEntry::OnRun()
{
}
void KProgramEntry::Save(KxXMLNode& rootNode) const
{
	KxXMLNode node = rootNode.NewElement("Entry");
	node.SetAttribute("ShowInMainMenu", m_ShowInMainMenu);

	node.NewElement("Name").SetValue(m_Name);
	node.NewElement("Executable").SetValue(m_Executable);

	if (!m_IconPath.IsEmpty())
	{
		node.NewElement("Icon").SetValue(m_IconPath);
	}
	if (!m_Arguments.IsEmpty())
	{
		node.NewElement("Arguments").SetValue(m_Arguments);
	}
	if (!m_WorkingDirectory.IsEmpty())
	{
		node.NewElement("WorkingDirectory").SetValue(m_WorkingDirectory);
	}
}

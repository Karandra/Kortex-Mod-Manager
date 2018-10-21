#pragma once
#include "stdafx.h"
#include "KWithBitmap.h"
class KxXMLNode;
class KProgramManager;

class KProgramEntry
{
	friend class KProgramManager;

	public:
		using Vector = std::vector<KProgramEntry>;
		using RefVector = std::vector<KProgramEntry*>;

	private:
		KWithBitmap m_SmallBitmap;
		KWithBitmap m_LargeBitmap;

		wxString m_Name;
		wxString m_IconPath;
		wxString m_Executable;
		wxString m_Arguments;
		wxString m_WorkingDirectory;

		wxDateTime m_LastRunTime;
		bool m_ShowInMainMenu = false;

	public:
		KProgramEntry();
		KProgramEntry(const KxXMLNode& node);

	public:
		const KWithBitmap& GetSmallBitmap() const
		{
			return m_SmallBitmap;
		}
		KWithBitmap& GetSmallBitmap()
		{
			return m_SmallBitmap;
		}
		const KWithBitmap& GetLargeBitmap() const
		{
			return m_LargeBitmap;
		}
		KWithBitmap& GetLargeBitmap()
		{
			return m_LargeBitmap;
		}

		bool IsOK() const
		{
			return HasName() && HasExecutable();
		}
		wxDateTime GetLastRunTime() const
		{
			return m_LastRunTime;
		}
		
		bool IsRequiresVFS() const;
		bool CanRunNow() const;
		
		bool ShouldShowInMainMenu() const
		{
			return m_ShowInMainMenu;
		}
		void SetShowInMainMenu(bool value)
		{
			m_ShowInMainMenu = value;
		}

		bool HasName() const
		{
			return !m_Name.IsEmpty();
		}
		const wxString& RawGetName() const
		{
			return m_Name;
		}
		wxString GetName() const;
		void SetName(const wxString& value)
		{
			m_Name = value;
		}
		
		bool HasIconPath() const
		{
			return  !m_IconPath.IsEmpty();
		}
		const wxString& RawGetIconPath() const
		{
			return m_IconPath;
		}
		wxString GetIconPath() const;
		void SetIconPath(const wxString& value)
		{
			m_IconPath = value;
		}
		
		bool HasExecutable() const
		{
			return !m_Executable.IsEmpty();
		}
		const wxString& RawGetExecutable() const
		{
			return m_Executable;
		}
		wxString GetExecutable() const;
		wxString GetExecutableReal() const;
		void SetExecutable(const wxString& value)
		{
			m_Executable = value;
		}
		
		bool HasArguments() const
		{
			return !m_Arguments.IsEmpty();
		}
		const wxString& RawGetArguments() const
		{
			return m_Arguments;
		}
		wxString GetArguments() const;
		void SetArguments(const wxString& value)
		{
			m_Arguments = value;
		}

		bool HasWorkingDirectory() const
		{
			return !m_WorkingDirectory.IsEmpty();
		}
		const wxString& RawGetWorkingDirectory() const
		{
			return m_WorkingDirectory;
		}
		wxString GetWorkingDirectory() const;
		void SetWorkingDirectory(const wxString& value)
		{
			m_WorkingDirectory = value;
		}

		void OnRun();
		void Save(KxXMLNode& rootNode) const;
};

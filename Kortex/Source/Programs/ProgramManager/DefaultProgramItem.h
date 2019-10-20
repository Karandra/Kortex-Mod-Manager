#pragma once
#include "stdafx.h"
#include "Programs/IProgramItem.h"

namespace Kortex::ProgramManager
{
	class DefaultProgramItem: public IProgramItem
	{
		private:
			KWithBitmap m_SmallBitmap;
			KWithBitmap m_LargeBitmap;

			wxString m_Name;
			wxString m_IconPath;
			wxString m_Executable;
			wxString m_Arguments;
			wxString m_WorkingDirectory;

			bool m_ShowInMainMenu = false;

		public:
			bool IsOK() const override
			{
				return !m_Name.IsEmpty() && !m_Executable.IsEmpty();
			}
			void Load(const KxXMLNode& node) override;
			void Save(KxXMLNode& node) const override;
		
			bool RequiresVFS() const override;
			bool CanRunNow() const override;
			void OnRun() override;

			bool ShouldShowInMainMenu() const override
			{
				return m_ShowInMainMenu;
			}
			void ShowInMainMenu(bool value) override
			{
				m_ShowInMainMenu = value;
			}

			wxString RawGetName() const override
			{
				return m_Name;
			}
			wxString GetName() const override;
			void SetName(const wxString& value) override
			{
				m_Name = value;
			}
		
			wxString RawGetIconPath() const override
			{
				return m_IconPath;
			}
			wxString GetIconPath() const override;
			void SetIconPath(const wxString& value) override
			{
				m_IconPath = value;
			}
		
			wxString RawGetExecutable() const override
			{
				return m_Executable;
			}
			wxString GetExecutable() const override;
			void SetExecutable(const wxString& value) override
			{
				m_Executable = value;
			}
		
			wxString RawGetArguments() const override
			{
				return m_Arguments;
			}
			wxString GetArguments() const override;
			void SetArguments(const wxString& value) override
			{
				m_Arguments = value;
			}

			wxString RawGetWorkingDirectory() const override
			{
				return m_WorkingDirectory;
			}
			wxString GetWorkingDirectory() const override;
			void SetWorkingDirectory(const wxString& value) override
			{
				m_WorkingDirectory = value;
			}

			const KWithBitmap& GetSmallBitmap() const override
			{
				return m_SmallBitmap;
			}
			KWithBitmap& GetSmallBitmap() override
			{
				return m_SmallBitmap;
			}
			const KWithBitmap& GetLargeBitmap() const override
			{
				return m_LargeBitmap;
			}
			KWithBitmap& GetLargeBitmap() override
			{
				return m_LargeBitmap;
			}
	};
}

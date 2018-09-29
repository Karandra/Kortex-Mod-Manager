#include "stdafx.h"
#include "KVariablesTable.h"
#include "KApp.h"
#include <KxFramework/KxShell.h>

wxString KIVariablesTable::Expand(const wxString& variables) const
{
	if (!variables.IsEmpty() && (!IsEmpty() || KApp::Get().IsTranslationLoaded()))
	{
		wxString out = variables;
		size_t entryStartPos = wxString::npos;
		size_t varNameStartPos = wxString::npos;
		bool isTranslationVar = false;
		bool isShellVar = false;

		for (size_t i = 0; i < out.Length(); i++)
		{
			if (out[i] == '$')
			{
				entryStartPos = i;

				// Look for 'SH'
				if (i + 2 < out.Length())
				{
					auto c1 = out[i + 1];
					auto c2 = out[i + 2];

					if (c1 == 'S' && c2 == 'H')
					{
						isShellVar = true;
						i += 2;
					}
				}

				// Check for 'T'
				if (i + 1 < out.Length())
				{
					auto cNext = out[i + 1];
					if (cNext == 'T')
					{
						isTranslationVar = true;
						i++;
					}
				}
			}

			// We are at the beginning of the variable name
			if (entryStartPos != wxString::npos && out[i] == '(')
			{
				varNameStartPos = i+1;
			}

			// All data collected, do replace
			if (entryStartPos != wxString::npos && varNameStartPos != wxString::npos && out[i] == ')')
			{
				size_t varEntryLength = i - entryStartPos + 1;
				size_t varNameLength = i - varNameStartPos;
				wxString varName = out.Mid(varNameStartPos, varNameLength);
				if (!varName.IsEmpty())
				{
					wxString value;
					if (isTranslationVar)
					{
						value = T(varName);
					}
					else if (isShellVar)
					{
						KxShellFolderID id = KxShell::GetShellFolderID(varName);
						if (id != KxSHF_MAX_ID)
						{
							value = KxShell::GetFolder(id);
						}
						else
						{
							KLogMessage("Can't expand shell variable \"%s\"", varName);
						}
					}
					else
					{
						value = GetVariable(varName);
					}

					if (!value.IsEmpty())
					{
						out.replace(entryStartPos, varEntryLength, value);

						// Set 'i' value to end of replacement
						i = entryStartPos + value.Length() - 1;
					}
				}

				// Reset parser state
				entryStartPos = wxString::npos;
				varNameStartPos = wxString::npos;
				isTranslationVar = false;
				isShellVar = false;
			}
		}
		return out;
	}
	return variables;
}

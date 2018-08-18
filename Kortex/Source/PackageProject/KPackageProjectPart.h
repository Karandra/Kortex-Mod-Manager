#pragma once
#include "stdafx.h"
#include "KPackageProjectDefs.h"
class KPackageProject;

class KPackageProjectPart
{
	private:
		KPackageProject& m_Project;

	public:
		KPackageProjectPart(KPackageProject& project);
		virtual ~KPackageProjectPart();

	public:
		KPackageProject& GetProject()
		{
			return m_Project;
		}
		const KPackageProject& GetProject() const
		{
			return m_Project;
		}
};

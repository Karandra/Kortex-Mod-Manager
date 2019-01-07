#pragma once
#include "stdafx.h"
#include "IDisplayModel.h"
#include "BethesdaDisplayModel.h"

namespace Kortex::PluginManager
{
	class BethesdaPluginManager2;

	class Bethesda2DisplayModel: public BethesdaDisplayModel
	{
		private:
			BethesdaPluginManager2& m_Manager;

		public:
			virtual void GetValue(wxAny& value, const IGamePlugin& plugin, const KxDataViewColumn* column) const override;

		public:
			Bethesda2DisplayModel(BethesdaPluginManager2& manager)
				:m_Manager(manager)
			{
			}
	};
}

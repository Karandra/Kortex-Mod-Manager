#pragma once
#include "stdafx.h"
#include "Common.h"
#include "DataType.h"

namespace Kortex::GameConfig
{
	class Item;
	class ItemValue;
	class ItemGroup;
	class ITypeDetector;
}

namespace Kortex::GameConfig
{
	class ISource: public KxRTTI::Interface<ISource>
	{
		KxDecalreIID(ISource, {0x838e049b, 0xb217, 0x4a11, {0xa9, 0x94, 0x4e, 0x14, 0xf1, 0x48, 0xcf, 0xe5}});

		protected:
			template<class TFunctor> TypeID InvokeTypeDetectors(const ItemGroup& group, TFunctor&& func)
			{
				TypeID type;
				group.GetDefinition().ForEachTypeDetector([this, &func, &type](const ITypeDetector& detector)
				{
					if (!type.IsDefinitiveType())
					{
						auto&&[valueName, valueData] = std::invoke(func, detector);
						type = detector.GetType(valueName, valueData);
					}
				});
				return type;
			}
		
		public:
			virtual ~ISource() = default;

		public:
			virtual SourceFormatValue GetFormat() const = 0;
			virtual wxString GetPathDescription() const = 0;

			virtual bool IsOpened() const = 0;
			virtual bool Open() = 0;
			virtual bool Save() = 0;
			virtual void Close() = 0;

			virtual bool WriteValue(const Item& item, const ItemValue& value) = 0;
			virtual bool ReadValue(Item& item, ItemValue& value) const = 0;
			virtual void LoadUnknownItems(ItemGroup& group) = 0;
	};
}

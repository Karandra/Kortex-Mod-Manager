#pragma once
#include "stdafx.h"
#include "Common.h"

namespace Kortex::GameConfig
{
	class ISource
	{
		public:
			virtual ~ISource() = default;

		public:
			virtual SourceTypeValue GetType() const = 0;
			virtual SourceFormatValue GetFormat() const = 0;

			virtual bool IsOpened() const = 0;
			virtual bool Open() = 0;
			virtual bool Save() = 0;
			virtual void Close() = 0;

			virtual bool WriteValue() = 0;
			virtual bool ReadValue() const = 0;
	};
}

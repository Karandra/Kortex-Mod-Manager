#pragma once
#include "stdafx.h"

namespace
{
	namespace Util
	{
		using namespace Kortex;

		enum class FindBy
		{
			GameID,
			InstanceID,
			ProfileID
		};
		template<class ObjectT, FindBy findBy, class VectorT> ObjectT* FindObjectInVector(VectorT& storageVector, const wxString& value, typename VectorT::const_iterator* itOut = nullptr)
		{
			auto it = std::find_if(storageVector.begin(), storageVector.end(), [&value](const auto& object)
			{
				if constexpr (findBy == FindBy::GameID)
				{
					return KxComparator::IsEqual(object->GetGameID(), value, true);
				}
				else if constexpr (findBy == FindBy::InstanceID)
				{
					return KxComparator::IsEqual(object->GetInstanceID(), value, true);
				}
				else if constexpr (findBy == FindBy::ProfileID)
				{
					return KxComparator::IsEqual(object->GetID(), value, true);
				}
				else
				{
					static_assert(false, "unsupported find request");
				}
			});

			if (it != storageVector.end())
			{
				if (itOut)
				{
					*itOut = it;
				}
				return it->get();
			}
			return nullptr;
		}
		template<class ObjectT, class MapT> ObjectT* FindObjectInMap(MapT& map, const wxString& id)
		{
			auto it = map.find(id);
			if (it != map.end())
			{
				return &it->second;
			}
			return nullptr;
		}

		template<class VectorT> void SortByOrder(VectorT& items)
		{
			std::sort(items.begin(), items.end(), [](const auto& v1, const auto& v2)
			{
				return v1->GetSortOrder() < v2->GetSortOrder();
			});
		}
		template<class VectorT> void SortByInstanceID(VectorT& items)
		{
			std::sort(items.begin(), items.end(), [](const auto& v1, const auto& v2)
			{
				return KxComparator::IsLess(v1->GetInstanceID(), v2->GetInstanceID(), true);
			});
		}
	}
}

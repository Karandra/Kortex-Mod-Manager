#pragma once
#include "stdafx.h"

class KUPtrVectorUtil
{
	private:
		enum class MoveMode
		{
			Before,
			After
		};
		template<class T> using ElementsVector = std::vector<std::unique_ptr<T>>;
		template<class T> using ElementsRefVector = std::vector<T*>;

		template<class T> static bool MoveElements(ElementsVector<T>& allItems,
										   const ElementsRefVector<T>& toMove,
										   const T& anchor,
										   MoveMode moveMode
		)
		{
			// Check if anchor is not one of moved elements
			if (std::find(toMove.begin(), toMove.end(), &anchor) != toMove.end())
			{
				return false;
			}

			// Remove all moved elements from existing place
			ElementsVector<T> removedItems;
			allItems.erase(std::remove_if(allItems.begin(), allItems.end(), [&toMove, &removedItems](auto& entry)
			{
				if (std::find(toMove.begin(), toMove.end(), entry.get()) != toMove.end())
				{
					removedItems.push_back(std::move(entry));
					return true;
				}
				return false;
			}), allItems.end());

			// Find anchor position
			auto anchorIt = std::find_if(allItems.begin(), allItems.end(), [&anchor](const auto& entry)
			{
				return entry.get() == &anchor;
			});
			if (anchorIt != allItems.end())
			{
				switch (moveMode)
				{
					case MoveMode::Before:
					{
						for (auto& item: removedItems)
						{
							allItems.insert(anchorIt, std::move(item));
						}
						break;
					}
					default:
					{
						size_t offset = 1;
						for (auto& item: removedItems)
						{
							allItems.insert(anchorIt + offset, std::move(item));
							offset++;
						}
						break;
					}
				};
				return true;
			}
			return false;
		}

	public:
		template<class T> static bool MoveBefore(ElementsVector<T>& allItems, const ElementsRefVector<T>& toMove, const T& anchor)
		{
			return MoveElements<T>(allItems, toMove, anchor, MoveMode::Before);
		}
		template<class T> static bool MoveAfter(ElementsVector<T>& allItems, const ElementsRefVector<T>& toMove, const T& anchor)
		{
			return MoveElements<T>(allItems, toMove, anchor, MoveMode::After);
		}
};

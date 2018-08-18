#pragma once
#include "stdafx.h"
#include <KxFramework/KxDataViewModelExBase.h>
#include <KxFramework/KxDataViewListModelEx.h>
#include <KxFramework/KxDataViewTreeModelEx.h>

#define KDataViewModelBase						KxDataViewModelExBase<KxDataViewModel>
#define KDataViewModelDragDropData				KxDataViewModelExDragDropData
#define KDataViewModelDragDropEnabled			KxDataViewModelExDragDropEnabled

#define KDataViewListModel						KxDataViewListModelEx
#define KDataViewVectorListModel				KxDataViewVectorListModelEx

#define KDataViewTreeModel						KxDataViewTreeModelEx<KxDataViewModel>
#define KDataViewVectorTreeModel				KxDataViewVectorTreeModelEx

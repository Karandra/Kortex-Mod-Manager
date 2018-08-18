#pragma once
#include "stdafx.h"

template<class T> class KEntryRef: public wxClientData
{
	public:
		using EntryType = T;

	protected:
		EntryType* m_Entry = NULL;

	public:
		KEntryRef(T* entry = NULL)
			:m_Entry(entry)
		{
		}
		virtual ~KEntryRef()
		{
		}

	public:
		bool IsOK() const
		{
			return m_Entry != NULL;
		}

		EntryType* GetEntry()
		{
			return m_Entry;
		}
		const EntryType* GetEntry() const
		{
			return m_Entry;
		}
		operator EntryType*()
		{
			return GetEntry();
		}
		operator const EntryType*() const
		{
			return GetEntry();
		}

		EntryType* operator->()
		{
			return m_Entry;
		}
		EntryType* operator->() const
		{
			return m_Entry;
		}
};

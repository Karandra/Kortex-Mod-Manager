#pragma once
#include "stdafx.h"
#include "KVFSService.h"
#include "KxVFSBase.h"
#include "MultiMirror/KxVFSMultiMirror.h"
class KVFSService;

class KVFSMultiMirror: public KxVFSMultiMirror
{
	public:
		KVFSMultiMirror(KVFSService* service, const wxString& mountPoint, const wxString& source);
		virtual ~KVFSMultiMirror();

	public:
		virtual NTSTATUS OnMount(EvtMounted& eventInfo) override;
		virtual NTSTATUS OnUnMount(EvtUnMounted& eventInfo) override;
};

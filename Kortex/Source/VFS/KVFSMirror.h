#pragma once
#include "stdafx.h"
#include "KxVFSBase.h"
#include "Mirror/KxVFSMirror.h"
class KVFSService;

class KVFSMirror: public KxVFSMirror
{
	public:
		KVFSMirror(KVFSService* service, const wxString& mountPoint, const wxString& source);
		virtual ~KVFSMirror();

	public:
		virtual NTSTATUS OnMount(EvtMounted& eventInfo) override;
		virtual NTSTATUS OnUnMount(EvtUnMounted& eventInfo) override;
};

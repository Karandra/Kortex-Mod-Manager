#pragma once
#include "stdafx.h"
#include "KVFSService.h"
#include "KxVFSBase.h"
#include "Convergence/KxVFSConvergence.h"
class KVFSService;

class KVFSConvergence: public KxVFSConvergence
{
	public:
		KVFSConvergence(KVFSService* service, const wxString& mountPoint, const wxString& writeTarget);
		virtual ~KVFSConvergence();

	public:
		virtual NTSTATUS OnMount(EvtMounted& eventInfo) override;
		virtual NTSTATUS OnUnMount(EvtUnMounted& eventInfo) override;
};

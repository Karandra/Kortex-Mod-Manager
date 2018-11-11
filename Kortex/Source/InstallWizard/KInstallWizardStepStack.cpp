#include "stdafx.h"
#include "KInstallWizardStepStack.h"

KIWStepStack::KIWStepStack()
{
}
KIWStepStack::~KIWStepStack()
{
}

void KIWStepStack::PushStep(KPPCStep* step)
{
	push(KIWStepStackItem(step));
}
void KIWStepStack::PushStep(KPPCStep* step, const KPPCEntry::RefVector& checked)
{
	push(KIWStepStackItem(step, checked));
}

void KIWStepStack::Clear()
{
	while (!empty())
	{
		pop();
	}
}

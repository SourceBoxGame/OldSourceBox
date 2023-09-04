#include "qscript/qscript_convar.h"
#include "qscript_structs.h"
#include "qscript.h"

extern IQScript* qscript;

ConCommandQScript::ConCommandQScript(const char* pName, QScriptCallback callback) : ConCommand(pName, (FnCommandCallbackVoid_t)callback)
{
	m_callback = callback;
}

void ConCommandQScript::Dispatch(const CCommand& command)
{
	QScriptArgs qargs = qscript->CreateArgs("s", command.GetCommandString());;
	qscript->CallCallback(m_callback, qargs);
	qscript->FreeArgs(qargs);
}
#include "qscript/qscript_convar.h"
#include "qscript_structs.h"
#include "qscript.h"

extern IQScript* qscript;

ConCommandQScript::ConCommandQScript(const char* pName, QScriptFunction callback) : ConCommand(pName, (FnCommandCallbackVoid_t)callback)
{
	m_callback = callback;
}

void ConCommandQScript::Dispatch(const CCommand& command)
{
	qscript->CallFunction(m_callback, "s", command.GetCommandString());
}
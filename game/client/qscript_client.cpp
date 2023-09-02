#include "qscript.h"
#include "convar.h"

extern IQScript* qscript;

void QScriptClientMsg(QScriptArgs args)
{
	Msg("%s\n", qscript->GetArgString(args, 0));
}

void InitQScriptClient()
{
	QScriptModule mod = qscript->CreateModule("sourcebox_client");
	qscript->CreateModuleFunction(mod, "Msg", "s", QScriptClientMsg);
}

void LoadModsClient()
{
	qscript->LoadMods("autorun_client");
}

/*
CON_COMMAND(execute_python, "The beginning")
{
	qscript->ExecuteScript(args.ArgS());
}
*/
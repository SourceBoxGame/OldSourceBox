#include "qscript.h"
#include "qscript_convar.h"
#include "convar.h"

extern IQScript* qscript;

QScriptReturn QScriptClientMsg(QScriptArgs args)
{
	Msg("%s\n", qscript->GetArgString(args, 0));
	return qscript->RetNone();
}

QScriptReturn RegisterCmd(QScriptArgs args)
{
	new ConCommandQScript(qscript->GetArgPermaString(args, 0), qscript->GetArgCallback(args, 1));
	return qscript->RetNone();
}



void InitQScriptClient()
{
	QScriptModule mod = qscript->CreateModule("sourcebox_client");
	qscript->CreateModuleFunction(mod, "Msg", "s", QScriptClientMsg);
	qscript->CreateModuleFunction(mod, "RegisterCmd", "sp", RegisterCmd);
}

void LoadModsClient()
{
	qscript->LoadMods("autorun_client");
	qscript->LoadModsInDirectory("client","cmds");
}

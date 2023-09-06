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

void InitQScriptServer()
{
	QScriptModule mod = qscript->CreateModule("sourcebox_server");
	qscript->CreateModuleFunction(mod, "Msg", "s", QScriptClientMsg);
	qscript->CreateModuleFunction(mod, "RegisterCmd", "sp", RegisterCmd);
}

void LoadModsServer()
{
	qscript->LoadMods("autorun_server");
	qscript->LoadModsInDirectory("server", "cmds");
}

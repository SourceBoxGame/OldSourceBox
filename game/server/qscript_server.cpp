#include "qscript.h"
#include "qscript_convar.h"
#include "convar.h"
#include "edict.h"

extern CGlobalVars* gpGlobals;
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

QScriptReturn GetTick(QScriptArgs args)
{
	return qscript->RetInt(gpGlobals->tickcount);
}

QScriptReturn GetTime(QScriptArgs args)
{
	return qscript->RetFloat(gpGlobals->curtime);
}

QScriptReturn GetDelayInSeconds(QScriptArgs args)
{
	return qscript->RetFloat(gpGlobals->curtime + qscript->GetArgFloat(args,0));
}

QScriptReturn GetDelayInTicks(QScriptArgs args)
{
	return qscript->RetFloat(gpGlobals->tickcount + qscript->GetArgInt(args, 0));
}

void InitQScriptServer()
{
	QScriptModule mod = qscript->CreateModule("sourcebox_server");
	qscript->CreateModuleFunction(mod, "Msg", "s", QScriptClientMsg);
	qscript->CreateModuleFunction(mod, "RegisterCmd", "sp", RegisterCmd);
	qscript->CreateModuleFunction(mod, "GetTick", "", GetTick);
	qscript->CreateModuleFunction(mod, "GetTime", "", GetTime);
	qscript->CreateModuleFunction(mod, "GetDelayInSeconds", "i", GetDelayInSeconds);
	qscript->CreateModuleFunction(mod, "GetDelayInTicks", "f", GetDelayInTicks);
}

void LoadModsServer()
{
	qscript->LoadMods("autorun_server");
	qscript->LoadModsInDirectory("server", "cmds");
}

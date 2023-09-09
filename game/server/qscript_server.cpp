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

/*
QScriptReturn CoolFunc(QScriptArgs args)
{
	return qscript->RetFloat(qscript->GetObjectFloat(qscript->GetObjectElementByName(qscript->GetArgObject(args, 0), "coolfloat")) + qscript->GetArgFloat(args, 1));
}
*/
/*
QScriptReturn HudHintYEAAAH(QScriptArgs args)
{
	QScriptObject obj = qscript->GetArgObject(args, 0);
	qscript->GetObjectElementByName(obj, "entitypointer");
	return qscript->RetNone();
}
*/

void InitQScriptServer()
{
	QScriptModule mod = qscript->CreateModule("sourcebox_server");
	qscript->CreateModuleFunction(mod, "Msg", "s", QScriptClientMsg);
	qscript->CreateModuleFunction(mod, "RegisterCmd", "sp", RegisterCmd);
	qscript->CreateModuleFunction(mod, "GetTick", "", GetTick);
	qscript->CreateModuleFunction(mod, "GetTime", "", GetTime);
	qscript->CreateModuleFunction(mod, "GetDelayInSeconds", "i", GetDelayInSeconds);
	qscript->CreateModuleFunction(mod, "GetDelayInTicks", "f", GetDelayInTicks);
	/*
	QScriptObjectCreator creator = qscript->StartObject();
	qscript->AddObject(creator, qscript->CreateFloatObject("coolfloat", 10.0));
	qscript->AddObject(creator, qscript->CreateFunctionObject("coolfunc", "f", CoolFunc));
	QScriptObject obj = qscript->FinishObject(creator, "epicobject");
	qscript->CreateModuleObject(mod, obj);
	*/
}

void LoadModsServer()
{
	qscript->LoadMods("autorun_server");
	qscript->LoadModsInDirectory("server", "cmds");
}

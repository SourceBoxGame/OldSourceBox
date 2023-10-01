#include "qscript.h"
#include "qscript_convar.h"
#include "convar.h"
#include "edict.h"
extern CGlobalVars* gpGlobals;
extern IQScript* qscript;


QReturn QScriptClientMsg(QScriptArgs args)
{
	Msg("%s\n", qscript->GetArgValue(args, 0).value_string);
	return qscript->RetNone();
}

QReturn RegisterCmd(QScriptArgs args)
{
	new ConCommandQScript(qscript->GetArgValue(args, 0).value_string, qscript->GetArgValue(args, 1).value_function);
	return qscript->RetNone();
}

QReturn GetTick(QScriptArgs args)
{
	return qscript->RetInt(gpGlobals->tickcount);
}

QReturn GetTime(QScriptArgs args)
{
	return qscript->RetFloat(gpGlobals->curtime);
}

QReturn GetDelayInSeconds(QScriptArgs args)
{
	return qscript->RetFloat(gpGlobals->curtime + qscript->GetArgValue(args,0).value_float);
}

QReturn GetDelayInTicks(QScriptArgs args)
{
	return qscript->RetFloat(gpGlobals->tickcount + qscript->GetArgValue(args, 0).value_int);
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


static QModuleDefFunc sv_module[] = {
	{QScriptClientMsg,"Msg",QType_None,"s"},
	{RegisterCmd,"RegisterCmd",QType_None,"sp"},
	{GetTick,"GetTick",QType_Int,""},
	{GetTime,"GetTime",QType_Float,""},
	{GetDelayInTicks,"GetDelayInTicks",QType_Int,"i"},
	{GetDelayInSeconds,"GetDelayInSeconds",QType_Float,"f"},
	{0,0,QType_None,0}
};

void InitQScriptServer()
{
	/*
	QScriptModule mod = qscript->CreateModule("sourcebox_server");
	qscript->CreateModuleFunction(mod, "Msg", "s", QScriptClientMsg);
	qscript->CreateModuleFunction(mod, "RegisterCmd", "sp", RegisterCmd);
	qscript->CreateModuleFunction(mod, "GetTick", "", GetTick);
	qscript->CreateModuleFunction(mod, "GetTime", "", GetTime);
	qscript->CreateModuleFunction(mod, "GetDelayInSeconds", "i", GetDelayInSeconds);
	qscript->CreateModuleFunction(mod, "GetDelayInTicks", "f", GetDelayInTicks);
	*/
	/*
	QScriptObjectCreator creator = qscript->StartObject();
	qscript->AddObject(creator, qscript->CreateFloatObject("coolfloat", 10.0));
	qscript->AddObject(creator, qscript->CreateFunctionObject("coolfunc", "f", CoolFunc));
	QScriptObject obj = qscript->FinishObject(creator, "epicobject");
	qscript->CreateModuleObject(mod, obj);
	*/

	QScriptModule mod = qscript->CreateModule("sv", (QModuleDefFunc*)&sv_module);
}

void LoadModsServer()
{
	/*
	qscript->LoadMods("autorun_server");
	qscript->LoadModsInDirectory("server", "cmds");
	*/
	qscript->LoadMods("sv_autorun");
	qscript->LoadModsInDirectory("sv", "cmds");
}



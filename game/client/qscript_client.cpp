#include "qscript.h"
#include "qscript_convar.h"
#include "convar.h"

extern IQScript* qscript;

QReturn QScriptClientMsg(QScriptArgs args)
{
	Msg("%s\n",qscript->GetArgValue(args,0).value_string);
	QReturn ret;
	ret.type = QType_None;
	return ret;
	//Msg("%s\n", qscript->GetArgString(args, 0));
	//return qscript->RetNone();
}
/*
QScriptReturn RegisterCmd(QScriptArgs args)
{
	new ConCommandQScript(qscript->GetArgPermaString(args, 0), qscript->GetArgCallback(args, 1));
	return qscript->RetNone();
}
*/
static QModuleDefFunc sourcebox_client[] = {
	{QScriptClientMsg,"Msg",QType_None,"s"},
	{0,0,QType_None,0}
	//{RegisterCmd,"RegisterCmd",QType_None,"sp"},
};

void InitQScriptClient()
{
	QScriptModule mod = qscript->CreateModule("sourcebox_client",(QModuleDefFunc*)&sourcebox_client);
	QScriptClassCreator testclass = qscript->StartClass("testclass");
	qscript->AddVariable(testclass, "testvar", QType_String);
	QScriptClass testclass_made = qscript->FinishClass(testclass);
	qscript->CreateModuleClass(mod, testclass_made);
}

void LoadModsClient()
{
	qscript->LoadMods("autorun_client");
	qscript->LoadModsInDirectory("client","cmds");
}

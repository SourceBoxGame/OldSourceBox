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

QReturn RegisterCmd(QScriptArgs args)
{
	new ConCommandQScript(qscript->GetArgValue(args, 0).value_string, qscript->GetArgValue(args, 1).value_function);
	QReturn ret;
	ret.type = QType_None;
	return ret;
}

static QModuleDefFunc cl_module[] = {
	{QScriptClientMsg,"Msg",QType_None,"s"},
	{RegisterCmd,"RegisterCmd",QType_None,"sp"},
	{0,0,QType_None,0}
	//{RegisterCmd,"RegisterCmd",QType_None,"sp"},
};

void InitQScriptClient()
{
	QScriptModule mod = qscript->CreateModule("cl",(QModuleDefFunc*)&cl_module);
	
	/*QScriptClassCreator testclass = qscript->StartClass("testclass");
	QValue val;
	val.value_float = 64.7;
	qscript->AddVariable(testclass, "testvar", QType_Float, val);
	QScriptClass testclass_made = qscript->FinishClass(testclass);
	qscript->CreateModuleClass(mod, testclass_made);*/
}

void LoadModsClient()
{
	qscript->LoadMods("cl_autorun");
	qscript->LoadModsInDirectory("cl","cmds");
}

#ifndef QSCRIPTCONVAR_H
#define QSCRIPTCONVAR_H

#if _WIN32
#pragma once
#endif
#include "qscript.h"
#include "convar.h"

class ConCommandQScript : public ConCommand
{
	friend class ConCommand;
public:
	typedef ConCommand BaseClass;

	ConCommandQScript(const char* pName, QScriptFunction callback);

	// Invoke the function
	virtual void Dispatch(const CCommand& command);

private:
	QScriptFunction m_callback;
};

#endif
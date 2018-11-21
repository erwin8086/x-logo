#include"ParserState.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

ParserState::ParserState()
{
	this->delay = true;
	this->vars = new PS_VARS();
	this->parent = NULL;
	this->procs = new PS_PROCS();
	this->stack = new std::vector<double>();
}

ParserState::ParserState(ParserState *parent)
{
	this->vars = new PS_VARS();
	this->procs = new PS_PROCS();
	this->parent = parent;
	this->stack = this->parent->getStack();
	this->delay = this->parent->getDelay();
	this->parent->listProcs();
}

ParserState::~ParserState()
{
	int i;
	for(i=0; i < this->vars->size(); i++)
	{
		free(this->vars->at(i).name);
	}
	for(i=0; i < this->procs->size(); i++)
	{
		free(this->procs->at(i).name);
		free(this->procs->at(i).commands);
	}
	delete this->procs;
	delete this->vars;
	if(!parent) delete this->stack;
}

bool ParserState::getDelay()
{
	return this->delay;
}

void ParserState::setDelay(bool delay)
{
	this->delay = delay;
}

bool ParserState::isVar(const char *name)
{
	int i;
	for(i=0; i < this->vars->size(); i++)
	{
		if(strcmp(this->vars->at(i).name, name)==0)
			return true;
	}
	if(this->parent)
		return this->parent->isVar(name);
	return false;
}

void ParserState::listVars()
{
	int i;
	printf("--Vars--\n");
	for(i=0; i < this->vars->size(); i++)
		printf("%s==%f\n", this->vars->at(i).name, this->vars->at(i).val);
	printf("--------\n");
}

double ParserState::getVar(const char *name)
{
	int i;
	for(i=0; i < this->vars->size(); i++)
	{
		if(strcmp(this->vars->at(i).name, name)==0)
		{
			return this->vars->at(i).val;
		}
	}
	if(this->parent)
		return this->parent->getVar(name);
	return 0;
}

void ParserState::setVar(const char *name, double val)
{
	int i;
	struct var v;
	v.name = (char*) malloc(strlen(name) + 1);
	strcpy(v.name, name);
	v.val = val;
	for(i=0; i < this->vars->size(); i++)
	{
		if(strcmp(this->vars->at(i).name, name)==0)
		{
			free(this->vars->at(i).name);
			this->vars->erase(this->vars->cbegin() + i);
		}
	}
	this->vars->push_back(v);
}	

void ParserState::setProc(const char *name, const char *cmds)
{
	struct proc p;
	p.name = (char*) malloc(strlen(name) + 1);
	p.commands = (char*) malloc(strlen(cmds) + 1);
	strcpy(p.name, name);
	strcpy(p.commands, cmds);

	int i;
	for(i=0; i<this->procs->size(); i++)
	{
		if(strcmp(this->procs->at(i).name, name)==0)
		{
			free(this->procs->at(i).name);
			free(this->procs->at(i).commands);
			this->procs->erase(this->procs->cbegin() + i);
		}
	}
	this->procs->push_back(p);
}

void ParserState::listProcs()
{
	int i;
	for(i=0; i<this->procs->size(); i++)
	{
		printf("to \"%s [ %s ]\n", this->procs->at(i).name, this->procs->at(i).commands);
	}
}

bool ParserState::isProc(const char *name)
{
	int i;
	for(i=0; i < this->procs->size(); i++)
	{
		if(strcmp(this->procs->at(i).name, name)==0)
			return true;
	}
	if(this->parent)
	{
		return this->parent->isProc(name);
	}
	return false;
}

const char* ParserState::getProc(const char *name)
{
	int i;
	for(i=0; i < this->procs->size(); i++)
	{
		if(strcmp(this->procs->at(i).name, name)==0)
			return this->procs->at(i).commands;
	}
	if(this->parent)
		return this->parent->getProc(name);
	return NULL;
}

std::vector<double>* ParserState::getStack()
{
	return this->stack;
}

void ParserState::push(double d)
{
	this->stack->push_back(d);
}

double ParserState::pop()
{
	if(this->stack->size() < 1)
		return 0;
	double r = this->stack->back();
	this->stack->pop_back();
	return r;
}

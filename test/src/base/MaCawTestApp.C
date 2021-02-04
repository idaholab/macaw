//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "MaCawTestApp.h"
#include "MaCawApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

InputParameters
MaCawTestApp::validParams()
{
  InputParameters params = MaCawApp::validParams();
  return params;
}

MaCawTestApp::MaCawTestApp(InputParameters parameters) : MooseApp(parameters)
{
  MaCawTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

MaCawTestApp::~MaCawTestApp() {}

void
MaCawTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  MaCawApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"MaCawTestApp"});
    Registry::registerActionsTo(af, {"MaCawTestApp"});
  }
}

void
MaCawTestApp::registerApps()
{
  registerApp(MaCawApp);
  registerApp(MaCawTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
MaCawTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  MaCawTestApp::registerAll(f, af, s);
}
extern "C" void
MaCawTestApp__registerApps()
{
  MaCawTestApp::registerApps();
}

#include "MaCawApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
MaCawApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

MaCawApp::MaCawApp(InputParameters parameters) : MooseApp(parameters)
{
  MaCawApp::registerAll(_factory, _action_factory, _syntax);
}

MaCawApp::~MaCawApp() {}

void
MaCawApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"MaCawApp"});
  Registry::registerActionsTo(af, {"MaCawApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
MaCawApp::registerApps()
{
  registerApp(MaCawApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
MaCawApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  MaCawApp::registerAll(f, af, s);
}
extern "C" void
MaCawApp__registerApps()
{
  MaCawApp::registerApps();
}

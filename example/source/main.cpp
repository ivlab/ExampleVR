/*
 * Copyright Regents of the University of Minnesota, 2014.  This software is released under the following license: http://opensource.org/licenses/lgpl-3.0.html.
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Code author(s):
 * 		Dan Orban (dtorban)
 */

#include "AppKit_GLFW/MVREngineGLFW.H"
#include "MVRCore/DataFileUtils.H"
#include <iostream>
#include "example/include/ExampleVrApp.h"

int
main(int argc, char** argv)
{
	MinVR::AbstractMVREngine *engine = new MinVR::MVREngineGLFW();
	engine->init(argc, argv);
	MinVR::AbstractMVRAppRef app(new ExampleVrApp());
	engine->runApp(app);
	delete engine;

	return 0;
}

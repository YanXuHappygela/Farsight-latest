/*=========================================================================
Copyright 2009 Rensselaer Polytechnic Institute
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
=========================================================================*/
#include "TraceMain.h"
#include <QtGui/QApplication>
#include <QtCore/QObject>
int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setOrganizationName("FARSIGHT Toolkit");
	app.setOrganizationDomain("farsight-toolkit.org");
	app.setApplicationName("Superellipsoid Trace 3D");
	TraceSEMain * Trace3D = new TraceSEMain();
	Trace3D->show();
	int retval = app.exec();
	delete Trace3D;
	return retval;
}
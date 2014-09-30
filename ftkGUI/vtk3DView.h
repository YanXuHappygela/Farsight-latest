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
#ifndef VTK3DVIEW_H
#define VTK3DVIEW_H

#include "vtkChartXYZ.h"
#include "vtkPlotPoints3D.h"
#include <vtkTable.h>
#include <vtkVariant.h>
#include <vtkSmartPointer.h>

#include "ObjectSelection.h"

class vtk3DView : public vtkChartXYZ
{

public:
	vtk3DView();
	void setModels(vtkSmartPointer<vtkTable> tbl, ObjectSelection * sels = NULL);
	vtkSmartPointer<vtkTable> GetTable(){ return table; };

protected:
    bool MouseButtonPressEvent(const vtkContextMouseEvent & mouse);

protected:
	vtkSmartPointer<vtkTable> table;
	ObjectSelection * selection;
    vtkSmartPointer<vtkPlotPoints3D> plot;
	vtkVector2i lastPos;
};

#endif 

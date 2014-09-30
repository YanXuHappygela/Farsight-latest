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
#include <math.h>

#include "vtk3DView.h"
#include <vtkDoubleArray.h>
#include <vtkVariantArray.h>
#include "vtkContextMouseEvent.h"
#include "vtkVector.h"

vtk3DView::vtk3DView()
{
    plot = vtkSmartPointer<vtkPlotPoints3D>::New();
	SetGeometry(vtkRectf(150.0, 40.0, 500, 520));
	lastPos.Set(0,0);
}

void vtk3DView::setModels(vtkSmartPointer<vtkTable> tbl, ObjectSelection * sels)
{
    table =  tbl;
    selection = sels;
    plot->SetInputData(table);
	AddPlot(plot);
	SetDecorateAxes(false);
}

bool vtk3DView::MouseButtonPressEvent(const vtkContextMouseEvent & mouse)
{
	vtkVector2f pos = mouse.GetPos();
	std::cout<< pos.GetX()<<std::endl;
	
	if( mouse.GetButton() == vtkContextMouseEvent::LEFT_BUTTON) 
	{
		vtkContextMouseEvent mouseEvent;
		std::cout<< lastPos.GetX()<<std::endl;
		vtkVector2i posNew(int(pos.GetX()), int(pos.GetY()));
		mouseEvent.SetLastScreenPos(lastPos);
		mouseEvent.SetScreenPos(posNew);
		MouseMoveEvent(mouseEvent);
		lastPos = posNew;
	}
    //vtkContextMouseEvent mouseEvent;
    //mouseEvent.SetInteractor(view->GetInteractor());
    //vtkVector2i pos;
    //vtkVector2i lastPos;

    //// rotate
    //mouseEvent.SetButton(vtkContextMouseEvent::LEFT_BUTTON);
    //lastPos.Set(114, 55);
    //mouseEvent.SetLastScreenPos(lastPos);
    //pos.Set(174, 121);
    //mouseEvent.SetScreenPos(pos);

    //vtkVector2d sP(pos.Cast<double>().GetData());
    //vtkVector2d lSP(lastPos.Cast<double>().GetData());

    //vtkVector2d screenPos(mouseEvent.GetScreenPos().Cast<double>().GetData());
    //vtkVector2d lastScreenPos(mouseEvent.GetLastScreenPos().Cast<double>().GetData());
    //chart->MouseMoveEvent(mouseEvent);
    //std::cout<< "add mouse rotate event."<<std::endl;

    //// mouse wheel zoom
    //chart->MouseWheelEvent(mouseEvent, -1);
    return true;
}

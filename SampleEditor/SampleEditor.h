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

//**************************************************************************
// STrenDEditor
//
// A simple tool to show how to use the ftkGUI classes for table viewing
// and manipulation.
// 
//**************************************************************************
#ifndef SAMPLEEDITOR_H
#define SAMPLEEDITOR_H

//QT Includes:
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QTableView>
#include <QtGui/QDockWidget>
#include <QtCore/QFileInfo>

//VTK Includes:
#include <vtkTable.h>
#include <vtkSmartPointer.h>
#include <vtkVariantArray.h>
#include <vtkDoubleArray.h>

//Farsight Includes:
#include "ftkGUI/TableWindow.h"
#include "ftkGUI/PlotWindow.h"
#include "ftkGUI/HistoWindow.h"
#include "ftkGUI/ObjectSelection.h"
#include "ftkGUI/StatisticsToolbar.h"
#include "ftkGUI/SelectiveClustering.h"
#include "ftkUtils.h"
//#include "ftkGUI/GraphWindow.h"
#include "ftkGUI/GraphWindowForNewSelection.h"
#include "ClusClus/clusclus.h"
#include "ClusClus/Dendrogram.h"
#include "ClusClus/HeatmapWindow.h"
#include "ClusClus/Heatmap.h"
#include "ClusClus/LocalGeometryRef.h"
#include "PatternAnalysis/STrenD/STrenDkNNGModuleMatch.h"
#include <vector>
#include <string>

/**
* SampleEditor
* A simple tool to show how to use the ftkGUI classes for table viewing
* and manipulation.
*/ 
class SampleEditor : public QMainWindow
{
    Q_OBJECT;

public:
	SampleEditor(QWidget * parent = 0, Qt::WindowFlags flags = 0);

protected:
	void closeEvent(QCloseEvent *event);

private slots:
	void loadFile(void);
	void loadRotateFile(void);
	void removeRows(void);
	void addBlankRow(void);
	void changeRowData(void);
	void showStatistics(void);
	void updateStatistics(void);

	void STrenDAnalysisModel();
	void DumpClusterSelections();
	void AddLabel();

signals:
    void selectionChanged(void);
private:
	void createMenus();
	void createStatusBar();
	void ReadFiles(std::string hname, std::string dname);
    QDockWidget *statisticsDockWidget;
	QMenu *fileMenu;
	QAction *loadAction;
	QAction *loadRotateAction;
	QMenu *editMenu;
	QAction *removeRowsAction;
	QAction *addBlankRowAction;
	QAction *changeRowDataAction;
	QAction *showStatisticsAction;
	QAction *updateStatisticsAction;

	QAction *CreateCluster;
	QAction *DisplayClusterSelections;

	StatisticsToolbar * statisticsToolbar;
	TableWindow *table;
	PlotWindow *plot;
	HistoWindow *histo;

	QAction *STrenDAction;
	STrenDkNNGModuleMatch *spdkNNGWin;
	
	int flag;

	vtkSmartPointer<vtkTable> data;
	ObjectSelection *selection;
	ObjectSelection *selection2;
	QString lastPath;

	SelectiveClustering * ClusterSelections;
	ClusterManager * SampleClusterManager;
 };


#endif

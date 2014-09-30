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

#include "SampleEditor.h"
//#include "Dendrogram.h"


//*******************************************************************************
// SampleEditor
//********************************************************************************

SampleEditor::SampleEditor(QWidget * parent, Qt::WindowFlags flags)
: QMainWindow(parent,flags)
{
	table = new TableWindow();
	plot = new PlotWindow(this);
	histo = new HistoWindow(this);
	
	spdkNNGWin = NULL;

	data = NULL;
	data = vtkSmartPointer<vtkTable>::New();		//Start with a new table

	selection = new ObjectSelection();
	selection2 = new ObjectSelection();

	lastPath = ".";

	createMenus();
	createStatusBar();
	this->flag = 0;

	setCentralWidget(table);
	setWindowTitle(tr("SampleEditor-v1.0"));
	connect(selection, SIGNAL(changed()), this, SLOT(updateStatistics()));
    
	this->ClusterSelections = new SelectiveClustering();
	//this->SampleClusterManager = new ClusterManager();
	//this->SampleClusterManager->setClusteringModel(this->ClusterSelections );
	//this->SampleClusterManager->setObjectSelection(selection);
	//this->SampleClusterManager->setVisible(true);

	this->resize(500,500);
}

//******************************************************************************
//Reimplement closeEvent to also close all other windows in the application
//******************************************************************************
void SampleEditor::closeEvent(QCloseEvent *event)
{
	//Then Close all other windows
	foreach (QWidget *widget, qApp->topLevelWidgets()) 
	{
		if (this != widget)
		{
			if(widget->isVisible())
				widget->close();
		}
    }
	//Then close myself
	event->accept();
}

//******************************************************************************
// Here we just show a message in the status bar when loading
//******************************************************************************
void SampleEditor::createStatusBar()
{
    QLabel *statusLabel = new QLabel(tr(" Ready"));
    statusBar()->addWidget(statusLabel, 1);
}

/*----------------------------------------------------------------------------*/
/* function: createMenus()                                                    */
/*                                                                            */
/* This function is used to create Menus that are associated with various     */
/* functions in the application. In order to add a menu, we need to do the    */
/* following:                                                                 */
/* 1.) Define a QMenu type (e.g., QMenu *fileMenu) and add it to menuBar()    */
/* 2.) Define QAction elements (e.g., QAction *openAction) associated with    */
/*     each QMenu                                                             */
/* 3.) Add a separator (menuBar()->addSeparator() after each menu group       */
/*																			  */
/*In order to create an Action, we need to do the							  */
/* following:                                                                 */
/* 1.) Define a QAction (e.g., QAction *openAction)                           */
/* 2.) Label the QAction element (e.g., openAction = new QAction(QIcon(":src/ */
/*     images/open.png"), tr("&Open..."), this). The QIcon argumenet is       */
/*     optional.                                                              */
/* 3.) Add optional "setShortcut" and "setStatusTip".                         */
/* 4.) Finally, bind this item with a "connect" that essentially calls the    */
/*     module to implement the operation (e.g.,                               */
/*     connect(openAction, SIGNAL(triggered()), this, SLOT(loadImage())). In  */
/*     this example, "loadImage()" is the module that is being called. These  */
/*     modules should be defined as "private" operators in the main class.    */
/*     The actual routines performing the operations (e.g., an image          */
/*     thresholding operation) must be accessed from within the called module.*/
/*	   Finally, after all these action's, we bind them to a "QActionGroup".       */ 
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void SampleEditor::createMenus()
{
	//FIRST HANDLE FILE MENU
	fileMenu = menuBar()->addMenu(tr("&File"));
	loadAction = new QAction(tr("Load File..."), this);
	loadAction->setStatusTip(tr("Load to table from text file"));
	connect(loadAction, SIGNAL(triggered()), this, SLOT(loadFile()));
	fileMenu->addAction(loadAction);

	loadRotateAction = new QAction(tr("Load Rotated Table..."), this);
	loadRotateAction->setStatusTip(tr("Load to table from text file"));
	connect(loadRotateAction, SIGNAL(triggered()), this, SLOT(loadRotateFile()));
	fileMenu->addAction(loadRotateAction);

	editMenu = menuBar()->addMenu(tr("&Edit"));
	removeRowsAction = new QAction(tr("Remove Selected Rows"),this);
	removeRowsAction->setStatusTip(tr("Remove selected rows from the table"));
	connect(removeRowsAction, SIGNAL(triggered()), this, SLOT(removeRows()));
	editMenu->addAction(removeRowsAction);

	showStatisticsAction = new QAction(tr("Show Statistics Toolbar"), this);
	connect(showStatisticsAction,SIGNAL(triggered()), this, SLOT(showStatistics()));
	editMenu->addAction(showStatisticsAction);

	updateStatisticsAction = new QAction(tr("Update Statistics"), this);
	connect(updateStatisticsAction, SIGNAL(triggered()), this, SLOT(updateStatistics()));
	//connect((this->selection), SIGNAL(selectionChanged()), this, SLOT(updateStatistics()));
	editMenu->addAction(updateStatisticsAction);

	//addBlankRowAction = new QAction(tr("Add Blank Row"), this);
	//addBlankRowAction->setStatusTip(tr("Add blank row to bottom of table"));
	//connect(addBlankRowAction, SIGNAL(triggered()), this, SLOT(addBlankRow()));
	//editMenu->addAction(addBlankRowAction);

	//changeRowDataAction = new QAction(tr("Change Row Data..."), this);
	//changeRowDataAction->setStatusTip(tr("Change data for selected row"));
	//connect(changeRowDataAction, SIGNAL(triggered()), this, SLOT(changeRowData()));
	//editMenu->addAction(changeRowDataAction);

	STrenDAction = new QAction(tr("Subspace Trend Discovery"), this);
	STrenDAction->setStatusTip(tr("Subspace Trend Discovery"));
	connect(STrenDAction, SIGNAL(triggered()), this, SLOT(STrenDAnalysisModel()));
	STrenDAction->setShortcut(Qt::CTRL + Qt::Key_S);
	editMenu->addAction(STrenDAction);
}

//********************************************************************************
// LoadFile()
//
// Ask for the header file and the data file that define a table and load the data
// into vtkTable, then show results.
//********************************************************************************
void SampleEditor::loadRotateFile()
{
	QString headername = QFileDialog::getOpenFileName(this,"Choose a Header File", lastPath, tr("TXT Files (*.txt)") );
	if(headername == "") return;
	lastPath = QFileInfo(headername).absolutePath();

	/*QString dataname = QFileDialog::getOpenFileName(this,"Choose a Data File", lastPath, tr("TXT Files (*.txt)") );
	if(dataname == "") return;
	lastPath = QFileInfo(dataname).absolutePath();
	
	selection->clear();

	ReadFiles(headername.toStdString(), dataname.toStdString());*/
	this->data = ftk::LoadRotatedTable(headername.toStdString());
	std::cout<< "Read table:"<<data->GetNumberOfRows()<<"\t"<< data->GetNumberOfColumns()<<endl;
	table->setModels(data,selection,selection2);//////////////////////////////////////////////////////////////////////////
	table->show();

	//plot->setModels(data,selection);
	//plot->show();
	/*this->histo->setModels(data, selection);
	this->histo->show();*/
	//std::cout << "I reached here inside the sample editor"<<std::endl;
	//this->graph->setModels(data, selection, selection2);
	//this->dendro1->setModels(data,selection);
	//this->dendro2->setModels(data,selection2);
	//this->heatmap->setModels(data,selection,selection2);
	//this->ClusterSelections->SetObjectTable(data);
	//this->SampleClusterManager->setVisible(true);

}

void SampleEditor::loadFile()
{
	QString headername = QFileDialog::getOpenFileName(this,"Choose a Header File", lastPath, tr("TXT Files (*.txt)") );
	if(headername == "") return;
	lastPath = QFileInfo(headername).absolutePath();

	/*QString dataname = QFileDialog::getOpenFileName(this,"Choose a Data File", lastPath, tr("TXT Files (*.txt)") );
	if(dataname == "") return;
	lastPath = QFileInfo(dataname).absolutePath();
	
	selection->clear();

	ReadFiles(headername.toStdString(), dataname.toStdString());*/
	this->data = ftk::LoadTable(headername.toStdString());
	std::cout<< "Read table:"<<data->GetNumberOfRows()<<"\t"<< data->GetNumberOfColumns()<<endl;
	table->setModels(data,selection,selection2);//////////////////////////////////////////////////////////////////////////
	table->show();

	plot->setModels(data,selection);
	plot->show();
	/*this->histo->setModels(data, selection);
	this->histo->show();*/
	//std::cout << "I reached here inside the sample editor"<<std::endl;
	//this->graph->setModels(data, selection, selection2);
	//this->dendro1->setModels(data,selection);
	//this->dendro2->setModels(data,selection2);
	//this->heatmap->setModels(data,selection,selection2);
	//this->ClusterSelections->SetObjectTable(data);
	//this->SampleClusterManager->setVisible(true);

}


void SampleEditor::ReadFiles(std::string hname, std::string dname)
{

	const int MAXLINESIZE = 102400;	//Numbers could be in scientific notation in this file
	char line[MAXLINESIZE];

	//data = vtkSmartPointer<vtkTable>::New();		//Start with a new table
	data->Initialize();
	std::cout << "The value of hname is "<<hname<<std::endl;
	std::cout << "The value of dname is "<<dname <<std::endl;

	//LOAD THE HEADER INFO:
	ifstream headerFile; 
	headerFile.open( hname.c_str() );
	if ( !headerFile.is_open() )
		return ;

	vtkSmartPointer<vtkDoubleArray> column = vtkSmartPointer<vtkDoubleArray>::New();
	headerFile.getline(line, MAXLINESIZE);
	/*std::cout << line << std::endl;*/
	while ( !headerFile.eof() ) //Get all values
	{
		std::string h;
		char * pch = strtok (line," \t");
		while (pch != NULL)
		{
			h = pch;
			pch = strtok (NULL, " \t");
		}
		column = vtkSmartPointer<vtkDoubleArray>::New();
		column->SetName( h.c_str() );
		data->AddColumn(column);
		/*std::cout<<"The headers is:"<<std::endl;*/
		/*data->Dump(3);*/
		headerFile.getline(line, MAXLINESIZE);
	}
	headerFile.close();
	/*std::cout << "Finished loading headers" << std::endl;*/

	//LOAD ALL OF THE FEATURES INFO:
	ifstream featureFile; 
	featureFile.open( dname.c_str() );
	if ( !featureFile.is_open() )
		return;

	featureFile.getline(line, MAXLINESIZE);
	
	std::cout << line<<std::endl;
	while ( !featureFile.eof() ) //Get all values
	{
		vtkSmartPointer<vtkVariantArray> row = vtkSmartPointer<vtkVariantArray>::New();
		char * pch = strtok (line," \t");
		while (pch != NULL)
		{
			row->InsertNextValue( vtkVariant( atof(pch) ) );
			pch = strtok (NULL, " \t");
		}
		data->InsertNextRow(row);
		/*std::cout <<"The data is:"<<std::endl;*/
		/*data->Dump(3);*/

		featureFile.getline(line, MAXLINESIZE);
	}
	featureFile.close();
	/*std::cout << "Finished loading data" << std::endl;*/
	
}

//***********************************************************************************
// This functions demonstrates how to remove rows from the table and how 
// to make sure that the views are updated accordingly.
//
// It is important to realize that row numbers change after removing a row.
// This means you cannot iterate through the table to remove rows, but 
// must look for each row independently.
//
// Also note that it is important to remove the ids of removed rows from selections.
//***********************************************************************************
void SampleEditor::removeRows(void)
{
	if( this->data->GetNumberOfRows() <= 0)
	{
		return;
	}

	std::set<long int> sels = selection->getSelections();
	std::set<long int>::iterator it;
	for(it=sels.begin(); it!=sels.end(); ++it)
	{
		for(int i=0; i<data->GetNumberOfRows(); ++i)
		{
			if( data->GetValue(i,0).ToLong() == (*it) )
			{
				data->RemoveRow(i);
				break;
			}
		}
	}
	selection->clear();
	table->update();
	this->ClusterSelections->update();
	plot->update();
}

void SampleEditor::addBlankRow(void)
{
	if( this->data->GetNumberOfRows() <= 0)
	{
		return;
	}
	//Get the maximum ID:
	long max = 0;
	for(int i=0; i<data->GetNumberOfRows(); ++i)
	{
		long val = data->GetValue(i,0).ToLong();
		if( val > max )
			max = val;
	}

	//Insert blank row:
	data->InsertNextBlankRow();

	long newID = max+1;

	//Set the ID of the new ROW:
	data->SetValue( data->GetNumberOfRows() - 1, 0, vtkVariant(newID));

	//Select the new row only:
	selection->select(newID);
	
	//update views:
	table->update();
	plot->update();
}

void SampleEditor::changeRowData(void)
{
	if( this->data->GetNumberOfRows() <= 0)
	{
		return;
	}
}

void SampleEditor::showStatistics(void)
{
	if( this->data->GetNumberOfRows() <= 0)
	{
		return;
	}
	if (this->flag == 1)
	{
		this->statisticsToolbar->statisticsDockWidget->close(); //QT not closing properly
		delete this->statisticsToolbar->statisticsDockWidget;
		this->statisticsToolbar->statisticsDockWidget = NULL;
		//std::cout << "Statistics widget close" << std::endl;
		this->flag = 0;
		return;
	}
	this->statisticsDockWidget = new QDockWidget();
	this->statisticsToolbar = new StatisticsToolbar(statisticsDockWidget);
	
	statisticsDockWidget->setWidget(statisticsToolbar->statisticsDockWidget);
	statisticsDockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
	addDockWidget(Qt::BottomDockWidgetArea, statisticsToolbar->statisticsDockWidget);

	SampleEditor::statisticsToolbar->setTable(data, selection);
	this->flag = 1;	
}

void SampleEditor::updateStatistics(void)
{
	if( this->data->GetNumberOfRows() <= 0)
	{
		return;
	}

	if (this->flag == 1)
	{
		//std::cout<< "updattteeeee" << std::endl;
		statisticsToolbar->statisticsDockWidget->close();
		delete this->statisticsToolbar->statisticsDockWidget;
		this->statisticsToolbar->statisticsDockWidget = NULL;

		this->statisticsDockWidget = new QDockWidget();
		this->statisticsToolbar = new StatisticsToolbar(statisticsDockWidget);
		
		statisticsDockWidget->setWidget(statisticsToolbar->statisticsDockWidget);
		statisticsDockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
		addDockWidget(Qt::BottomDockWidgetArea, statisticsToolbar->statisticsDockWidget);

		SampleEditor::statisticsToolbar->setTable(data, selection);

		//showStatistics();
	}
}

void SampleEditor::STrenDAnalysisModel()
{
	if( spdkNNGWin)
	{
		delete spdkNNGWin;
	}
	spdkNNGWin = new STrenDkNNGModuleMatch();
	if( this->data->GetNumberOfRows() <= 0)
	{
		spdkNNGWin->setModels();
	}
	else
	{
		spdkNNGWin->setModels( this->data, selection);
	}
	spdkNNGWin->show();
}

void SampleEditor::AddLabel()
{
	std::cout<< "AddLabel."<<std::endl;
	std::set<long int> IDs = selection->getSelections();  // be cautious here
	std::cout<< IDs.size()<<std::endl;

	if( IDs.size() <= 0)
	{
		return;
	}

	if(this->data->GetColumnByName("prediction") == NULL)
	{
		std::cout<< "Adding label."<<std::endl;
		vtkSmartPointer<vtkDoubleArray> column = vtkSmartPointer<vtkDoubleArray>::New();
		column->SetName( "prediction");
		for(vtkIdType id = 0; id < this->data->GetNumberOfRows(); id++)
		{
			column->InsertNextValue(0);
		}
		this->data->AddColumn(column);
		std::cout<< "Adding label done."<<std::endl;
	}

	bool ok1;
	int label = QInputDialog::getInt(this, tr("Label"),tr("Label:"), 1, 1, 100, 1, &ok1);
	if(ok1)
	{
		std::cout<< "Set label."<<std::endl;
		std::set<long int>::iterator iter = IDs.begin();
		vtkIdType coln = this->data->GetNumberOfColumns();
		while( iter != IDs.end())
		{
			this->data->SetValue((vtkIdType)*iter, coln - 1, label);	
			iter++;
		}
	}
	ftk::SaveTable( "LabeledTableFromSampleEditor.txt", this->data);
}

void SampleEditor::DumpClusterSelections()
{
	//
	vtkIdType numClus = this->ClusterSelections->NumberOfClusters();
	std::cout << " Number of clusters: " <<  numClus << " Listed: " << std::endl;
	std::set< vtkIdType > clusterIDs =  this->ClusterSelections->GetClusterIDs();
	std::set< vtkIdType >::iterator iter = clusterIDs.begin();
	for (; iter != clusterIDs.end(); iter++)
	{
		std::cout << "\nTable for Cluster: " << *iter << std::endl;
		this->ClusterSelections->GetTableOfSelectedFromCluster(*iter)->Dump(16);
	}
		std::cout << "\nTable of all Clusters " << std::endl;
	this->ClusterSelections->GetTableOfAllSelected()->Dump(16);
}
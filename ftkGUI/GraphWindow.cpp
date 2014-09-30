#include "GraphWindow.h"
#include <vtkAnnotationLink.h>
#include <vtkCommand.h>
#include <vtkDataRepresentation.h>
#include <vtkSelectionNode.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkDoubleArray.h>
#include <vtkSelection.h>
#include <vtkMutableDirectedGraph.h>
#include <vtkMutableUndirectedGraph.h>
#include <vtkViewTheme.h>
#include <vtkDataSetAttributes.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkAbstractArray.h>
#include <vtkVariantArray.h>
#include <vtkStringArray.h>
#include <vtkCellPicker.h>
#include <vtkScalarBarActor.h>
#include <vtkTextProperty.h>
#include <vtkForceDirectedLayoutStrategy.h>
#include <QMessageBox>
#include <QInputDialog>
#include <fstream>
#include <vector>
#include <map>
#include <queue>
#include <math.h>
#include <iomanip>

#define pi 3.1415926
#define ANGLE_STEP 90
#define MAX_HOP 200
#define ANNOTATION_CORNER 0

static double selectColor[3]={0,1,0};
static double progressionPathColor[3] = {1,0,0};
static double edgeDefaultColor[3] = {0, 0, 0};

#define ROUND(X)(int)(X+0.5)
GraphWindow::GraphWindow(QWidget *parent)
: QMainWindow(parent)
{
	this->mainQTRenderWidget;// = new QVTKWidget;
	this->TTG = vtkSmartPointer<vtkTableToGraph>::New();
	this->view = vtkSmartPointer<vtkGraphLayoutView>::New();
	this->observerTag = 0;
	this->lookupTable = vtkSmartPointer<vtkLookupTable>::New();
	this->edgeLookupTable = vtkSmartPointer<vtkLookupTable>::New();
	bTrendStart = true;
	progressionStartID = -1;
	progressionEndID = -1;
	cornAnnotation = NULL;
}

GraphWindow::~GraphWindow()
{
}

void GraphWindow::setModels(vtkSmartPointer<vtkTable> table, ObjectSelection * sels, std::vector<int> *indexCluster, ObjectSelection * sels2)
{
	this->dataTable = table;
	this->indMapFromVertexToInd.clear();
	this->indMapFromIndToVertex.clear();

	for( long int i = 0; i < this->dataTable->GetNumberOfRows(); i++)
	{
		long int var = this->dataTable->GetValue( i, 0).ToLong();
		this->indMapFromVertexToInd.insert( std::pair< long int, long int>(var, i));
		this->indMapFromIndToVertex.push_back( var);
	}

	this->indMapFromVertexToClusInd.clear();
	for( int i = 0; i < this->indMapFromClusIndToVertex.size(); i++)
	{
		this->indMapFromClusIndToVertex[i].clear();
		this->indMapFromClusIndToInd[i].clear();
	}
	this->indMapFromClusIndToVertex.clear();
	this->indMapFromClusIndToInd.clear();

	if( indexCluster != NULL)
	{
		int clusterSize = 0;
		for( int i = 0; i < indexCluster->size(); i++)
		{
			if( (*indexCluster)[i] + 1 > clusterSize)
			{
				clusterSize = (*indexCluster)[i] + 1;
			}
			this->indMapFromVertexToClusInd.insert( std::pair< int, int>(this->indMapFromIndToVertex[i], (*indexCluster)[i]));
		}
			/// rebuild the datamatrix for MST 
		std::vector<int> clus;
		for( int i = 0; i < clusterSize; i++)
		{
			this->indMapFromClusIndToVertex.push_back(clus);
			this->indMapFromClusIndToInd.push_back(clus);
		}
		for( int i = 0; i < indexCluster->size(); i++)
		{
			int index = (*indexCluster)[i];
			this->indMapFromClusIndToVertex[index].push_back( this->indMapFromIndToVertex[i]);
			this->indMapFromClusIndToInd[index].push_back( i);
		}
	}

	if(!sels)
		this->selection = new ObjectSelection();
	else
		this->selection = sels;

	if(!sels2)
		this->selection2 = new ObjectSelection();
	else
		this->selection2 = sels2;
	connect( this->selection, SIGNAL( changed()), this, SLOT( UpdateGraphView()));
}
	

void GraphWindow::SetGraphTable(vtkSmartPointer<vtkTable> table)
{
	//graphTable->Dump(8);	//debug dump
	this->TTG->ClearLinkVertices();
	this->TTG->SetInputData(0, table);
	this->TTG->AddLinkEdge("Source", "Target"); 
	vtkSmartPointer<vtkViewTheme> theme = vtkSmartPointer<vtkViewTheme>::New();
	
	theme.TakeReference(vtkViewTheme::CreateMellowTheme());
	theme->SetLineWidth(5);
	theme->SetCellOpacity(0.9);
	theme->SetCellAlphaRange(0.5,0.5);
	theme->SetPointSize(10);
	theme->SetSelectedCellColor(1,0,1);
	theme->SetSelectedPointColor(1,0,1); 

	
	this->view->AddRepresentationFromInputConnection(TTG->GetOutputPort());
	this->view->SetEdgeLabelVisibility(true);
	this->view->SetEdgeLabelArrayName("Distance");
	this->view->SetLayoutStrategyToForceDirected();
	this->view->SetVertexLabelArrayName("label");
	this->view->VertexLabelVisibilityOn();
	this->view->SetVertexLabelFontSize(20);
}

void GraphWindow::SetGraphTable(vtkSmartPointer<vtkTable> table, std::string ID1, std::string ID2)
{
	//graphTable->Dump(8);	//debug dump
	this->TTG->ClearLinkVertices();
	this->TTG->SetInputData(0, table);
	this->TTG->AddLinkEdge(ID1.c_str(), ID2.c_str()); 
	vtkSmartPointer<vtkViewTheme> theme = vtkSmartPointer<vtkViewTheme>::New();
	
	theme.TakeReference(vtkViewTheme::CreateMellowTheme());
	theme->SetLineWidth(5);
	theme->SetCellOpacity(0.9);
	theme->SetCellAlphaRange(0.5,0.5);
	theme->SetPointSize(10);
	theme->SetSelectedCellColor(1,0,1);
	theme->SetSelectedPointColor(1,0,1); 

	this->view->AddRepresentationFromInputConnection(TTG->GetOutputPort());
	/*this->view->SetEdgeLabelVisibility(true);
	this->view->SetEdgeLabelArrayName("Distance");*/
	this->view->SetLayoutStrategyToForceDirected();
	this->view->SetVertexLabelArrayName("label");
	this->view->VertexLabelVisibilityOn();
	this->view->SetVertexLabelFontSize(20);
}

void GraphWindow::SetGraphTable(vtkSmartPointer<vtkTable> table, std::string ID1, std::string ID2, std::string edgeLabel, std::string xCol, std::string yCol, std::string zCol)
{
	std::cout<< "SetGraphTable"<<endl;

	vtkAbstractArray *arrayID1 = table->GetColumnByName( ID1.c_str());
	vtkAbstractArray *arrayID2 = table->GetColumnByName( ID2.c_str());
	vtkSmartPointer<vtkViewTheme> theme = vtkSmartPointer<vtkViewTheme>::New();
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

	vtkSmartPointer<vtkIntArray> vertexIDarrays = vtkSmartPointer<vtkIntArray>::New();
	vertexIDarrays->SetNumberOfComponents(1);
	vertexIDarrays->SetName("vertexIDarrays");

  // Create the edge weight array
	vtkSmartPointer<vtkDoubleArray> weights = vtkSmartPointer<vtkDoubleArray>::New();
	weights->SetNumberOfComponents(1);
	weights->SetName("edgeLabel");

	vtkSmartPointer<vtkMutableUndirectedGraph> graph = vtkMutableUndirectedGraph::New();
	for( int i = 0; i <  this->dataTable->GetNumberOfRows(); i++)
	{
		int vertexID = graph->AddVertex();
		//add coord points for pass through here
		double x = this->dataTable->GetValueByName(i, xCol.c_str()).ToDouble();
		double y = this->dataTable->GetValueByName(i, yCol.c_str()).ToDouble();
		double z = this->dataTable->GetValueByName(i, zCol.c_str()).ToDouble();
		points->InsertNextPoint(x,y,z);
		vertexIDarrays->InsertNextValue( this->indMapFromIndToVertex[i]);
	}

	for( vtkIdType i = 0; i < table->GetNumberOfRows(); i++)
	{
		long int ver1 = arrayID1->GetVariantValue(i).ToLong();
		long int ver2 = arrayID2->GetVariantValue(i).ToLong();
		std::map< long int, long int>::iterator iter1 = this->indMapFromVertexToInd.find( ver1);
		std::map< long int, long int>::iterator iter2 = this->indMapFromVertexToInd.find( ver2);
		if( iter1 != this->indMapFromVertexToInd.end() && iter2 != this->indMapFromVertexToInd.end())
		{
			long int index1 = iter1->second;
			long int index2 = iter2->second;
			graph->AddEdge( index1, index2);
			weights->InsertNextValue(table->GetValueByName(i, edgeLabel.c_str()).ToDouble());
		}
		else
		{
			QMessageBox msg;
			msg.setText("Index Mapping Error!");
			msg.exec();
			exit(-1);
		}
	}
	
	theme = vtkSmartPointer<vtkViewTheme>::New();
	theme.TakeReference(vtkViewTheme::CreateMellowTheme());
	theme->SetLineWidth(5);
	//theme->SetCellOpacity(0.9);
	//theme->SetCellAlphaRange(0.8,0.8);
	theme->SetPointSize(10);
	theme->SetSelectedCellColor(1,0,0);
	theme->SetSelectedPointColor(1,0,0);
	theme->SetVertexLabelColor(1,1,1);
	theme->SetBackgroundColor(0,0,0); 
	theme->SetBackgroundColor2(0,0,0);

	vtkSmartPointer<vtkIntArray> vertexColors = vtkSmartPointer<vtkIntArray>::New();
	vertexColors->SetNumberOfComponents(1);
	vertexColors->SetName("Color");
	
	this->lookupTable->SetNumberOfTableValues( this->dataTable->GetNumberOfRows());

	//std::cerr << "Number of Lookup Table values: " << this->dataTable->GetNumberOfRows() << std::endl;

	for( vtkIdType i = 0; i < this->dataTable->GetNumberOfRows(); i++)
	{
		vertexColors->InsertNextValue( i);
		this->lookupTable->SetTableValue(i, 1, 0, 0); // color the vertices- red
    }
	lookupTable->Build();

	graph->GetVertexData()->AddArray(vertexColors);
	graph->GetVertexData()->AddArray(vertexIDarrays);
	graph->GetEdgeData()->AddArray(weights);
	graph->SetPoints(points);

	this->view->AddRepresentationFromInput( graph);
	this->view->SetEdgeLabelVisibility(true);
	this->view->SetColorVertices(true); 
	this->view->SetVertexLabelVisibility(true);

	this->view->SetVertexColorArrayName("Color");

    theme->SetPointLookupTable(lookupTable);
	this->view->ApplyViewTheme(theme);

	this->view->SetEdgeLabelArrayName("edgeLabel");

	//this->view->SetLayoutStrategyToForceDirected();
	this->view->SetLayoutStrategyToPassThrough();

	this->view->SetVertexLabelArrayName("vertexIDarrays");
	this->view->SetVertexLabelFontSize(20);
}

void GraphWindow::SetTreeTable(vtkSmartPointer<vtkTable> table, std::string ID1, std::string ID2, std::string edgeLabel, std::vector<double> *colorVec,
							    std::vector<double> *disVec, std::set<long int>* colSels, QString filename)
{
	this->fileName = filename;
	vtkAbstractArray *arrayID1 = table->GetColumnByName( ID1.c_str());
	vtkAbstractArray *arrayID2 = table->GetColumnByName( ID2.c_str());
	if( colSels)
	{
		this->colSelectIDs = *colSels;
	}

	
	if( colorVec != NULL)
	{
		std::cout<< "Set color Vector"<<std::endl;
		colorVector.set_size(colorVec->size());
		for( int i = 0; i < colorVec->size(); i++)
		{
			double color = (*colorVec)[i];
			colorVector[i] = color;
		}
	}
	else
	{
		colorVector.set_size(table->GetNumberOfRows() + 1);
		for( int i = 0; i < table->GetNumberOfRows() + 1; i++)
		{
			colorVector[i] = (COLOR_MAP_SIZE - 16.0) / COLOR_MAP_SIZE;
		}
	}
	featureColorVector = colorVector;


	vnl_vector<double> distanceVec;
	if(disVec) 
	{
		distanceVec.set_size( disVec->size());
		for( int i = 0; i < disVec->size(); i++)
		{
			distanceVec[i] = (*disVec)[i];
		}
	}
	else
	{
		distanceVec.set_size(table->GetNumberOfRows() + 1);
		for( int i = 0; i < table->GetNumberOfRows() + 1; i++)
		{
			distanceVec[i] = -1;
		}
	}

	vtkSmartPointer<vtkViewTheme> theme = vtkSmartPointer<vtkViewTheme>::New();
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	

	vtkSmartPointer<vtkIntArray> vertexIDarrays = vtkSmartPointer<vtkIntArray>::New();
	vertexIDarrays->SetNumberOfComponents(1);
	vertexIDarrays->SetName("vertexIDarrays");

	vtkSmartPointer<vtkStringArray> vertexLabel = vtkSmartPointer<vtkStringArray>::New();
	vertexLabel->SetNumberOfComponents(1);
	vertexLabel->SetName("vertexLabel");

	// Create the edge weight array
	vtkSmartPointer<vtkDoubleArray> weights = vtkSmartPointer<vtkDoubleArray>::New();
	weights->SetNumberOfComponents(1);
	weights->SetName("edgeLabel");

	std::cout<< "construct graph"<<endl;
	vtkSmartPointer<vtkMutableUndirectedGraph> graph = vtkMutableUndirectedGraph::New();
	vnl_matrix<long int> adj_matrix( table->GetNumberOfRows() + 1, table->GetNumberOfRows() + 1);
	
	vertextList.set_size( table->GetNumberOfRows(), 3);
	edgeWeights.set_size( table->GetNumberOfRows());

	double minPer = colorVector.min_value();
	double maxDisper = distanceVec.max_value();

	for( int i = 0; i < table->GetNumberOfRows() + 1; i++)
	{
		int vertexID = graph->AddVertex();
		//int index = floor( i / 4.0);
		QString str;
		//if (index <= 10)
		//{
		//	str = QString::number(0.05 + index * 0.1);
		//}
		//else
		//{
		//	str = QString("real");
		//}

		if (this->indMapFromClusIndToVertex.size() <= 0)
		{
			if( this->indMapFromIndToVertex.size() > 0)
			{
				vertexIDarrays->InsertNextValue( this->indMapFromIndToVertex[i]);
				str = QString::number(this->indMapFromIndToVertex[i]);
			}
			else
			{
				vertexIDarrays->InsertNextValue( i);
				str = QString::number(i);
			}
			vertexLabel->InsertNextValue( str.toUtf8().constData());
		}
		else
		{
			vertexIDarrays->InsertNextValue( this->indMapFromClusIndToVertex[i].size());   // which should be the cluster index
			//int per = colorVector[i] * 100 + 0.5;
			//int disper = distanceVec[i] * 100 + 0.5;
			//QString strPercent;
			//QString strDisPercent;

			//if(  minPer < 1 - 1e-5)
			//{
			//	if( per > 100)
			//	{
			//		per = 100;
			//	}
			//	strPercent= QString::number(per);
			//}

			//if( maxDisper > 1e-5)
			//{
			//	if( disper > 100)
			//	{
			//		disper = 100;
			//	}
			//	strDisPercent = QString::number(disper);
			//}

			//if( strPercent.length() > 0 )
			//{
			//	if( strDisPercent.length() > 0)
			//	{
			//		strPercent = "(" + strPercent + "%," + strDisPercent + "%)";
			//	}
			//	else
			//	{
			//		strPercent = "(" + strPercent + "%)";
			//	}
			//}
			//else if( strDisPercent.length() > 0)
			//{
			//	strPercent = "( ," + strDisPercent + "%)";
			//}
			
			if( this->indMapFromClusIndToVertex[i].size() == 1)
			{
				str = QString::number(this->indMapFromIndToVertex[i]);
				vertexLabel->InsertNextValue(str.toUtf8().constData());
			}
			else
			{
				str = QString::number(this->indMapFromClusIndToVertex[i].size());
				vertexLabel->InsertNextValue(str.toUtf8().constData());
			}
		}
	}

	//edgeMapping.clear();

	for( int i = 0; i < table->GetNumberOfRows(); i++) 
	{

		long int ver1 = arrayID1->GetVariantValue(i).ToLong();
		long int ver2 = arrayID2->GetVariantValue(i).ToLong();

		//if (this->indMapFromClusIndToVertex.size() > 0)
		//{
			//std::map< long int, long int>::iterator iter1 = this->indMapFromVertexToInd.find( ver1);
			//std::map< long int, long int>::iterator iter2 = this->indMapFromVertexToInd.find( ver2);
			//if( iter1 != this->indMapFromVertexToInd.end() && iter2 != this->indMapFromVertexToInd.end())
			//{
				long int index1 = ver1;
				long int index2 = ver2;
				graph->AddEdge( index1, index2);

				std::pair< long int, long int> pair1 = std::pair< long int, long int>( index1, index2);
				std::pair< long int, long int> pair2 = std::pair< long int, long int>( index2, index1);
				//edgeMapping.insert( std::pair< std::pair< long int, long int>, long int>(pair1, i));
				//edgeMapping.insert( std::pair< std::pair< long int, long int>, long int>(pair2, i));

				adj_matrix( index1, index2) = 1;
				adj_matrix( index2, index1) = 1;
				vertextList( i, 0) = index1;
				vertextList( i, 1) = index2;
				edgeWeights[i] = table->GetValueByName(vtkIdType(i), edgeLabel.c_str()).ToDouble();
				weights->InsertNextValue(table->GetValueByName(vtkIdType(i), edgeLabel.c_str()).ToDouble());
		//	}
		//	else
		//	{
		//		QMessageBox msg;
		//		msg.setText("Index Mapping Error!");
		//		msg.exec();
		//		exit(-1);
		//	}
		//}
		//else    // cluster index, no mapping
		//{
		//	graph->AddEdge( ver1, ver2);
		//	adj_matrix( ver1, ver2) = 1;
		//	adj_matrix( ver2, ver1) = 1;
		//	vertextList( i, 0) = ver1;
		//	vertextList( i, 1) = ver2;
		//	edgeWeights[i] = table->GetValueByName(vtkIdType(i), edgeLabel.c_str()).ToDouble();
		//	weights->InsertNextValue(table->GetValueByName(vtkIdType(i), edgeLabel.c_str()).ToDouble());
		//}
	}

	//edgeWeights = edgeWeights - edgeWeights.mean();
	//double std = edgeWeights.two_norm();
	//if( std != 0)
	//{
	//	edgeWeights = edgeWeights / std;
	//	edgeWeights = ( edgeWeights - edgeWeights.min_value()) / ( edgeWeights.max_value() - edgeWeights.min_value()) + 0.1;
	//	edgeWeights = 2.0 * edgeWeights;
	//}
	edgeWeights = edgeWeights / edgeWeights.max_value() * 5;
	for(int i = 0; i < vertextList.rows(); i++)
	{
		vertextList( i, 2) = edgeWeights[i];
	}

	//std::ofstream verofs("adjmatrix.txt");
	//verofs<< adj_matrix<<endl;
	//verofs.close();

	std::cout<< "calculate coordinates"<<endl;
	std::vector<Point> oldPointList;
	std::vector<Point> newPointList;
	CalculateCoordinates(adj_matrix, oldPointList);
	newPointList = oldPointList;
	//UpdateCoordinatesByEdgeWeights( oldPointList, vertextList, newPointList);

	//std::vector<Point> newPointList;
	//CalculateCoordinates(adj_matrix, newPointList);

	if( newPointList.size() > 0)
	{
		//std::ofstream ofs("Coordinates.txt");
		//ofs<< "3\t5"<<std::endl;
		std::cout<< newPointList.size()<<endl;
		for( int i = 0; i <  newPointList.size(); i++)
		{
			//ofs<<newPointList[i].x<<"\t"<<newPointList[i].y<<std::endl;
			points->InsertNextPoint(newPointList[i].x, newPointList[i].y, 0);
		}
		graph->SetPoints( points);
		//ofs.close();
	}
	else
	{	
		QMessageBox msg;
		msg.setText("No coordinates Input!");
		msg.exec();
		exit(-2);
	}

	vtkSmartPointer<vtkIntArray> vertexColors = vtkSmartPointer<vtkIntArray>::New();
	vertexColors->SetNumberOfComponents(1);
	vertexColors->SetName("Color");
	this->lookupTable->SetNumberOfTableValues( table->GetNumberOfRows() + 1);

	for( vtkIdType i = 0; i < table->GetNumberOfRows() + 1; i++)               
	{             
		vertexColors->InsertNextValue( i);
		int k = int( featureColorVector[i] * COLOR_MAP_SIZE + 0.5);
		if( k >= COLOR_MAP_SIZE)
		{
			k = COLOR_MAP_SIZE - 1;
		}
		this->lookupTable->SetTableValue(i, COLORMAP[k].r, COLORMAP[k].g, COLORMAP[k].b); 
	}
	lookupTable->Build();

	vtkSmartPointer<vtkIntArray> edgeColors = vtkSmartPointer<vtkIntArray>::New();

	edgeColors->SetNumberOfComponents(1);
	edgeColors->SetName("EdgeColor");
	this->edgeLookupTable->SetNumberOfTableValues( table->GetNumberOfRows());
	for( vtkIdType i = 0; i < table->GetNumberOfRows(); i++)               
	{ 
		edgeColors->InsertNextValue(i); // color the edges by default color
		this->edgeLookupTable->SetTableValue(i, edgeDefaultColor[0], edgeDefaultColor[1], edgeDefaultColor[2]);
	}

	graph->GetVertexData()->AddArray(vertexColors);
	graph->GetVertexData()->AddArray(vertexIDarrays);
	graph->GetVertexData()->AddArray(vertexLabel);
	graph->GetEdgeData()->AddArray(weights);
	graph->GetEdgeData()->AddArray(edgeColors);
	
	//theme.TakeReference(vtkViewTheme::CreateMellowTheme());
	theme->SetLineWidth(3);
	theme->SetCellOpacity(0.9);
	theme->SetCellAlphaRange(0.8,0.8);
	theme->SetPointSize(10);
	//theme->SetCellColor(0.6,0.6,0.6);
	theme->SetSelectedCellColor(selectColor);
	theme->SetSelectedPointColor(selectColor); 
	theme->SetVertexLabelColor(0.3,0.3,0.3);
	theme->SetBackgroundColor(1,1,1); 
	theme->SetBackgroundColor2(1,1,1);

	vtkSmartPointer<vtkLookupTable> scalarbarLut = vtkSmartPointer<vtkLookupTable>::New();
	scalarbarLut->SetTableRange (0, 1);
	scalarbarLut->SetNumberOfTableValues(COLOR_MAP_SIZE);
	for(int index = 0; index < COLOR_MAP_SIZE; index++)
	{
		rgb rgbscalar = COLORMAP[index];
		scalarbarLut->SetTableValue(index, rgbscalar.r, rgbscalar.g, rgbscalar.b);
	}
	scalarbarLut->Build();

	vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
	scalarBar->SetLookupTable(scalarbarLut);
	scalarBar->SetTitle("Color Map");
	scalarBar->SetNumberOfLabels(11);
	scalarBar->GetTitleTextProperty()->SetColor(0,0,0);
	scalarBar->GetTitleTextProperty()->SetFontSize(10);
	scalarBar->GetLabelTextProperty()->SetColor(0,0,0);
	scalarBar->GetTitleTextProperty()->SetFontSize (10);
	scalarBar->SetMaximumHeightInPixels(1000);
	scalarBar->SetMaximumWidthInPixels(100);

	this->view->GetRenderer()->AddActor2D(scalarBar);

	this->view->RemoveAllRepresentations();
	this->view->SetRepresentationFromInput( graph);
	this->view->SetEdgeLabelVisibility(true);
	this->view->SetColorVertices(true); 
	this->view->SetColorEdges(true);
	this->view->SetVertexLabelVisibility(true);

	this->view->SetVertexColorArrayName("Color");
	this->view->SetEdgeColorArrayName("EdgeColor");
	this->view->SetEdgeLabelArrayName("edgeLabel");
	this->view->SetVertexLabelArrayName("vertexLabel");
	this->view->SetVertexLabelFontSize(5);

    theme->SetPointLookupTable(lookupTable);
	theme->SetCellLookupTable(edgeLookupTable);
	this->view->ApplyViewTheme(theme);

	this->view->SetLayoutStrategyToPassThrough();
	this->view->SetVertexLabelFontSize(15);
}

void GraphWindow::AdjustedLayout(vtkSmartPointer<vtkTable> table, std::string ID1, std::string ID2, std::string edgeLabel, std::vector<long int> *treeOrder, std::vector<double> *colorVec, std::vector<double> *disVec)
{
	std::vector<Point> newPointList;
	std::ifstream file("Coordinates.txt");
	double x = 0;
	double y = 0;
	int linewidth = 3;
	int pointSize = 10;
	file >> linewidth;
	file >> pointSize;

	while(!file.eof())
    {
        file >> x;
        file >> y;
        Point pt(x,y);
		newPointList.push_back(pt);
    }
	newPointList.pop_back();

	vtkAbstractArray *arrayID1 = table->GetColumnByName( ID1.c_str());
	vtkAbstractArray *arrayID2 = table->GetColumnByName( ID2.c_str());

	vnl_vector<int> label;
	if(treeOrder)
	{
		label.set_size(treeOrder->size());
		for( int i = 0; i < treeOrder->size(); i++)
		{
			label[(*treeOrder)[i]] = i;
		}
	}

	if( colorVec)
	{
		colorVector.set_size(colorVec->size());
		for( int i = 0; i < colorVec->size(); i++)
		{
			double color = (*colorVec)[i];
			colorVector[i] = color;
		}
	}
	else
	{
		colorVector.set_size(table->GetNumberOfRows() + 1);
		for( int i = 0; i < table->GetNumberOfRows() + 1; i++)
		{
			colorVector[i] = 1;
		}
	}
	featureColorVector = colorVector;


	vnl_vector<double> distanceVec;
	if(disVec) 
	{
		distanceVec.set_size( disVec->size());
		for( int i = 0; i < disVec->size(); i++)
		{
			distanceVec[i] = (*disVec)[i];
		}
	}
	else
	{
		distanceVec.set_size(table->GetNumberOfRows() + 1);
		for( int i = 0; i < table->GetNumberOfRows() + 1; i++)
		{
			distanceVec[i] = -1;
		}
	}

	vtkSmartPointer<vtkViewTheme> theme = vtkSmartPointer<vtkViewTheme>::New();
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	

	vtkSmartPointer<vtkIntArray> vertexIDarrays = vtkSmartPointer<vtkIntArray>::New();
	vertexIDarrays->SetNumberOfComponents(1);
	vertexIDarrays->SetName("vertexIDarrays");

	vtkSmartPointer<vtkStringArray> vertexLabel = vtkSmartPointer<vtkStringArray>::New();
	vertexLabel->SetNumberOfComponents(1);
	vertexLabel->SetName("vertexLabel");

	// Create the edge weight array
	vtkSmartPointer<vtkDoubleArray> weights = vtkSmartPointer<vtkDoubleArray>::New();
	weights->SetNumberOfComponents(1);
	weights->SetName("edgeLabel");

	std::cout<< "construct graph"<<endl;
	vtkSmartPointer<vtkMutableUndirectedGraph> graph = vtkMutableUndirectedGraph::New();
	vnl_matrix<long int> adj_matrix( table->GetNumberOfRows() + 1, table->GetNumberOfRows() + 1);
	
	vertextList.set_size( table->GetNumberOfRows(), 3);
	edgeWeights.set_size( table->GetNumberOfRows());

	double minPer = colorVector.min_value();
	double maxDisper = distanceVec.max_value();

	for( int i = 0; i < table->GetNumberOfRows() + 1; i++)
	{
		int vertexID = graph->AddVertex();
		if (this->indMapFromClusIndToVertex.size() <= 0)
		{
			vertexIDarrays->InsertNextValue( this->indMapFromIndToVertex[i]);
			QString str = QString::number(this->indMapFromIndToVertex[i]);
			vertexLabel->InsertNextValue( str.toUtf8().constData());
		}
		else
		{
			vertexIDarrays->InsertNextValue( this->indMapFromClusIndToVertex[i].size());   // which should be the cluster index
			int per = colorVector[i] * 100 + 0.5;
			int disper = distanceVec[i] * 100 + 0.5;
			QString strPercent;
			QString strDisPercent;

			if(  minPer < 1 - 1e-5)
			{
				if( per > 100)
				{
					per = 100;
				}
				strPercent= QString::number(per);
			}

			if( maxDisper > 1e-5)
			{
				if( disper > 100)
				{
					disper = 100;
				}
				strDisPercent = QString::number(disper);
			}

			if( strPercent.length() > 0 )
			{
				if( strDisPercent.length() > 0)
				{
					strPercent = "(" + strPercent + "%," + strDisPercent + "%)";
				}
				else
				{
					strPercent = "(" + strPercent + "%)";
				}
			}
			else if( strDisPercent.length() > 0)
			{
				strPercent = "( ," + strDisPercent + "%)";
			}
			
			QString str;
			if(label.size() > 0)
			{
				str = "C"+QString::number(label[i])+": "+QString::number(this->indMapFromClusIndToVertex[i].size()) + strPercent;
			}
			else
			{
				str = QString::number(this->indMapFromClusIndToVertex[i].size()) + strPercent;
			}
			vertexLabel->InsertNextValue(str.toUtf8().constData());
		}
	}

	//edgeMapping.clear();

	for( int i = 0; i < table->GetNumberOfRows(); i++) 
	{

		long int ver1 = arrayID1->GetVariantValue(i).ToLong();
		long int ver2 = arrayID2->GetVariantValue(i).ToLong();

		long int index1 = ver1;
		long int index2 = ver2;
		graph->AddEdge( index1, index2);

		std::pair< long int, long int> pair1 = std::pair< long int, long int>( index1, index2);
		std::pair< long int, long int> pair2 = std::pair< long int, long int>( index2, index1);
		//edgeMapping.insert( std::pair< std::pair< long int, long int>, long int>(pair1, i));
		//edgeMapping.insert( std::pair< std::pair< long int, long int>, long int>(pair2, i));

		adj_matrix( index1, index2) = 1;
		adj_matrix( index2, index1) = 1;
		vertextList( i, 0) = index1;
		vertextList( i, 1) = index2;
		edgeWeights[i] = table->GetValueByName(vtkIdType(i), edgeLabel.c_str()).ToDouble();
		weights->InsertNextValue(table->GetValueByName(vtkIdType(i), edgeLabel.c_str()).ToDouble());
	}

	edgeWeights = edgeWeights / edgeWeights.max_value() * 5;
	for(int i = 0; i < vertextList.rows(); i++)
	{
		vertextList( i, 2) = edgeWeights[i];
	}

	if( newPointList.size() > 0)
	{
		std::cout<< newPointList.size()<<endl;
		for( int i = 0; i <  newPointList.size(); i++)
		{
			points->InsertNextPoint(newPointList[i].x, newPointList[i].y, 0);
		}
		graph->SetPoints( points);
	}
	else
	{	
		QMessageBox msg;
		msg.setText("No coordinates Input!");
		msg.exec();
		exit(-2);
	}

	vtkSmartPointer<vtkIntArray> vertexColors = vtkSmartPointer<vtkIntArray>::New();
	vertexColors->SetNumberOfComponents(1);
	vertexColors->SetName("Color");
	this->lookupTable->SetNumberOfTableValues( table->GetNumberOfRows() + 1);

	for( vtkIdType i = 0; i < table->GetNumberOfRows() + 1; i++)               
	{             
		vertexColors->InsertNextValue( i);
		int k = int( featureColorVector[i] * COLOR_MAP_SIZE + 0.5);
		if( k >= COLOR_MAP_SIZE)
		{
			k = COLOR_MAP_SIZE - 1;
		}
		this->lookupTable->SetTableValue(i, COLORMAP[k].r, COLORMAP[k].g, COLORMAP[k].b); 
	}
	lookupTable->Build();

	vtkSmartPointer<vtkIntArray> edgeColors = vtkSmartPointer<vtkIntArray>::New();

	edgeColors->SetNumberOfComponents(1);
	edgeColors->SetName("EdgeColor");
	this->edgeLookupTable->SetNumberOfTableValues( table->GetNumberOfRows());
	for( vtkIdType i = 0; i < table->GetNumberOfRows(); i++)               
	{ 
		edgeColors->InsertNextValue(i); // color the edges by default color
		this->edgeLookupTable->SetTableValue(i, edgeDefaultColor[0], edgeDefaultColor[1], edgeDefaultColor[2]);
	}

	graph->GetVertexData()->AddArray(vertexColors);
	graph->GetVertexData()->AddArray(vertexIDarrays);
	graph->GetVertexData()->AddArray(vertexLabel);
	graph->GetEdgeData()->AddArray(weights);
	graph->GetEdgeData()->AddArray(edgeColors);
	
	//theme.TakeReference(vtkViewTheme::CreateMellowTheme());
	theme->SetLineWidth(linewidth);
	theme->SetCellOpacity(0.9);
	theme->SetCellAlphaRange(0.8,0.8);
	theme->SetPointSize(pointSize);
	//theme->SetCellColor(0.6,0.6,0.6);
	theme->SetSelectedCellColor(selectColor);
	theme->SetSelectedPointColor(selectColor); 
	theme->SetVertexLabelColor(0.3,0.3,0.3);
	theme->SetBackgroundColor(1,1,1); 
	theme->SetBackgroundColor2(1,1,1);

	vtkSmartPointer<vtkLookupTable> scalarbarLut = vtkSmartPointer<vtkLookupTable>::New();
	scalarbarLut->SetTableRange (0, 1);
	scalarbarLut->SetNumberOfTableValues(COLOR_MAP_SIZE);
	for(int index = 0; index < COLOR_MAP_SIZE; index++)
	{
		rgb rgbscalar = COLORMAP[index];
		scalarbarLut->SetTableValue(index, rgbscalar.r, rgbscalar.g, rgbscalar.b);
	}
	scalarbarLut->Build();

	vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
	scalarBar->SetLookupTable(scalarbarLut);
	scalarBar->SetTitle("Color Map");
	scalarBar->SetNumberOfLabels(11);
	scalarBar->GetTitleTextProperty()->SetColor(0,0,0);
	scalarBar->GetTitleTextProperty()->SetFontSize(10);
	scalarBar->GetLabelTextProperty()->SetColor(0,0,0);
	scalarBar->GetTitleTextProperty()->SetFontSize (10);
	scalarBar->SetMaximumHeightInPixels(1000);
	scalarBar->SetMaximumWidthInPixels(100);

	this->view->GetRenderer()->AddActor2D(scalarBar);

	this->view->RemoveAllRepresentations();
	this->view->SetRepresentationFromInput( graph);
	this->view->SetEdgeLabelVisibility(true);
	this->view->SetColorVertices(true); 
	this->view->SetColorEdges(true);
	this->view->SetVertexLabelVisibility(true);

	this->view->SetVertexColorArrayName("Color");
	this->view->SetEdgeColorArrayName("EdgeColor");
	this->view->SetEdgeLabelArrayName("edgeLabel");
	this->view->SetVertexLabelArrayName("vertexLabel");
	this->view->SetVertexLabelFontSize(5);

    theme->SetPointLookupTable(lookupTable);
	theme->SetCellLookupTable(edgeLookupTable);
	this->view->ApplyViewTheme(theme);

	this->view->SetLayoutStrategyToPassThrough();
	this->view->SetVertexLabelFontSize(15);
}

bool GraphWindow::FindCycle(vnl_matrix<unsigned char> &adjacent_matrix, std::vector< std::list<int> > &cycleList, std::vector<int> &seperatePts)  
{  
	bool bCycles = false;
	cycleList.clear();
	unsigned int maxNeighborNum = 0;
	int startingnode = 0;
	for( int i = 0; i < adjacent_matrix.rows(); i++)
	{
		vnl_vector< unsigned char> rowi = adjacent_matrix.get_row(i);
		if( rowi.sum() > maxNeighborNum)
		{
			maxNeighborNum = rowi.sum();
			startingnode = i;
		}
	}

	std::vector< std::list<int> > cycleNodeLongest;
	cycleNodeLongest.resize(adjacent_matrix.rows());

	std::list<int> cycleNode;
	cycleNode.push_back(startingnode); // starting point
	std::set<int> path;
	path.insert(startingnode);  // set node 0 as the starting point
    SearchCycles(adjacent_matrix, cycleNode, startingnode, -1, path, cycleNodeLongest);

	for( int i = 0; i < cycleNodeLongest.size(); i++)
	{
		if( cycleNodeLongest[i].size() > 0)
		{  
			bCycles = true;  
			cycleList.push_back(cycleNodeLongest[i]);
		}  
		else
		{
			seperatePts.push_back(i);
		}
	}
 
    return bCycles;  
}  
  
void GraphWindow::SearchCycles(vnl_matrix<unsigned char> &adjacent_matrix, std::list<int> &cycleNode, int i, int preNode, std::set<int> &path, std::vector< std::list<int> >&cycleLongest)
{  
	if( cycleNode.size() <= 4)
	{
		std::list<int>::iterator iter = cycleNode.begin();
		for(; iter != cycleNode.end(); iter++)
		{
			std::cout<< *iter<<"\t";
		}
		std::cout<< std::endl;
	}

    for (int j = 0; j < adjacent_matrix.rows(); ++j)   
    {  
        if ( adjacent_matrix[i][j] == 1 && j != preNode)  // not going backward or form small loop
        {  
			if( path.find(j) == path.end())  
			{  
				path.insert(j);
				cycleNode.push_back(j);
				SearchCycles(adjacent_matrix, cycleNode, j, i, path, cycleLongest);
				std::set<int>::iterator iter = path.find(j);
				path.erase(iter);
				cycleNode.pop_back();
			}  
			else
			{  
				int cycleSize = path.size() + 1;
				std::list<int>::iterator iter = cycleNode.begin();
				int oriNode = *iter;
				if( oriNode == j)  // the circle belongs to every node on the circle
				{
					for(; iter != cycleNode.end(); iter++)
					{
						int node = *iter;
						if(cycleSize > cycleLongest[node].size())
						{
							cycleLongest[node].clear();
							std::list<int>::iterator iter2 = cycleNode.begin();
							for(; iter2 != cycleNode.end(); iter2++)
							{
								cycleLongest[node].push_back(*iter2);
							}
							cycleLongest[node].push_back(j);
						}
					}
				}
				else
				{
					for(iter = cycleNode.begin(); iter != cycleNode.end(); iter++)
					{
						if(cycleSize <= cycleLongest[j].size())
						{
							break;
						}
						if(*iter == j)
						{
							cycleLongest[j].clear();
							for(; iter != cycleNode.end(); iter++)
							{
								cycleLongest[j].push_back(*iter);
							}
							cycleLongest[j].push_back(j);
							break;
						}
						cycleSize--;
					}
				}
			}  
        }  
    }  
}  

template<class T>
vnl_vector< T > GraphWindow::VectorAnd(vnl_vector< T > const &v, vnl_vector< T > const &u)
{
	vnl_vector< T> vu(v.size());
	for( unsigned int i = 0; i < v.size(); i++)
	{
		vu[i] = v[i] & u[i];
	}
	return vu;
}

template<class T>
vnl_vector< T > GraphWindow::VectorOr(vnl_vector< T > const &v, vnl_vector< T > const &u)
{
	vnl_vector< T> vu(v.size());
	for( unsigned int i = 0; i < v.size(); i++)
	{
		vu[i] = v[i] | u[i];
	}
	return vu;
}

void GraphWindow::MergeCircles(std::list<int> &circle1, std::list<int> &circle2, vnl_vector<int> &commonNode, std::vector< std::list<int> > &leftCycleList)
{
	std::vector< std::list<int> > circleLine1;
	BreakCircles(circle1, commonNode, circleLine1);
	std::vector< std::list<int> > circleLine2;
	BreakCircles(circle2, commonNode, circleLine2);
	circle1.clear();
	
	for(int i = 0; i < circleLine1.size(); i++)
	{
		if( circleLine1[i].size() >= circleLine2[i].size())
		{
			std::list<int> line = circleLine1[i];
			std::list<int>::iterator lineIter = line.begin();
			for(; lineIter != line.end(); lineIter++)
			{
				if( lineIter != line.end())
				{
					circle1.push_back(*lineIter);
				}
			}
			leftCycleList.push_back(circleLine2[i]);
		}
		else
		{
			std::list<int> line = circleLine2[i];
			std::list<int>::iterator lineIter = line.begin();
			for(; lineIter != line.end(); lineIter++)
			{
				if( lineIter != line.end())
				{
					circle1.push_back(*lineIter);
				}
			}
			leftCycleList.push_back(circleLine1[i]);
		}
	}
}

void GraphWindow::BreakCircles(std::list<int> &circle, vnl_vector<int> &commonNode, std::vector< std::list<int> > &lines)   // have more than 2 common nodes in the middle
{
	std::cout<< commonNode<<std::endl;
	lines.clear();
	std::list<int> beginLine;  // if 0, then save as beginLine
	std::list<int>::iterator iter = circle.begin();
	int node = *iter;
	
	if( iter != circle.end() && commonNode[node] == 0)
	{
		while( commonNode[node] == 0)
		{
			beginLine.push_back(node);
			iter++;
			node = *iter;
		}
		beginLine.push_back(node);   // end pt
	}

	while( commonNode[*iter] == 1) // iter pts to 0
	{
		node = *iter;
		iter++;
	}

	while( iter != circle.end())
	{
		std::list< int> midLine;
		midLine.push_back(node);
		while( iter != circle.end() && commonNode[*iter] == 0)
		{
			midLine.push_back( *iter);
			iter++;
		}

		if( iter != circle.end())
		{
			midLine.push_back( *iter);   // last 1
		}

		while( iter != circle.end() && commonNode[*iter] == 1)
		{
			node = *iter;   // first 1
			iter++;
		}
		lines.push_back(midLine);
	}

	if( beginLine.size() > 0)
	{
		std::list<int>::iterator beginIter = beginLine.begin();
		int endInd = lines.size() - 1;
		for(beginIter++; beginIter != beginLine.end(); beginIter++)
		{
			lines[endInd].push_back(*beginIter);
		}
	}
}

void GraphWindow::GetConnectedLines(vnl_vector<int> &circle1, std::list<int> &circle2, vnl_vector<int> &commonNode, std::vector< std::list<int> > &lineList)
{
	std::vector< std::list<int> > circleLine2;
	BreakCircles(circle2, commonNode, circleLine2);  // only save the lines that have non-common nodes
	lineList.clear();
	for(int i = 0; i < circleLine2.size(); i++)
	{
		std::list<int> line = circleLine2[i];
		std::list<int>::iterator iter = line.begin();
		//iter++;
		//int node = *iter;
		//if( circle1[node] == 0)
		//{
			lineList.push_back( line);
		//}
	}
}

int GraphWindow::MergeCyclesToSuperNodes(vnl_matrix<unsigned char> &adj_matrix, std::vector< std::list<int> > &cycleList, std::vector< std::vector<std::list<int> > >&superNodeList, vnl_matrix<int> &superNodeConnection)
{
	// order the cycles according to the size
	for( int i = 0; i < cycleList.size(); i++)
	{
		for( int j = i + 1; j < cycleList.size(); j++)
		{
			if( cycleList[i].size() < cycleList[j].size())
			{
				std::list<int> tmp = cycleList[j];
				cycleList[j] = cycleList[i];
				cycleList[i] = tmp;
			}
		}
	}

	std::vector< std::vector<std::list<int> > > superNodeLineList;
	vnl_vector<int> tag(cycleList.size());
	vnl_vector<int> merged_tag(cycleList.size());
	merged_tag.fill(0);
	vnl_matrix< int> SuperNodesSet( cycleList.size(), adj_matrix.cols());   // nodes in the super-node, info about cycleList
	SuperNodesSet.fill(0);
	for( int i = 0; i < cycleList.size(); i++)
	{
		tag[i] = i;  // merge status
		std::list<int> iCycle = cycleList[i];
		std::list<int>::iterator iter = iCycle.begin();
		for( ; iter != iCycle.end(); iter++)
		{
			SuperNodesSet(i, *iter) = 1;
		}
	}

    // only consider the same circle and overlapping problem
	for(unsigned int i = 0; i < SuperNodesSet.rows(); i++)
	{
		std::vector< std::list<int> > lines;
		if( tag[i] == i)  // if this circle is included by other circles, it cannot include other circles itself.
		{
			vnl_vector<int> rowi = SuperNodesSet.get_row(i);

			for( unsigned int j = i + 1; j < SuperNodesSet.rows(); j++)
			{
				if( tag[j] == j)    
				{
					vnl_vector<int> rowj = SuperNodesSet.get_row(j);
					vnl_vector<int> commonVec = VectorAnd<int>(rowi, rowj);
					if( commonVec.sum() == rowj.sum())
					{
						tag[j] = -1;
					}
					else if( commonVec.sum() >= 2 && commonVec.sum() < rowj.sum())  // have at least two common nodes
					{
						tag[j] = i;  
						merged_tag[j] = 1;  // this circle is broken into lines, merged with other circle
						vnl_vector<int> orVec = VectorOr<int>(rowi, rowj);
						std::vector< std::list<int> > tmplines;
						GetConnectedLines(rowi, cycleList[j], commonVec, tmplines);
						for(int k = 0; k < tmplines.size(); k++)
						{
							lines.push_back(tmplines[k]);
						}
					}
				}
			}
		}
		superNodeLineList.push_back(lines);
	}
	
    // now consider the circle connection
	superNodeList.clear();
    std::vector< std::list<int> > newcycleList;
    int countCycles = 0;

	vnl_vector<int> adjust_num(tag.size());   // save for the circle id adjust excluding the overlapped circles.
	adjust_num.fill(0);
	int negCount = 0;
	for( int ind = 0; ind < tag.size(); ind++)
	{
		if( tag[ind] >= 0 && merged_tag[ind] == 0)
		{
			countCycles++;
			newcycleList.push_back(cycleList[ind]);   // these are non-overlapping circles
			superNodeList.push_back( superNodeLineList[ind]);
			adjust_num[ind] = negCount;
		}
		else
		{
			negCount++;
			adjust_num[ind] = negCount;
		}
	}
	cycleList = newcycleList;

	superNodeConnection.set_size(countCycles, countCycles);
	superNodeConnection.fill(-1);
	for( unsigned int i = 0; i < SuperNodesSet.rows(); i++)
	{
		if( tag[i] >= 0)
		{
			vnl_vector<int> rowi = SuperNodesSet.get_row(i);
			for( unsigned int j = i + 1; j < SuperNodesSet.rows(); j++)    // have only one common node, supernode connection
			{
				if( tag[j] >= 0 )
				{
					vnl_vector<int> rowj = SuperNodesSet.get_row(j);
					vnl_vector<int> commonVec = VectorAnd<int>(rowi, rowj);
					if( commonVec.sum() == 1 )
					{
						for( unsigned int k = 0; k < commonVec.size(); k++)
						{
							if( commonVec[k] == 1)
							{
                                int indi = tag[i];
                                int indj = tag[j];
								indi = tag[i] - adjust_num[indi];
								indj = tag[j] - adjust_num[indj];
								superNodeConnection(indi, indj) = k;  // which node is connected
								superNodeConnection(indj, indi) = k;
								break;
							}
						}
					}
				}
			}
		}
	}
	
	return countCycles;
}

long int GraphWindow::IsConnectedSuperNodeToNode(vnl_matrix<unsigned char> &adj_matrix, int node, std::list<int> &superNodePt)
{
    std::list<int>::iterator iter = superNodePt.begin();
    long int rtn = 0;
    for(; iter != superNodePt.end(); iter++)
    {
        int sn = *iter;
        if( adj_matrix( node, sn) == 1)
        {
            rtn = 1;
            break;
        }
    }
    return rtn;
}

void GraphWindow::SetGraphTableToPassThrough(vtkSmartPointer<vtkTable> table, unsigned int nodesNumber, std::string ID1, std::string ID2, std::string edgeLabel, std::vector<double> *colorVec,
							    std::vector<double> *disVec, std::set<long int>* colSels, QString filename)
{
	this->fileName = filename;
	vtkAbstractArray *arrayID1 = table->GetColumnByName( ID1.c_str());
	vtkAbstractArray *arrayID2 = table->GetColumnByName( ID2.c_str());

	vnl_matrix<unsigned char> adj_matrix( nodesNumber, nodesNumber); // adjacent matrix
	adj_matrix.fill(0);

	for( int i = 0; i < table->GetNumberOfRows(); i++) 
	{
		double ver1 = arrayID1->GetVariantValue(i).ToDouble();
		double ver2 = arrayID2->GetVariantValue(i).ToDouble();

		int index1 = ROUND(ver1);
		int index2 = ROUND(ver2);
		adj_matrix( index1, index2) = 1;
		adj_matrix( index2, index1) = 1;
	}

	//test
	//vnl_matrix<unsigned char> adj_matrix( 15, 15); // adjacent matrix
	//adj_matrix.fill(0);
	//adj_matrix(0,1) = adj_matrix(1, 0) = 1;
	//adj_matrix(0,4) = adj_matrix(4, 0) = 1;
	//adj_matrix(2,1) = adj_matrix(1, 2) = 1;
	//adj_matrix(14,1) = adj_matrix(1, 14) = 1;
	//adj_matrix(14,5) = adj_matrix(5, 14) = 1;
	//adj_matrix(3,2) = adj_matrix(2, 3) = 1;
	//adj_matrix(6,2) = adj_matrix(2, 6) = 1;
	//adj_matrix(3,7) = adj_matrix(7, 3) = 1;
	//adj_matrix(3,8) = adj_matrix(8, 3) = 1;
	//adj_matrix(3,9) = adj_matrix(9, 3) = 1;
	//adj_matrix(8,9) = adj_matrix(9, 8) = 1;
	//adj_matrix(10,9) = adj_matrix(9, 10) = 1;
	//adj_matrix(4,5) = adj_matrix(5, 4) = 1;
	//adj_matrix(6,5) = adj_matrix(5, 6) = 1;
	//adj_matrix(6,7) = adj_matrix(7, 6) = 1;
	//adj_matrix(11,7) = adj_matrix(7, 11) = 1;
	//adj_matrix(12,7) = adj_matrix(7, 12) = 1;
	//adj_matrix(11,12) = adj_matrix(12, 11) = 1;
	//adj_matrix(11,13) = adj_matrix(13, 11) = 1;
	std::ofstream ofs("CycleList.txt");
	ofs<< adj_matrix<<std::endl;

	std::vector< std::list<int> > cycleList;
	std::vector< int> seperateNodes;
	FindCycle(adj_matrix, cycleList, seperateNodes);
	std::vector< std::vector<std::list<int> > > superNodeList;   // super-nodes for the progression tree, each supernode has a list of cycles, trees or nodes
	vnl_matrix<int> connectionMatrix;
	MergeCyclesToSuperNodes(adj_matrix, cycleList, superNodeList, connectionMatrix);

	for( int i = 0; i < cycleList.size(); i++)
	{
		std::list<int> cycle = cycleList[i];
		std::list<int>::iterator listIter = cycle.begin();
		std::vector< std::list<int> > linesList = superNodeList[i];
		ofs<< "CycleList:"<<std::endl;
		ofs<<i<<std::endl;
		for(; listIter != cycle.end(); listIter++)
		{
			ofs<< *listIter<<"\t";
		}
		ofs<<std::endl;
		ofs<< "Lines:"<<std::endl;
		for( int j = 0; j < linesList.size(); j++)
		{
			std::list<int> lines = linesList[j];
			std::list<int>::iterator lineIter = lines.begin();
			for(; lineIter != lines.end(); lineIter++)
			{
				ofs<< *lineIter<<"\t";
			}
			ofs<< std::endl;
		}
		ofs<<std::endl;
	}
	ofs<< connectionMatrix<<std::endl;
	ofs<< "Seperate Nodes:"<<std::endl;
	for(int i = 0; i < seperateNodes.size(); i++)
	{
		ofs<<seperateNodes[i]<<"\t";
	}
	ofs<<std::endl;
	
	std::vector< std::list<int> > superNodePtSet;
	for( int i = 0; i < cycleList.size(); i++)
	{
        std::list<int> superNodePt;
		std::list<int> cycle = cycleList[i];
		std::list<int>::iterator listIter = cycle.begin();
		listIter++;
		std::vector< std::list<int> > linesList = superNodeList[i];
		for(; listIter != cycle.end(); listIter++)
		{
			superNodePt.push_back(*listIter);
		}
		for( int j = 0; j < linesList.size(); j++)
		{
			std::list<int> lines = linesList[j];
			std::list<int>::iterator lineIter = lines.begin();
			lineIter++;
			for(; lineIter != lines.end(); lineIter++)
			{
				superNodePt.push_back(*lineIter);
			}
			superNodePt.pop_back();
		}
		superNodePtSet.push_back(superNodePt);
	}

    // build tree for supernodes
	int superNodeCount = connectionMatrix.rows();
	std::vector<int> nodeIndex;  // supernode, normal node
	std::map<int, int> nodeIndexMap;  // 
	for(unsigned int index = 0; index < connectionMatrix.rows(); index++)
	{
        nodeIndex.push_back( index);  // supernode
		nodeIndexMap.insert( std::pair<int, int>(index, index));
	}
	
	for( unsigned int i = 0; i < seperateNodes.size(); i++) // normal node
	{
        nodeIndex.push_back( seperateNodes[i]);
		nodeIndexMap.insert( std::pair<int, int>(seperateNodes[i], superNodeCount + i));
	}
	
	unsigned int treeNodeSize = nodeIndex.size();
	vnl_matrix<long int> adjacent_matrix(treeNodeSize, treeNodeSize);
	adjacent_matrix.fill(0);

	for(unsigned int i = 0; i < superNodeCount; i++)
	{
		for( unsigned int j = i + 1; j < superNodeCount; j++)
		{
			if( connectionMatrix(i,j) >= 0)
			{
				adjacent_matrix(j, i) = 1;
				adjacent_matrix(i, j) = 1;
			}
		}
		for( unsigned int j = superNodeCount; j < treeNodeSize; j++)
		{
            int node = nodeIndex[j];
            adjacent_matrix(i, j) = IsConnectedSuperNodeToNode(adj_matrix, node, superNodePtSet[i]);  // watch for the case that within connected small circle.
            adjacent_matrix(j, i) = adjacent_matrix(i, j);
		}
	}

	for( unsigned int i = superNodeCount; i < treeNodeSize; i++)
	{
        int nodei = nodeIndex[i];
        for( unsigned int j = i + 1; j < treeNodeSize; j++)
        {
            int nodej = nodeIndex[j];
            adjacent_matrix(i, j) = adj_matrix(nodei, nodej);
            adjacent_matrix(j, i) = adjacent_matrix(i,j);
        }
	}
	
	ofs<< adjacent_matrix<<std::endl;
	ofs.close();

	vtkSmartPointer<vtkStringArray> vertexLabel = vtkSmartPointer<vtkStringArray>::New();
	vertexLabel->SetNumberOfComponents(1);
	vertexLabel->SetName("vertexLabel");

	// coordinates for the supernode and their connection
	std::cout<< "construct graph"<<endl;
	vtkSmartPointer<vtkMutableUndirectedGraph> graph = vtkMutableUndirectedGraph::New();
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	for( int i = 0; i < treeNodeSize; i++)
	{
		int vertexID = graph->AddVertex();
		if( i < superNodePtSet.size())
		{
			//vertexIDarrays->InsertNextValue( this->indMapFromIndToVertex[i]);
			QString str = QString::number(superNodePtSet[i].size());
			vertexLabel->InsertNextValue( str.toUtf8().constData());
		}
		else
		{
            QString str = QString::number(1);
            vertexLabel->InsertNextValue( str.toUtf8().constData());
		}
	}
	
	for( unsigned int i = 0; i < treeNodeSize; i++)
	{
        for( unsigned int j = i + 1; j < treeNodeSize; j++)
        {
            if( adjacent_matrix(i, j) == 1)
            {
                graph->AddEdge( i, j);
            }
        }
	} 
    std::cout<< "calculate coordinates for supernodes"<<endl;
	std::vector<Point> pointList;
	CalculateCoordinates(adjacent_matrix, pointList);

	if( pointList.size() > 0)
	{
		std::cout<< pointList.size()<<endl;
		for( int i = 0; i <  pointList.size(); i++)
		{
			points->InsertNextPoint(pointList[i].x, pointList[i].y, 0);
		}
	}

	// coordinates for the circle
	std::cout<< "calculate coordinates for cicles"<<endl;
	double radius = 5;
	for( int i = 0; i < superNodeCount; i++)
	{
		std::vector<Point> pointListSuperNode;
		std::list<int> cycle = cycleList[i];
		int nodesAdded = cycle.size() - 1;
		cycle.pop_back();
		std::list<int>::iterator cycleIter = cycle.begin();
		int oriVertex = graph->AddVertex();
		nodeIndex.push_back(*cycleIter);
		QString str = QString::number(1);
		vertexLabel->InsertNextValue( str.toUtf8().constData());

		nodeIndexMap.insert( std::pair<int, int>(*cycleIter, oriVertex));

		int preVertex = oriVertex;
		for( cycleIter++; cycleIter != cycle.end(); cycleIter++)
		{
			int nextVertex = graph->AddVertex();
			nodeIndex.push_back(*cycleIter);
			QString str = QString::number(1);
			vertexLabel->InsertNextValue( str.toUtf8().constData());
			nodeIndexMap.insert( std::pair<int, int>(*cycleIter, nextVertex));

			graph->AddEdge( preVertex, nextVertex);
			preVertex = nextVertex;
		}
		graph->AddEdge( preVertex, oriVertex);

		std::vector< std::list<int> > linesList = superNodeList[i];
		for( int k = 0; k < linesList.size(); k++)
		{
			std::list<int> line = linesList[k];
			nodesAdded += line.size() - 2;
			std::list<int>::iterator lineIter = line.begin();
			std::list<int>::iterator lineLastIter = line.end();
			lineLastIter--;
			int lastVertex = *lineLastIter;
			line.pop_back();

			int preVertex = *lineIter;
			for( lineIter++; lineIter != line.end(); lineIter++)
			{
				int nextVertex = graph->AddVertex();
				nodeIndex.push_back(*lineIter);
				QString str = QString::number(1);
				vertexLabel->InsertNextValue( str.toUtf8().constData());
				nodeIndexMap.insert( std::pair<int, int>(*lineIter, nextVertex));
				graph->AddEdge( preVertex, nextVertex);
				preVertex = nextVertex;
			}
			graph->AddEdge( preVertex, lastVertex);
		}

		CalculateCoordinatesForCircle(cycleList[i], superNodeList[i], pointList[i], radius, pointListSuperNode);
		if( pointListSuperNode.size() == nodesAdded)
		{
			for( int k = 0; k <  pointListSuperNode.size(); k++)
			{
				points->InsertNextPoint(pointListSuperNode[k].x, pointListSuperNode[k].y, 0);
			}
		}
		else
		{
			std::cout<< "Error in circle layout "<< i<<std::endl;
		}
	}
	
	std::cout<< "Vertex Lable "<< vertexLabel->GetDataSize()<<std::endl;
	std::cout<< "Points "<< points->GetNumberOfPoints()<<std::endl;
	graph->GetVertexData()->AddArray(vertexLabel);
	graph->SetPoints( points);
	vtkSmartPointer<vtkViewTheme> theme = vtkSmartPointer<vtkViewTheme>::New();
	theme->SetLineWidth(3);
	theme->SetCellOpacity(0.9);
	theme->SetCellAlphaRange(0.8,0.8);
	theme->SetPointSize(10);
	theme->SetCellColor(0.6,0.6,0.6);
	theme->SetSelectedCellColor(selectColor);
	theme->SetSelectedPointColor(selectColor); 
	theme->SetVertexLabelColor(0.3,0.3,0.3);
	theme->SetBackgroundColor(1,1,1); 
	theme->SetBackgroundColor2(1,1,1);

	vtkSmartPointer<vtkLookupTable> scalarbarLut = vtkSmartPointer<vtkLookupTable>::New();
	scalarbarLut->SetTableRange (0, 1);
	scalarbarLut->SetNumberOfTableValues(COLOR_MAP_SIZE);
	for(int index = 0; index < COLOR_MAP_SIZE; index++)
	{
		rgb rgbscalar = COLORMAP[index];
		scalarbarLut->SetTableValue(index, rgbscalar.r, rgbscalar.g, rgbscalar.b);
	}
	scalarbarLut->Build();

	vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
	scalarBar->SetLookupTable(scalarbarLut);
	scalarBar->SetTitle("Color Map");
	scalarBar->SetNumberOfLabels(11);
	scalarBar->GetTitleTextProperty()->SetColor(0,0,0);
	scalarBar->GetTitleTextProperty()->SetFontSize(10);
	scalarBar->GetLabelTextProperty()->SetColor(0,0,0);
	scalarBar->GetTitleTextProperty()->SetFontSize (10);
	scalarBar->SetMaximumHeightInPixels(1000);
	scalarBar->SetMaximumWidthInPixels(100);

	this->view->GetRenderer()->AddActor2D(scalarBar);

	this->view->RemoveAllRepresentations();
	this->view->SetRepresentationFromInput( graph);
	this->view->SetVertexLabelVisibility(true);
	this->view->SetVertexLabelArrayName("vertexLabel");
	this->view->SetVertexLabelFontSize(5);

	this->view->ApplyViewTheme(theme);

    this->view->SetLayoutStrategyToPassThrough();
	this->view->SetVertexLabelFontSize(15);
}

void GraphWindow::UpdateCoordinatesByEdgeWeights(std::vector<Point>& oldPointList, vnl_matrix<double>& vertexList, std::vector<Point>& newPointList)
{
	long int oldFirstIndex = backbones[0];
	long int oldSecondIndex = 0;
	newPointList[ oldFirstIndex] = oldPointList[oldFirstIndex];
	Point newFirst = oldPointList[oldFirstIndex];
	Point oldFirst = oldPointList[oldFirstIndex];
	Point oldSecond(0,0);
	for( int i = 1; i < backbones.size(); i++)
	{
		oldSecondIndex = backbones[i];
		oldSecond = oldPointList[oldSecondIndex];
		double weight = GetEdgeWeight( vertexList, oldFirstIndex, oldSecondIndex);
		Point newSecond = GetNewPointFromOldPoint(oldFirst, oldSecond, newFirst, weight);
		newPointList[ oldSecondIndex] = newSecond;
		newFirst = newSecond;
		oldFirst = oldSecond;
		oldFirstIndex = oldSecondIndex;
		UpdateChainPointList(oldSecondIndex, oldPointList, vertexList, newPointList);
	}
}

void GraphWindow::CalculateCoordinatesForCircle(std::list<int> &circle, std::vector< std::list<int> > lineList, Point &center, double radius, std::vector<Point> &pointList)
{
    std::list<int>::iterator iter = circle.begin();
    circle.pop_back();
    int circleSize = circle.size();
    double step = 2 * 3.1415926 / circleSize;
    std::map<int, int> nodeMap;
    for( int i = 0; i < circleSize; i++)
    {
        double angle = step * i;
        Point pt;
        pt.x = center.x + cos( angle);
        pt.y = center.y + sin( angle);
        pointList.push_back(pt);
        nodeMap.insert(std::pair<int, int>( *iter, i));
        iter++;
    }
    for( int i = 0; i < lineList.size(); i++)
    {
        std::list<int> line = lineList[i];
        std::list<int>::iterator lineIter = line.begin();
        std::list<int>::iterator lineIterEnd = line.end();
        lineIterEnd--;
        int startNode = *lineIter;
        int endNode = *lineIterEnd;
        int startIndex = nodeMap[ startNode];
        Point startPt = pointList[ startIndex];
        int endIndex = nodeMap[ endNode];
        Point endPt = pointList[ endIndex];
        line.pop_back();
        double stepX = abs(endPt.x - startPt.x) / (double)line.size();
        double stepY = abs(endPt.y - startPt.y) / (double)line.size();
        int signX = endPt.x > startPt.x ? 1 : -1;
        int signY = endPt.y > startPt.y ? 1 : -1;
        for( int k = 1; k < line.size(); k++)
        {
            Point pt;
            pt.x = startPt.x + k * signX * stepX;
            pt.y = startPt.y + k * signY * stepY;
            pointList.push_back(pt);
        }
    }
}

// attachnode's coordinate has been updated already in pointlist
void GraphWindow::UpdateChainPointList(long int attachnode, std::vector<Point>& oldPointList, vnl_matrix<double>& vertexList, std::vector<Point>& newPointList)
{
	for( long int i = 0; i < chainList.size(); i++)
	{
		std::pair<long int, std::vector<long int> > pair = chainList[i];
		if( pair.first == attachnode)
		{
 			Point oldFirst = oldPointList[ attachnode];
			Point newFirst = newPointList[ attachnode];
			std::vector<long int> vec = pair.second;	
			Point oldSecond(0,0);
			long int oldFirstIndex = attachnode; 
			long int oldSecondIndex = 0;
			for( long int j = 0; j < vec.size(); j++)
			{
				oldSecondIndex = vec[j];
				oldSecond = oldPointList[ oldSecondIndex];
				double weight = GetEdgeWeight( vertexList, oldFirstIndex, oldSecondIndex);
				Point newSecond = GetNewPointFromOldPoint(oldFirst, oldSecond, newFirst, weight);
				newPointList[oldSecondIndex] = newSecond;
				newFirst = newSecond;
				oldFirst = oldSecond;
				oldFirstIndex = oldSecondIndex;	

				UpdateChainPointList(oldSecondIndex, oldPointList, vertexList, newPointList);
			}
		}
	}
}

double GraphWindow::GetEdgeWeight( vnl_matrix<double>& vertexList, long firstIndex, long secondIndex)
{
	double weight;
	for( int i = 0; i < vertexList.rows(); i++)
	{
		if( vertexList(i, 0) == firstIndex && vertexList( i, 1)== secondIndex
			|| vertexList(i, 0) ==  secondIndex && vertexList(i, 1) ==  firstIndex)
		{
			weight = vertexList(i,2);
			break;
		}
	}
	return weight;
}

/// calculate the new point nodes based on the two old points and their weight
Point GraphWindow::GetNewPointFromOldPoint( Point &oldPointFirst, Point &oldPointSecond, Point &newPointFirst, double weight)
{
	Point newpoint(0,0);
	double xd = (oldPointFirst.x - oldPointSecond.x) * (oldPointFirst.x - oldPointSecond.x);
	double yd = (oldPointFirst.y - oldPointSecond.y) * (oldPointFirst.y - oldPointSecond.y);
	double distance = sqrt( xd + yd);

	newpoint.x = newPointFirst.x + ( oldPointSecond.x - oldPointFirst.x) * weight / distance;
	newpoint.y = newPointFirst.y + ( oldPointSecond.y - oldPointFirst.y) * weight /  distance;
	return newpoint;
}

void GraphWindow::ShowGraphWindow()
{
	this->mainQTRenderWidget.SetRenderWindow(view->GetRenderWindow());
	this->mainQTRenderWidget.resize(600, 600);
	this->mainQTRenderWidget.show();
	std::cout<< "view->ResetCamera"<<endl;
	view->ResetCamera();

	if( cornAnnotation == NULL)
	{
		cornAnnotation = vtkSmartPointer<vtkCornerAnnotation>::New();
		cornAnnotation->SetText(ANNOTATION_CORNER, "Colored by Device Sample Percentage");
		cornAnnotation->GetTextProperty()->SetFontSize(10);
		cornAnnotation->GetTextProperty()->SetColor(0,0,0);
		this->view->GetRenderer()->AddActor2D(cornAnnotation);
	}
	else
	{
		cornAnnotation->SetText(ANNOTATION_CORNER, "Colored by Device Sample Percentage");
		cornAnnotation->GetTextProperty()->SetFontSize(10);
		cornAnnotation->GetTextProperty()->SetColor(0,0,0);
	}

	std::cout<< "view->Render"<<endl;
	view->Render();
	std::cout<< "Successfully rendered"<<endl;

    this->selectionCallback = vtkSmartPointer<vtkCallbackCommand>::New(); 
    this->selectionCallback->SetClientData(this);
    this->selectionCallback->SetCallback ( SelectionCallbackFunction);
	vtkAnnotationLink *link = view->GetRepresentation()->GetAnnotationLink();
	this->observerTag = link->AddObserver(vtkCommand::AnnotationChangedEvent, this->selectionCallback);

	this->keyPress = vtkSmartPointer<vtkCallbackCommand>::New();
	this->keyPress->SetCallback(HandleKeyPress);
	this->keyPress->SetClientData(this);
	this->view->GetInteractor()->AddObserver(vtkCommand::KeyPressEvent, this->keyPress);

	view->GetInteractor()->Start();
}

void GraphWindow::SelectionCallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData )
{
	vtkAnnotationLink* annotationLink = static_cast<vtkAnnotationLink*>(caller);
	vtkSelection* selection = annotationLink->GetCurrentSelection();
	GraphWindow* graphWin = (GraphWindow*)clientData;

	vtkSelectionNode* vertices = NULL;
	vtkSelectionNode* edges = NULL;

	if( selection->GetNode(0))
	{
		if( selection->GetNode(0)->GetFieldType() == vtkSelectionNode::VERTEX)
		{
			vertices = selection->GetNode(0);
		}
		else if( selection->GetNode(0)->GetFieldType() == vtkSelectionNode::EDGE)
		{
			edges = selection->GetNode(0);
		}
	}

	if( selection->GetNode(1))
	{
		if( selection->GetNode(1)->GetFieldType() == vtkSelectionNode::VERTEX)
		{
			vertices = selection->GetNode(1);
		}
		else if( selection->GetNode(1)->GetFieldType() == vtkSelectionNode::EDGE)
		{
			edges = selection->GetNode(1);
		}
	}

	if( vertices != NULL)
	{
		vtkIdTypeArray* vertexList = vtkIdTypeArray::SafeDownCast(vertices->GetSelectionList());
		if(vertexList->GetNumberOfTuples() > 0)
		{
			int key = graphWin->view->GetInteractor()->GetControlKey();
			if( 0 == key)
			{
				std::cout<< vertexList->GetValue(0)<<endl;
				std::set<long int> IDs;
				for( vtkIdType i = 0; i < vertexList->GetNumberOfTuples(); i++)
				{
					long int value = vertexList->GetValue(i);
					IDs.insert(value);
				}
				graphWin->SetSelectedIds( IDs);
				graphWin->SetSelectedIds2();  // only select the selected feature columns
				graphWin->SetTrendStartTag(true);
			}
			else
			{
				long int value = vertexList->GetValue(0);
				graphWin->SetUserDefineTrend(value);
			}
		}
		else
		{
			graphWin->SetTrendStartTag(true);
		}
	}
	//graphWin->mainQTRenderWidget.GetRenderWindow()->Render();
	//graphWin->view->GetRenderer()->Render();
}

void GraphWindow::HandleKeyPress(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData )
{
	GraphWindow* graphWin = (GraphWindow*)clientData;
	char key = graphWin->view->GetInteractor()->GetKeyCode();
	switch (key)
	{
	case 'r':
		graphWin->RestoreLookupTable();
		break;
	default:
		break;
	}
}

void GraphWindow::RestoreLookupTable()
{
	for( int i = 0; i < colorVector.size(); i++)               
	{
		colorVector[i] = featureColorVector[i];
		int k = int( featureColorVector[i] * COLOR_MAP_SIZE + 0.5);
		if( k >= COLOR_MAP_SIZE)
		{
			k = COLOR_MAP_SIZE - 1;
		}
		this->lookupTable->SetTableValue(i, COLORMAP[k].r, COLORMAP[k].g, COLORMAP[k].b); 
	}
	lookupTable->Build();
	//this->view->GetRenderer()->RemoveActor2D(cornAnnotation);
	cornAnnotation->SetText(ANNOTATION_CORNER, "Colored by Device Sample Percentage");
	cornAnnotation->GetTextProperty()->SetFontSize(10);
	cornAnnotation->GetTextProperty()->SetColor(0,0,0);
	//this->view->GetRenderer()->AddActor2D(cornAnnotation);
	this->view->GetRenderer()->Render();
	this->selection->clear();
}

void GraphWindow::SetTrendStartTag(bool bstart)
{
	bTrendStart = bstart;
}

void GraphWindow::SetUserDefineTrend(long int nodeID)
{
	if( bTrendStart)
	{
		progressionStartID = nodeID;
		bTrendStart = false;
		std::cout<< "progression start: "<<progressionStartID<<endl;
	}
	else
	{
		progressionEndID = nodeID;
		bTrendStart = true;
		std::cout<< "progression end: "<<progressionEndID<<endl;
		UpdateTrendPath();
	}
}

void GraphWindow::ResetLookupTable(vtkSmartPointer<vtkLookupTable> lookuptable, double* color)
{
	for( int i = 0; i < lookuptable->GetNumberOfTableValues(); i++)
	{
		lookuptable->SetTableValue( i, color[0], color[1], color[2]);
	}
}

void GraphWindow::UpdateTrendPath()
{
	progressionPath.clear();
	ResetLookupTable( edgeLookupTable, edgeDefaultColor);
	GetTrendPath(shortest_hop, progressionStartID, progressionEndID, progressionPath);

	for( int i = 0; i < progressionPath.size() - 1; i++)
	{
		std::pair<long int, long int> pair = std::pair<long int, long int>( progressionPath[i], progressionPath[i + 1]);
		std::map< std::pair< long int, long int>, long int>::iterator iter = edgeMapping.find( pair);
		if( iter != edgeMapping.end())
		{
			vtkIdType index = (vtkIdType)iter->second;
			edgeLookupTable->SetTableValue( index, 0, 0 ,0);
		}
	}
	//this->mainQTRenderWidget.GetRenderWindow()->Render();
	this->view->GetRenderer()->Render();
}

void GraphWindow::GetTrendPath(vnl_matrix<long int> &hopMat, long int startNode, long int endNode, std::vector< long int> &path)
{
	std::map< long int, long int> tmpChain;  // for ordering 
	long int maxhop = hopMat( startNode, endNode);

	for( long int i = 0; i < shortest_hop.cols(); i++)
	{
		if( shortest_hop( startNode, i) + shortest_hop( endNode, i) == maxhop)
		{
			tmpChain.insert( std::pair< long int, long int>( shortest_hop( startNode, i), i));
		}
	}

	path.clear();
	std::map< long int, long int>::iterator iter;
	for( iter = tmpChain.begin(); iter != tmpChain.end(); iter++)
	{
		path.push_back((*iter).second);
	}
}

ObjectSelection * GraphWindow::GetSelection()
{
	return selection;
}

void GraphWindow::SetSelectedIds(std::set<long int>& IDs)
{
	if( IDs.size() > 0)
	{
		std::set<long int> selectedIDs;
		std::set<long int>::iterator iter = IDs.begin();
		while( iter != IDs.end())
		{
			if( this->indMapFromClusIndToVertex.size() <= 0)
			{
				long int var = indMapFromIndToVertex[*iter];
				selectedIDs.insert( var);
			}
			else
			{
				std::vector<int> clusVertex = indMapFromClusIndToVertex[*iter];
				for( int ind = 0; ind < clusVertex.size(); ind++)
				{
					selectedIDs.insert(clusVertex[ind]);
				}
			}
			iter++;
		}
		this->selection->select( selectedIDs);
		UpdataLookupTable( selectedIDs);
	}
	this->view->GetRenderer()->Render();
}

void GraphWindow::SetSelectedIds2()
{
	if( this->colSelectIDs.size() > 0)
	{
		this->selection2->select( this->colSelectIDs);
	}
}

void GraphWindow::UpdataLookupTable( std::set<long int>& IDs)
{
	std::set<long int> selectedIDs; 
	std::set<long int>::iterator iter = IDs.begin();

	while( iter != IDs.end())
	{
		long int var = 0;
		if( this->indMapFromClusIndToVertex.size() <= 0)
		{
			var = this->indMapFromVertexToInd.find( *iter)->second;
		}
		else
		{
			var = this->indMapFromVertexToClusInd.find( *iter)->second;
		}

		if( selectedIDs.find( var) == selectedIDs.end())   // selectIDs doesn't have var
		{
			selectedIDs.insert( var);
		}
		iter++;
	}
	//std::cerr << "Number of Lookup Table values: " << this->dataTable->GetNumberOfRows() << std::endl;
	int vertexnum = 0;
	if( this->indMapFromClusIndToVertex.size() <= 0)
	{
		vertexnum = this->dataTable->GetNumberOfRows();
	}
	else
	{
		vertexnum = this->indMapFromClusIndToVertex.size();  // vertex size equals cluster size
	}

	for( vtkIdType i = 0; i < vertexnum; i++)
	{
		if (selectedIDs.find(i) != selectedIDs.end())
		{
			this->lookupTable->SetTableValue(i, selectColor[0], selectColor[1], selectColor[2]); // color the vertices
		}
		else if ( featureColorVector.size() > 0)
		{
			int k = int( featureColorVector[i] * COLOR_MAP_SIZE + 0.5);
			if( k >= COLOR_MAP_SIZE)
			{
				k = COLOR_MAP_SIZE - 1;
			}
			this->lookupTable->SetTableValue(i, COLORMAP[k].r, COLORMAP[k].g, COLORMAP[k].b); // color the vertices- blue
		}
		else
		{
			this->lookupTable->SetTableValue(i,0,0,1);
		}
	}

	this->lookupTable->Build();
}

void GraphWindow::UpdateGraphView()
{
	std::set<long int> IDs;
	std::set<long int> IDsnew = this->selection->getSelections();
	if( this->indMapFromClusIndToVertex.size() <= 0)
	{
		IDs = IDsnew;
	}
	else
	{
		std::set<long int>::iterator iter = IDsnew.begin();
		while( iter != IDsnew.end())
		{
			if( this->indMapFromVertexToClusInd.find( *iter) != this->indMapFromVertexToClusInd.end()) 
			{
				int var = this->indMapFromVertexToClusInd.find( *iter)->second;

				if( IDs.find( var) == IDs.end())   // selectIDs doesn't have var
				{
					IDs.insert( var);
				}
				iter++;
			}
		}
	}

	if( this->view->GetRepresentation())
	{
		vtkAnnotationLink* annotationLink = this->view->GetRepresentation()->GetAnnotationLink();

		if( this->observerTag != 0)
		{
			annotationLink->RemoveObserver( this->observerTag);
			this->observerTag = 0;
		}

		vtkSelection* selection = annotationLink->GetCurrentSelection();
	
		vtkSmartPointer<vtkIdTypeArray> vertexList = vtkSmartPointer<vtkIdTypeArray>::New();
		vertexList->SetNumberOfComponents(1);

		std::set<long int>::iterator iter = IDs.begin();
		for( int id = 0; id < IDs.size(); id++, iter++)
		{
			vertexList->InsertNextValue( *iter);
		}

		vtkSmartPointer<vtkSelectionNode> selectNodeList = vtkSmartPointer<vtkSelectionNode>::New();
		selectNodeList->SetSelectionList( vertexList);
		selectNodeList->SetFieldType( vtkSelectionNode::VERTEX);
		selectNodeList->SetContentType( vtkSelectionNode::INDICES);

		selection->RemoveAllNodes();
		selection->AddNode(selectNodeList);
		annotationLink->SetCurrentSelection( selection);

		if( this->colSelectIDs.size() > 0)
		{
			selection2->select( this->colSelectIDs);    // only select the selected feature columns
		}

		std::set<long int> updataIDes =  this->selection->getSelections();
		UpdataLookupTable( updataIDes);

		this->mainQTRenderWidget.GetRenderWindow()->Render();
		this->view->GetRenderer()->Render();
		this->observerTag = annotationLink->AddObserver("AnnotationChangedEvent", this->selectionCallback);
	}
}

void GraphWindow::GetTreeNodeBetweenDistance(vtkSmartPointer<vtkTable> table, std::string ID1, std::string ID2, std::string edgeLabel, vnl_matrix<double> &disMat)
{
	unsigned int nodesNumber = table->GetNumberOfRows() + 1;
	vtkAbstractArray *arrayID1 = table->GetColumnByName( ID1.c_str());
	vtkAbstractArray *arrayID2 = table->GetColumnByName( ID2.c_str());
	vtkAbstractArray *arrayID3 = table->GetColumnByName( edgeLabel.c_str());

	disMat.set_size( nodesNumber, nodesNumber); // edge weight matrix
	vnl_matrix<unsigned char> tagMat(nodesNumber, nodesNumber);
	disMat.fill(0);
	tagMat.fill(0);

	for( int i = 0; i < table->GetNumberOfRows(); i++) 
	{
		int ver1 = arrayID1->GetVariantValue(i).ToInt();
		int ver2 = arrayID2->GetVariantValue(i).ToInt();
		double weight = arrayID3->GetVariantValue(i).ToDouble();

		disMat( ver1, ver2) = weight;
		disMat( ver2, ver1) = weight;
		tagMat( ver1, ver2) = 1;
		tagMat( ver2, ver1) = 1;
	}

	/// Generate between distance matrix
	bool bchanged = true;
	while( bchanged)
	{
		bchanged = false;
		for( unsigned int i = 0; i < disMat.rows(); i++)
		{
			for( unsigned int j = i + 1; j < disMat.cols(); j++)
			{
				if( tagMat(i,j) == 0)
				{
					bchanged = true;
					for( unsigned int k = 0; k < disMat.cols(); k++)
					{
						if( k != i && k != j && tagMat(i,k) == 1 && tagMat(k,j) == 1)
						{
							disMat(i, j) = disMat(i,k) + disMat(k,j);
							disMat(j, i) = disMat(i, j);
							tagMat(i, j) = 1;
							tagMat(j, i) = 1;
							break;
						}
					}
				}
			}
		}
	}
}

void GraphWindow::CalculateCoordinates(vnl_matrix<long int>& adj_matrix, std::vector<Point>& pointList)
{
	shortest_hop.set_size(adj_matrix.rows(), adj_matrix.cols());
	vnl_vector<long int> mark(adj_matrix.rows());
	shortest_hop.fill(0);
	mark.fill(0);
	mark[0] = 1;
	std::vector< long int> checkNode;
	std::vector<long int> noncheckNode;
	find( mark, 0, noncheckNode, checkNode);

	/// calculate the shortest hop matrix
	while( noncheckNode.size() > 0)
	{
		long int checkNodeInd = -1;
		long int noncheckNodeInd = -1;
		for( long int i = 0; i < checkNode.size(); i++)
		{
			for( long int j = 0; j < noncheckNode.size(); j++)
			{
				if( adj_matrix(checkNode[i], noncheckNode[j]) != 0)
				{
					checkNodeInd = checkNode[i];
					noncheckNodeInd =  noncheckNode[j];
					break;
				}
			}
			if( checkNodeInd != -1 && noncheckNodeInd != -1)
			{
				break;
			}
		}

		for( long int i = 0; i < checkNode.size(); i++)
		{
			if( indMapFromClusIndToInd.size() <= 0)
			{
				shortest_hop( checkNode[i], noncheckNodeInd) =  shortest_hop( checkNode[i], checkNodeInd) + 1;
				shortest_hop( noncheckNodeInd, checkNode[i]) =  shortest_hop( checkNode[i], checkNodeInd) + 1;
			}
			else
			{
				int size = ( this->indMapFromClusIndToInd[noncheckNodeInd].size() + this->indMapFromClusIndToInd[checkNodeInd].size()) / 2;
				shortest_hop( checkNode[i], noncheckNodeInd) =  shortest_hop( checkNode[i], checkNodeInd) + size;
				shortest_hop( noncheckNodeInd, checkNode[i]) =  shortest_hop( checkNode[i], checkNodeInd) + size;
			}
		}

		mark[ noncheckNodeInd] = 1;

		checkNode.clear();
		noncheckNode.clear();
		find( mark, 0, noncheckNode, checkNode);
	}

	//QString hopStr = this->fileName + "shortest_hop.txt";
	//ofstream ofs(hopStr.toStdString().c_str());
	//ofs<< shortest_hop<<endl;
	//ofs.close();

	/// find the root and chains of the tree
	long int maxhop = shortest_hop.max_value();
	unsigned int maxId = shortest_hop.arg_max();
	unsigned int coln = maxId / shortest_hop.rows();
	unsigned int rown = maxId  - coln * shortest_hop.rows();

	backbones.clear();
	chainList.clear();  // store the chains, no repeat

	std::queue<long int> tmpbackbones;   // for calculating all the sidechains
	std::vector<long int> debugbackbones;  // for debugging
	std::map< long int, long int> tmpChain;  // for ordering 
	vnl_vector< int> tag( shortest_hop.rows());     // whether the node has been included in the chain
	tag.fill( 0);

	std::cout<<"build chains"<<endl;
	//QString chainStr = this->fileName + "chains.txt";
	//ofstream ofChains( chainStr.toStdString().c_str());
	/// find the backbone
	for( long int i = 0; i < shortest_hop.cols(); i++)
	{
		if( shortest_hop( coln, i) + shortest_hop( rown, i) == maxhop)
		{
			tmpChain.insert( std::pair< long int, long int>( shortest_hop( coln, i), i));
		}
	}

	std::map< long int, long int>::iterator iter;
	for( iter = tmpChain.begin(); iter != tmpChain.end(); iter++)
	{
		backbones.push_back((*iter).second);
		tmpbackbones.push((*iter).second);
		debugbackbones.push_back( (*iter).second);
		tag[ (*iter).second] = 1;
	}

	/// update the progression path
	progressionPath = backbones;   
	progressionStartID = backbones[0];
	progressionEndID = backbones[ backbones.size() - 1];

	/// find the branches' backbones
	std::vector< long int> branchnodes;
	std::vector< long int> chains;
	while( !tmpbackbones.empty())
	{
		long int ind = tmpbackbones.front(); 
		for( long int i = 0; i < adj_matrix.cols(); i++)
		{
			if( adj_matrix( ind, i) != 0 && tag[i] == 0)    // ind's  neighbours that haven't been checked
			{
				branchnodes.clear();
				branchnodes.push_back( ind);
				for( long int j = 0; j < shortest_hop.cols(); j++)
				{
					if( shortest_hop( ind, j) > shortest_hop( i, j))  // find neighbour i's branch nodes including i
					{
						if( tag[j] == 0)
						{
							branchnodes.push_back( j);
						}
						else
						{
							//ofChains<< "already searched branchnode: "<< ind<<"\t"<< i <<"\t"<< j<<endl;
							//for( long int st = 0; st < debugbackbones.size(); st++)
							//{
							//	ofChains<< debugbackbones[st] + 1<<endl;
							//}
							//ofChains<< tag<<endl;
						}
					}
				}

				if( branchnodes.size() > 1)
				{
					getBackBones( shortest_hop, branchnodes, chains);
					chainList.push_back( std::pair< long int, std::vector<long int> >(ind, chains));
					for( long int k = 0; k < chains.size(); k++)
					{
						tmpbackbones.push( chains[k]);
						debugbackbones.push_back( chains[k]);
						tag[ chains[k]] = 1;
					}
				}
			}
		}
		tmpbackbones.pop();
	}

	//ofChains << "backbones:" <<endl;
	//for( long int i = 0; i < backbones.size(); i++)
	//{
	//	ofChains << backbones[i]<<"\t";
	//}
	//ofChains <<endl;
	//ofChains << "branch chains:"<<endl;
	//std::vector< std::pair< long int, std::vector<long int> > >::iterator chainIter;
	//for( chainIter = chainList.begin(); chainIter != chainList.end(); chainIter++)
	//{
	//	std::pair< long int, std::vector< long int> >tmp = *chainIter;
	//	ofChains << tmp.first;
	//	std::vector< long int> branchlist = tmp.second;
	//	for( long int i = 0; i < branchlist.size(); i++)
	//	{
	//		ofChains << "\t"<< branchlist[i];
	//	}
	//	ofChains <<endl;
	//}

	SortChainList( shortest_hop, backbones, chainList);   // the order to draw the subbones
	//for( chainIter = chainList.begin(); chainIter != chainList.end(); chainIter++)
	//{
	//	std::pair< long int, std::vector< long int> >tmp = *chainIter;
	//	ofChains << tmp.first;
	//	std::vector< long int> branchlist = tmp.second;
	//	for( long int i = 0; i < branchlist.size(); i++)
	//	{
	//		ofChains << "\t"<< branchlist[i];
	//	}
	//	ofChains <<endl;
	//}
	//ofChains.close();

	/// calculate the coordinates of the nodes
	std::cout<<"calculate nodes position"<<endl;
	//QString corStr = this->fileName + "coordinates.txt";
	//ofstream ofCoordinate( corStr.toStdString().c_str());
	//ofCoordinate.precision(4);

	vnl_matrix< double> nodePos( 2, adj_matrix.cols());
	vnl_vector< int> nodePosAssigned( adj_matrix.cols());
	nodePos.fill(0);
	nodePosAssigned.fill(0);

	/// calculate backbone node position
	vnl_vector< double> backboneAngle( backbones.size());
	for( long int i = 0; i < backboneAngle.size(); i++)
	{
		backboneAngle[i] = i + 1;
	}
	backboneAngle = backboneAngle -  backboneAngle.mean();
	backboneAngle = backboneAngle / backboneAngle.max_value() * pi / ANGLE_STEP * 25;
	
	double powx = pow( sin(backboneAngle[0]) - sin(backboneAngle[1]), 2);
	double powy = pow( cos(backboneAngle[1]) - cos(backboneAngle[0]), 2);
	double norm = sqrt( powx + powy);

	//if( backbones.size() > 500)
	//{
	//	long int size = backbones.size();
	//	for( long int i = 0; i < backbones.size(); i++)
	//	{
	//		nodePos(0, backbones[i]) = sin( backboneAngle[i]) / norm * 500 / size;
	//		nodePos(1, backbones[i]) = -cos( backboneAngle[i]) / norm * 500 / size;
	//		nodePosAssigned[ backbones[i]] = 1;
	//	}
	//}
	//else
	//{
		for( long int i = 0; i < backbones.size(); i++)
		{
			nodePos(0, backbones[i]) = sin( backboneAngle[i]) / norm;
			nodePos(1, backbones[i]) = -cos( backboneAngle[i]) / norm;
			nodePosAssigned[ backbones[i]] = 1;
		}
	//}

	//if( backbones.size() > 500)
	//{
	//	long int size = backbones.size();
	//	for( long int i = 0; i < backbones.size(); i++)
	//	{
	//		nodePos(0, backbones[i]) = backboneAngle[i] * 500 / size;
	//		nodePos(1, backbones[i]) = 0;
	//		nodePosAssigned[ backbones[i]] = 1;
	//	}
	//}
	//else
	//{
	//	for( long int i = 0; i < backbones.size(); i++)
	//	{
	//		nodePos(0, backbones[i]) = backboneAngle[i];
	//		nodePos(1, backbones[i]) = 0;
	//		nodePosAssigned[ backbones[i]] = 1;
	//	}
	//}

	/// cacluate the branch backbone nodes position
	//std::ofstream ofs("force.txt");
	for( long int i = 0; i < chainList.size(); i++)
	{
		if( i % 100 == 0)
		{
			std::cout<<i<<endl;
		}
		std::pair< long int, std::vector<long int> > branch = chainList[i];
		long int attachNode = branch.first;
		
		std::vector< long int> branchNode = branch.second;
		for( long int j = 0; j < branchNode.size(); j++)
		{
			long int newNode = branchNode[j];

			double bestR = 0.3;
			double bestTheta = 0;

			double rbeg = 0.3;
			double rend = 0.9;
			double rInter = 0.04;
			int count = 0;

			vnl_vector< double> minForce( (int)((rend - rbeg) / rInter + 1));
			vnl_vector< double> mintheta( (int)((rend - rbeg) / rInter + 1));
			minForce.fill(1e9);
			mintheta.fill(0);
	
			for( double r = rbeg; r <= rend; r = r + rInter, count++)
			{	
				bool bfirst = true;
				double minVar = 0;

				for( double theta = 0; theta <= 2 * pi; theta += 2 * pi / 90)
				{
					Point newNodePoint;
					vnl_matrix<double> repel_mat;
					GetElementsIndexInMatrix( shortest_hop, newNode, MAX_HOP, nodePos, repel_mat, nodePosAssigned);

					newNodePoint.x = nodePos( 0, attachNode) + r * cos( theta);
					newNodePoint.y = nodePos( 1, attachNode) + r * sin( theta);

					for( long int k = 0; k < repel_mat.cols(); k++)
					{
						repel_mat(0, k) = newNodePoint.x - repel_mat( 0, k);
						repel_mat(1, k) = newNodePoint.y - repel_mat( 1, k);
					}

					vnl_vector< double> repel_tmp_xvector(repel_mat.cols());
					vnl_vector< double> repel_tmp_yvector(repel_mat.cols());
					for( long int k = 0; k < repel_mat.cols(); k++)
					{
						double force = sqrt( pow(repel_mat(0, k),2) + pow(repel_mat(1, k),2));
						force = pow( force, 5);
						repel_tmp_xvector[ k] = repel_mat(0, k) / force;
						repel_tmp_yvector[ k] = repel_mat(1, k) / force;
					}

					vnl_vector<double> repel_force(2);
					repel_force[ 0] = repel_tmp_xvector.sum();
					repel_force[ 1] = repel_tmp_yvector.sum();

					double cosalfa = (repel_force[0] * cos( theta) + repel_force[1] * sin( theta)) / repel_force.two_norm();
					double sinalfa = sqrt( 1 - pow( cosalfa, 2));

					//std::cout<< cosalfa<<std::endl;
		
					if( bfirst)
					{
						if( repel_force.two_norm() * cosalfa >= 0)
						{
							minForce[ count] = repel_force.two_norm() * cosalfa;
							mintheta[ count] = theta;
							minVar = repel_force.two_norm() * sinalfa;
							bfirst = false;
						}
					}
					else
					{
						if( repel_force.two_norm() * cosalfa >= 0 && repel_force.two_norm() * sinalfa < minVar )
						{
							minForce[ count] = repel_force.two_norm() * cosalfa;
							mintheta[ count] = theta;
							minVar = repel_force.two_norm() * sinalfa;
						}
					}
				}
			}
			//ofs << minForce <<std::endl;
			//ofs << mintheta <<std::endl<<std::endl;

			int min = minForce.arg_min();
			bestR = rbeg + rInter * min;
			bestTheta = mintheta[ min];

			nodePos(0, newNode) = nodePos(0, attachNode) + bestR * cos( bestTheta);
			nodePos(1, newNode) = nodePos(1, attachNode) + bestR * sin( bestTheta);
			nodePosAssigned[ newNode] = 1;
			attachNode = newNode;
		}
	}
	//ofs.close();

	double medianX = Median( nodePos.get_row(0));
	double medianY = Median( nodePos.get_row(1));
	double medianXY[2] = {medianX, medianY};

	for( int i = 0; i < 2; i++)
	{
		vnl_vector<double> tmpNodePos = nodePos.get_row(i) - medianXY[i];
		vnl_vector<double> tmp = tmpNodePos;
		for( long int j = 0; j < tmpNodePos.size(); j++)
		{
			tmp[j] = abs( tmp[j]);
		}
		
		if( tmp.max_value() > 1e-6)
		{
			tmpNodePos = tmpNodePos / tmp.max_value() * 50;
		}
		nodePos.set_row(i, tmpNodePos);
	}

	//ofCoordinate << setiosflags(ios::fixed)<< nodePos.transpose()<<endl;
	//ofCoordinate.close();

	for( long int i = 0; i < nodePos.cols(); i++)
	{
		Point pt( nodePos(0, i), nodePos(1, i));
		pointList.push_back( pt);
	}
}

/// branchnodes first element is the chain's attach node
void GraphWindow::getBackBones(vnl_matrix< long int>& shortest_hop, std::vector< long int>& branchnodes, std::vector< long int>& chains)
{
	chains.clear();
	vnl_vector< long int> branchShortestHop( branchnodes.size());

	long int attachNode = branchnodes.front();
	std::map< long int, long int> tmpChain;  // for ordering 

	for( long int i = 0; i < branchShortestHop.size(); i++)
	{
		branchShortestHop[i] = shortest_hop( attachNode, branchnodes[i]);
	}

	long int maxHop = branchShortestHop.max_value();
	long int endNodeIndex = branchShortestHop.arg_max();
	long int endNode = branchnodes[ endNodeIndex];
	for( long int i = 0; i < shortest_hop.cols(); i++)
	{
		if( shortest_hop( attachNode, i) + shortest_hop( endNode, i) == maxHop)
		{
			tmpChain.insert( std::pair< long int, long int>( shortest_hop( attachNode, i), i));
		}
	}

	std::map< long int, long int>::iterator iter;
	for( iter = tmpChain.begin(); iter != tmpChain.end(); iter++)
	{
		if( (*iter).second != attachNode)
		{
			chains.push_back((*iter).second);
		}
	}
}

void GraphWindow::find(vnl_vector<long int>& vec, long int val, std::vector<long int>& equal, std::vector<long int>& nonequal)
{
	for( unsigned int i = 0; i < vec.size(); i++)
	{
		if( vec[i] == val)
		{
			equal.push_back(i);
		}
		else
		{
			nonequal.push_back(i);
		}
	}
}

void GraphWindow::GetElementsIndexInMatrix(vnl_matrix<long int>& mat, long int rownum, long int max, vnl_matrix<double>& oldmat, vnl_matrix<double>& newmat, vnl_vector< int>& tag)
{
	std::vector< long int> index;
	for( long int i = 0; i < mat.cols(); i++)
	{
		if( tag[i] == 1)
		{
			index.push_back( i);
		}
	}

	newmat.set_size( oldmat.rows(), index.size());
	for( long int i = 0; i < index.size(); i++)
	{
		vnl_vector< double> tmpcol = oldmat.get_column( index[i]);
		newmat.set_column( i, tmpcol);
	}
}

double GraphWindow::Median( vnl_vector<double> vec)
{
	vnl_vector<double> vect = vec;
	vnl_vector<double> tmp( vec.size());
	double max = vect.max_value() + 1;
	double med;
	for( long int i = 0; i < vect.size(); i++)
	{
		tmp[i] = vect.min_value();
		long int ind = vect.arg_min();
		vect[ind] = max;
	}

	long int size = tmp.size();
	if( size % 2 == 1)
	{
		med = tmp[ size / 2];
	}
	else
	{
		med = 1.0 / 2.0 * ( tmp[ size / 2] + tmp[ size / 2 - 1]);
	}
	return med;
}

void GraphWindow::SortChainList( vnl_matrix<long int>& shortest_hop, std::vector<long int>& backbones, 
				   std::vector< std::pair<long int, std::vector<long int> > >& chainList)
{
	std::vector< std::pair<long int, std::vector<long int> > > tmpchainList;
	std::multimap< long int, long int> sortMap;
	long int startNode = backbones[0];
	long int endNode = backbones[ backbones.size() - 1];
	for( long int i = 0; i < chainList.size(); i++)
	{
		std::pair<long int, std::vector<long int> > chainPair = chainList[i];
		if( IsExist( backbones, chainPair.first))
		{
			long int abHop = abs( shortest_hop(chainPair.first, startNode) - shortest_hop( chainPair.first, endNode));
			sortMap.insert( std::pair< long int, long int>(abHop, i));
		}
		else
		{
			std::multimap< long int, long int>::iterator iter;
			for( iter = sortMap.begin(); iter != sortMap.end(); iter++)
			{
				std::pair<long int, std::vector<long int> > pair = chainList[ (*iter).second];
				tmpchainList.push_back( pair);
			}
			for( long int k = i; k < chainList.size(); k++)
			{
				tmpchainList.push_back( chainList[k]);
			}
			chainList = tmpchainList;
			break;
		}
	}
}

bool GraphWindow::IsExist(std::vector<long int>& vec, long int value)
{
	for( long int i = 0; i < vec.size(); i++)
	{
		if( value == vec[i])
		{
			return true;
		}
	}
	return false;
}

void GraphWindow::GetTrendTreeOrder(std::vector<long int> &order)
{
	order.clear();
	for( long int i = 0; i < backbones.size(); i++)
	{
		order.push_back(backbones[i]);
		GetOrder(backbones[i], order);
	}

	//if( this->indMapFromClusIndToInd.size() > 0 )
	//{
	//	for( int i = 0; i < clusterOrder.size(); i++)
	//	{
	//		std::vector<int> clusterNodes = this->indMapFromClusIndToInd[ clusterOrder[i]];
	//		for( int j = 0; j < clusterNodes.size(); j++)
	//		{
	//			order.push_back( clusterNodes[j]);
	//		}
	//	}
	//}
	//else
	//{
	//	order = clusterOrder;
	//}
}

void GraphWindow::GetOrder(long int node, std::vector<long int> &order)
{
	std::vector<long int> vec;
	for( long int i = 0; i < chainList.size(); i++)
	{
		std::pair<long int, std::vector<long int> > pair = chainList[i];
		if( pair.first == node)
		{
			vec = pair.second;
			for( long int j = 0; j < vec.size(); j++)
			{
				order.push_back( vec[j]);
				GetOrder( vec[j], order);
			}
		}
	}
	vec.clear();
}

void GraphWindow::ColorTreeAccordingToFeatures(vnl_vector<double> &feature, const char *featureName)
{
	featureColorVector = feature;
	featureColorVector -= feature.mean();
	double var = featureColorVector.two_norm();
	featureColorVector /= var;

	double max = featureColorVector.max_value();
	double min = featureColorVector.min_value();
	featureColorVector = (featureColorVector - min) / ( max - min);
	
	for( int i = 0; i < colorVector.size(); i++)               
	{
		int k = int( featureColorVector[i] * COLOR_MAP_SIZE + 0.5);
		if( k >= COLOR_MAP_SIZE)
		{
			k = COLOR_MAP_SIZE - 1;
		}
		this->lookupTable->SetTableValue(i, COLORMAP[k].r, COLORMAP[k].g, COLORMAP[k].b); 
	}
	lookupTable->Build();

	//this->view->GetRenderer()->RemoveActor2D(cornAnnotation);
	cornAnnotation->SetText(ANNOTATION_CORNER, featureName);
	//this->view->GetRenderer()->AddActor2D(cornAnnotation);
	cornAnnotation->GetTextProperty()->SetFontSize(10);
	cornAnnotation->GetTextProperty()->SetColor(0,0,0);
	this->view->GetRenderer()->Render();
	this->mainQTRenderWidget.GetRenderWindow()->Render();
	this->selection->clear();
}

void GraphWindow::closeEvent(QCloseEvent *event)
{
	mainQTRenderWidget.close();
}
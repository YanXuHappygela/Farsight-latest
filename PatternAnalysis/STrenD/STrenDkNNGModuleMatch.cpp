#include "STrenDkNNGModuleMatch.h"
#include <QGridLayout>
#include <QLCDNumber>
#include <QFileDialog>
#include <QDir>
#include <fstream>
#include <QMessageBox>
//define NDEBUG
#include <assert.h>
#include "clusclus.h"
#include <cstdlib>
#include <ctime>
#include "tsne.h"
#include "vtkContextMouseEvent.h"
#include "vtkContextScene.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkVector.h"

using std::ifstream;
using std::endl;

STrenDkNNGModuleMatch::STrenDkNNGModuleMatch(QWidget *parent) :
    QWidget(parent)
{
	setWindowTitle(QString("STrenD: Subspace Trend Discovery"));
	STrenDModel = NULL;
	selection = NULL;
	selection2 = NULL;
	graph = NULL;
	simHeatmap = NULL;
	histo = NULL;
	originalHeatmap = NULL;
	progressionHeatmap = NULL;
	autoHeatmapWin = NULL;
	manualHeatmapWin = NULL;
	plot = NULL;
	connectedNum = 0;
	bselect = false;
	bconnected = true;
	int frameStyle = QFrame::Sunken | QFrame::Panel;

	rawHeatmapButton = new QPushButton(tr("Original Heatmap"), this);

    featureNumLabel = new QLabel(tr("Feature size:"), this);
    featureNum = new QLabel(this);
    sampleNumLabel = new QLabel(tr("Sample size:"), this);
    sampleNum = new QLabel(this);

    clusterCoherenceLabel = new QLabel(tr("Agglomerative Clustering of Features:"), this);
	QFont font2 = clusterCoherenceLabel->font();
	font2.setBold(true);
	font2.setPointSize(10);
	clusterCoherenceLabel->setFont(font2);

	clusterCoherenceThresLabel = new QLabel(tr("Threshold:"), this);
    clusterCoherenceBox = new QDoubleSpinBox(this);
	clusterCoherenceBox->setValue(0.8);
	clusterCoherenceBox->setRange(0,1); 
	clusterCoherenceBox->setSingleStep(0.1);

    nsLabel = new QLabel(tr("Calculate Neighborhood Similarity (NS) for feature clusters:"), this);
	font2 = nsLabel->font();
	font2.setBold(true);
	font2.setPointSize(10);
	nsLabel->setFont(font2);

	kNearestNeighborLabel = new QLabel(tr("k:"), this);
	kNearestNeighborBox = new QSpinBox(this);
	kNearestNeighborBox->setValue(4);
	kNearestNeighborBox->setMinimum (0);
	kNearestNeighborBox->setSingleStep(1);

	nBinLabel = new QLabel(tr("Bin size:"), this);
	nBinBox = new QSpinBox(this);
	nBinBox->setValue(20);
	nBinBox->setMinimum (2);
	nBinBox->setSingleStep(1);
	nsmButton = new QPushButton(tr("Calculate"), this);

	selectionLabel = new QLabel(tr("Feature Selection:"), this);
	font2 = selectionLabel->font();
	font2.setBold(true);
	font2.setPointSize(10);
	selectionLabel->setFont(font2);

    radioAuto = new QRadioButton(tr("Auto selection"), this);
    radioManual = new QRadioButton(tr("Manual selection"), this);

    autoSelButton = new QPushButton(tr("Select"), this);
	autoSelList = new QLabel(tr("Auto selection:"), this);

	listbox = new QListWidget( this);
    

	continSelectLabel = new QLabel(tr("Continous:"), this);
	continSelectCheck = new QCheckBox(this);
	psdModuleSelectBox = new QLineEdit(this);
	manualSelButton = new QPushButton(tr("Select"), this);

	QLabel *treeVisLabel = new QLabel(tr("Force-directed tree:"), this); 
	font2 = treeVisLabel->font();
	font2.setBold(true);
	font2.setPointSize(10);
	treeVisLabel->setFont(font2);
	treeVisButton = new QPushButton(tr("Visualize"), this);


	QLabel *visualLabel = new QLabel(tr("t-SNE:"), this); 
	font2 = visualLabel->font();
	font2.setBold(true);
	font2.setPointSize(10);
	visualLabel->setFont(font2);

	autoTrendButton = new QPushButton(tr("Visualize"), this);
	perplexityBox = new QDoubleSpinBox(this);
	perplexLabel = new QLabel(tr("Perplexity:"), this); 
	perplexityBox->setValue(4);
	perplexityBox->setMinimum(1); 
	perplexityBox->setSingleStep(0.1);

	thetaLabel = new QLabel(tr("Theta:"), this); 
	thetaBox = new QDoubleSpinBox(this);
	thetaBox->setValue(0);
	thetaBox->setRange(0,1); 
	thetaBox->setSingleStep(0.1);

	dimLabel = new QLabel(tr("Dimension:"), this); 
	dimBox = new QSpinBox(this);
	dimBox->setValue(2);
	dimBox->setMinimum (1);
	dimBox->setSingleStep(1);

	heatmapButton = new QPushButton(tr("MST-ordered Heatmap"), this);
	//testKButton = new QPushButton(tr("Test K"), this);
	//testLButton = new QPushButton(tr("Test L"), this);

	radioAuto->setChecked(true);
	radioManual->setChecked(false);
	nsmButton->setEnabled(true);
	autoSelButton->setEnabled(false);
	manualSelButton->setEnabled(false);
	autoTrendButton->setEnabled(false);
	heatmapButton->setEnabled(false);
	
    QGridLayout *mainLayout = new QGridLayout(this);

    for ( int col = 0; col<= 2; col++)
    {
        mainLayout->setColumnMinimumWidth(col,100);
        mainLayout->setColumnStretch(col, 1);
    }

     for ( int row = 0; row <= 24; row++)
    {
        mainLayout->setRowMinimumHeight(row,20);
        mainLayout->setRowStretch(row, 1);
    }

	mainLayout->addWidget(rawHeatmapButton, 0, 2);

    mainLayout->addWidget(featureNumLabel, 0, 0);
    mainLayout->addWidget(featureNum, 0, 1);

    mainLayout->addWidget(sampleNumLabel, 1, 0);
    mainLayout->addWidget(sampleNum, 1, 1);
	
	QFrame* line = new QFrame();
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	mainLayout->addWidget(line, 2, 0, 1, 3);

	mainLayout->addWidget(clusterCoherenceLabel, 3, 0, 1, 3);
	mainLayout->addWidget(clusterCoherenceThresLabel, 4, 0);
	mainLayout->addWidget(clusterCoherenceBox, 4, 1);

	mainLayout->addWidget(nsLabel, 5, 0, 1, 3);
    mainLayout->addWidget(kNearestNeighborLabel, 6, 0);
    mainLayout->addWidget(kNearestNeighborBox, 6, 1);
	
	mainLayout->addWidget(nBinLabel, 7, 0);
	mainLayout->addWidget(nBinBox, 7, 1);
	mainLayout->addWidget(nsmButton, 7, 2);

	QFrame* line2 = new QFrame();
	line2->setFrameShape(QFrame::HLine);
	line2->setFrameShadow(QFrame::Sunken);
	mainLayout->addWidget(line2, 8, 0, 1, 3);

	mainLayout->addWidget(selectionLabel, 9, 0);
	mainLayout->addWidget(radioAuto, 10, 0);
	mainLayout->addWidget(autoSelList, 11, 0);
	mainLayout->addWidget(autoSelButton, 11, 2);
	mainLayout->addWidget(listbox, 12, 0, 2, 2);

	mainLayout->addWidget(radioManual, 14, 0);
	mainLayout->addWidget(manualSelButton, 14, 2);
	mainLayout->addWidget(continSelectLabel, 15, 0);
	mainLayout->addWidget(continSelectCheck, 15, 1);
	mainLayout->addWidget(psdModuleSelectBox, 16, 0, 1, 2);

	
	QFrame* line4 = new QFrame();
	line4->setFrameShape(QFrame::HLine);
	line4->setFrameShadow(QFrame::Sunken);
	mainLayout->addWidget(line4, 17, 0, 1, 3);
	mainLayout->addWidget(treeVisLabel, 18, 0, 1, 2);
	mainLayout->addWidget( treeVisButton, 18, 2);

	mainLayout->addWidget(visualLabel, 19, 0, 1, 2);
	mainLayout->addWidget( perplexLabel, 20, 0);
	mainLayout->addWidget( perplexityBox, 20, 1);
	mainLayout->addWidget( thetaLabel, 21, 0);
	mainLayout->addWidget( thetaBox, 21, 1);
	mainLayout->addWidget( dimLabel, 22, 0);
	mainLayout->addWidget( dimBox, 22, 1);
	mainLayout->addWidget( autoTrendButton, 22, 2);

	QFrame* line5 = new QFrame();
	line5->setFrameShape(QFrame::HLine);
	line5->setFrameShadow(QFrame::Sunken);
	mainLayout->addWidget(line5, 23, 0, 1, 3);

	//mainLayout->addWidget(testKButton, 23, 0);
	//mainLayout->addWidget(testLButton, 23, 1);
	mainLayout->addWidget(heatmapButton, 24, 2);

	mainLayout->setSizeConstraint( QLayout::SetFixedSize);
    setLayout(mainLayout);

	connect( rawHeatmapButton, SIGNAL(clicked()), this, SLOT(showOriginalHeatmap()));
	connect( nsmButton,SIGNAL(clicked()), this, SLOT(generateNSM()));
	connect( radioAuto, SIGNAL(clicked()), this, SLOT(autoSelectState()));
	connect( radioManual, SIGNAL(clicked()), this, SLOT(manualSelectState()));
	connect( autoSelButton, SIGNAL(clicked()), this, SLOT(autoSelection()));
	connect( autoTrendButton, SIGNAL(clicked()), this, SLOT(autoClick()));
	connect( heatmapButton, SIGNAL(clicked()), this, SLOT(showTrendHeatmap()));
	connect( treeVisButton, SIGNAL(clicked()), this, SLOT(showMSTGraph()));
	//connect( testKButton, SIGNAL(clicked()), this, SLOT(TestKTrend()));
	//connect( testLButton, SIGNAL(clicked()), this, SLOT(TestLTrend()));

	connect( manualSelButton, SIGNAL(clicked()), this, SLOT(showNSMForManualSelection()));

	STrenDModel = new STrenDAnalysisModel();
}

STrenDkNNGModuleMatch::~STrenDkNNGModuleMatch()
{
	delete(STrenDModel);
	delete(selection);
	delete(selection2);
}

void STrenDkNNGModuleMatch::setModels(vtkSmartPointer<vtkTable> table, ObjectSelection * sels, ObjectSelection * sels2)
{
	if(table == NULL)
	{
		return;
	}
	else
	{
		data = table;
		STrenDModel->ParseTraceFile( data);
		this->featureNum->setText( QString::number(this->STrenDModel->GetFeatureNum()));
		this->sampleNum->setText( QString::number(this->STrenDModel->GetSampleNum()));
	}

	if( sels == NULL)
	{
		selection = new ObjectSelection();
	}
	else
	{
		selection = sels;
	}
	//connect( this->selection, SIGNAL( changed()), this, SLOT( UpdateVisualization()));

	if( sels2 == NULL)
	{
		selection2 = new ObjectSelection();
	}
	else
	{
		selection2 = sels2;
	}

	if(this->simHeatmap)
	{
		delete this->simHeatmap;
	}
	this->simHeatmap = new OrderedHeatmap( this);
	connect( simHeatmap, SIGNAL( SelChanged()), this, SLOT( updateSelMod()));

	if(this->graph)
	{
		delete this->graph;
	}
	this->graph = new GraphWindow( this);

}

void STrenDkNNGModuleMatch::showOriginalHeatmap()
{
	vtkSmartPointer<vtkTable> tableAfterCellCluster = STrenDModel->GetDataTableAfterCellCluster();
	if( this->originalHeatmap)
	{
		delete this->originalHeatmap;
		this->originalHeatmap = NULL;
	}

	this->originalHeatmap = new TrendHeatmapWindow(tr("Original TrendHeatmapWindow"), this);
	std::vector< int> sampleOrder;
	for( int i = 0; i < tableAfterCellCluster->GetNumberOfRows(); i++)
	{
		sampleOrder.push_back(i);
	}
	std::vector< int> selOrder;
	for( int i = 0; i < tableAfterCellCluster->GetNumberOfColumns() - 1; i++)
	{
		selOrder.push_back(i);
	}
	std::vector< int> unselOrder;

	std::map< int, int> indexMap;
	STrenDModel->GetClusterMapping(indexMap);

	this->originalHeatmap->setModelsforSPD( tableAfterCellCluster, selection, sampleOrder, selOrder, unselOrder, &indexMap);
	this->originalHeatmap->showGraphforSPD( selOrder.size(), unselOrder.size(), true);
}

void STrenDkNNGModuleMatch::showHeatmapAfterFeatureClustering()
{
	vtkSmartPointer<vtkTable> tableAfterCellCluster = STrenDModel->GetDataTableAfterCellCluster();
	if( this->originalHeatmap)
	{
		delete this->originalHeatmap;
		this->originalHeatmap = NULL;
	}

	this->originalHeatmap = new TrendHeatmapWindow(tr("Original TrendHeatmapWindow"),this);
	std::vector< int> sampleOrder;
	for( int i = 0; i < tableAfterCellCluster->GetNumberOfRows(); i++)
	{
		sampleOrder.push_back(i);
	}
	std::vector< int> selOrder;
	std::vector< unsigned int> inputSelOrder;
	for( unsigned int i = 0; i < tableAfterCellCluster->GetNumberOfColumns(); i++)
	{
		inputSelOrder.push_back(i);
	}
	GetFeatureOrder(inputSelOrder, selOrder, unselOrder);

	this->originalHeatmap->setModelsforSPD( tableAfterCellCluster, selection, sampleOrder, selOrder, unselOrder);
	this->originalHeatmap->showGraphforSPD( selOrder.size(), unselOrder.size(), true);
}

void STrenDkNNGModuleMatch::clusterFunction()
{
	if ( this->STrenDModel->GetFeatureNum() <= 0 && this->STrenDModel->GetSampleNum() <= 0)
	{
		QMessageBox mes;
		mes.setText("You haven't loaded the data file!");
		mes.exec();
	}

	std::string clusterCor = this->clusterCoherenceBox->text().toStdString();
	
	try
	{
		this->STrenDModel->ClusterAgglomerate( atof(clusterCor.c_str()), 2);
		//showHeatmapAfterFeatureClustering();
	}
	catch(...)
	{
		std::cout<< "Clustering exception, please try again!"<<endl;
	}
}

void STrenDkNNGModuleMatch::generateNSM()
{
	clusterFunction();
	std::string kNeighbor = this->kNearestNeighborBox->text().toStdString();
	std::string nBin = this->nBinBox->text().toStdString();
	this->STrenDModel->ModuleCorrelationMatrixMatch(atoi(kNeighbor.c_str()), atoi(nBin.c_str()));
	bselect = true;
	if( radioAuto->isChecked())
	{
		autoSelectState();
	}
	else
	{
		manualSelectState();
	}
}

void STrenDkNNGModuleMatch::autoSelectState()
{
	if(bselect)
	{
		radioManual->setChecked(false);
		autoSelButton->setEnabled(true);
		autoTrendButton->setEnabled(true);
		manualSelButton->setEnabled(false);
		continSelectCheck->setEnabled(false);
		psdModuleSelectBox->setEnabled(false);
	}
}

void STrenDkNNGModuleMatch::manualSelectState()
{
	if(bselect)
	{
		radioAuto->setChecked(false);
		autoSelButton->setEnabled(false);
		autoTrendButton->setEnabled(false);
		manualSelButton->setEnabled(true);
		continSelectCheck->setEnabled(true);
		psdModuleSelectBox->setEnabled(true);
	}
}

void STrenDkNNGModuleMatch::autoSelection()
{
	double threshold = this->STrenDModel->GetAutoSimilarityThreshold();
	std::cout<< "Auto Threshold: "<< threshold<<std::endl;

	std::vector<std::vector<unsigned int> > modID;
	STrenDModel->GetSelectedFeaturesModulesByConnectedComponent(threshold, modID);
	
	listbox->clear();
	size_t maxIndex = 0;
	size_t maxSize = 0;
	size_t secMaxSize = 0;
	size_t secondMax = 0;
	for( size_t k = 0; k < modID.size(); k++)
	{
		if( modID[k].size() > maxSize)
		{
			secondMax = maxIndex;
			secMaxSize = maxSize;
			maxIndex = k;
			maxSize = modID[k].size();
		}
		else if( modID[k].size() > secMaxSize)
		{
			secondMax = k;
			secMaxSize = modID[k].size();
		}	

		QString str;
		for( size_t i = 0; i < modID[k].size(); i++)
		{
			str += QString::number(modID[k][i])+",";
		}
		listbox->addItem(str);
	}
	//std::cout<< maxIndex << "\t"<< secondMax<<std::endl;
	//if( maxIndex != secondMax)
	//{
	//	QString mergedItem = listbox->item(maxIndex)->text() + listbox->item(secondMax)->text();
	//	listbox->addItem(mergedItem);
	//}

	std::cout<< "Current Index: "<< maxIndex<<std::endl;
	listbox->setCurrentRow(maxIndex);
	autoTrendButton->setEnabled(true);
}

void STrenDkNNGModuleMatch::showNSMForManualSelection()
{
	clusclus *clus1 = new clusclus();
	clusclus *clus2 = new clusclus();

	vnl_matrix<double> iterMat;
	this->STrenDModel->EMDMatrixIteration(iterMat);

	vnl_vector<double> diagnalVec;
	this->STrenDModel->GetBiClusData(iterMat, clus1, &diagnalVec);
	optimalleaforder.set_size(clus1->num_samples);
	clus2->optimalleaforder = new int[clus1->num_samples];
	clus2->num_samples = clus1->num_samples;
	for( int i = 0; i < clus1->num_samples; i++)
	{
		optimalleaforder[i] = clus1->optimalleaforder[i];
		clus2->optimalleaforder[i] = clus1->optimalleaforder[i];
	}

	//vtkSmartPointer<vtkTable> tableAfterCellCluster = STrenDModel->GetDataTableAfterCellCluster();
	this->simHeatmap->setModels();
	this->simHeatmap->setDataForSimilarMatrixHeatmap(clus1->features, clus1->optimalleaforder, clus2->optimalleaforder, clus1->num_samples, clus2->num_samples);	
	this->simHeatmap->creatDataForSimilarMatrixHeatmap(diagnalVec.data_block());
	this->simHeatmap->showSimilarMatrixGraph();

	delete clus1;
	delete clus2;
}

void STrenDkNNGModuleMatch::autoClick()
{
	std::cout<< "View auto trend."<<std::endl;
	viewTrendAuto(true);
}

void STrenDkNNGModuleMatch::showMSTGraph()
{
	UpdateConnectedNum();  // update connected component
	std::cout<< connectedNum<<std::endl;

	if(connectedNum <= 1e-9)
	{
		return;
	}

	//if( this->originalHeatmap)
	//{
	//	delete this->originalHeatmap;
	//	this->originalHeatmap = NULL;
	//}

	std::string selectModulesID = "";

	if( radioAuto->isChecked())
	{
		selectModulesID = listbox->currentItem()->text().toStdString();
	}
	else
	{
		selectModulesID = psdModuleSelectBox->text().toStdString();
	}

	std::vector< unsigned int> selModuleID;
	std::vector< int> clusterSize;
	selFeatureID.clear();
	selOrder.clear();
	unselOrder.clear();

	split( selectModulesID, ',', selModuleID);
	std::cout<< "Selected Module Size:" << selModuleID.size() <<std::endl;
	STrenDModel->GetFeatureIdbyModId(selModuleID, selFeatureID);
	std::cout<< "Selected Features:" << selFeatureID.size() <<std::endl;
	STrenDModel->SaveSelectedFeatureNames("AutoSelFeatures.txt", selFeatureID);

	//for( size_t i = 0; i < selModuleID.size(); i++)
	//{
	//	std::cout<< selModuleID[i]<< "\t";
	//}
	//std::cout<<std::endl;

	// For validation:
	vnl_vector<int> validationVec;
	STrenDModel->GetValidationVec(validationVec);
	if( validationVec.max_value() > 0)
	{
		vnl_matrix<double> clusAverageMat;
		STrenDModel->GetDataMatrix(clusAverageMat);
		std::vector<int> clusterNum(1);
		clusterNum[0] = clusAverageMat.rows();
		vtkSmartPointer<vtkTable> treeTable = STrenDModel->GenerateMST( clusAverageMat, selFeatureID, clusterNum);

		std::vector<std::string> headers;
		STrenDModel->GetTableHeaders( headers);

		vnl_matrix<double> betweenDis;
		vnl_vector<double> accVec;
		vnl_vector<double> aggDegree;

		GraphWindow::GetTreeNodeBetweenDistance(treeTable, headers[0], headers[1], headers[2], betweenDis);
		double aggDegreeValue = 0;
		double averConnectionAccuracy = STrenDModel->GetConnectionAccuracy(treeTable, betweenDis, accVec, aggDegree, aggDegreeValue, 1, 0);
		std::cout<< "ConnectionAccuracy: "<< averConnectionAccuracy<<std::endl;
		std::cout<< "ClusteringAccuracy: "<< aggDegreeValue<<std::endl;
	}
	// end valdiation
	
	vtkSmartPointer<vtkTable> tableAfterFeatureSelection = STrenDModel->GetNormalizedTableAfterFeatureSelection(selFeatureID);
	ftk::SaveTable("data_selected_vis.txt",tableAfterFeatureSelection);

	heatmapButton->setEnabled(true);

	// visualize with trees
	if(this->graph)
	{
		delete this->graph;
	}
	this->graph = new GraphWindow( this);

	vnl_matrix<double> clusAverageMat;
	STrenDModel->GetDataMatrix(clusAverageMat);
	std::vector<int> clusterNum(1);
	clusterNum[0] = clusAverageMat.rows();
	std::cout<< clusterNum[0]<<std::endl;

	//std::vector<double> colorVec2(68);
	//for(size_t i = 0; i < 8; i++)
	//{
	//	colorVec2[i] = 0;
	//}
	//for(size_t i = 8; i < 26; i++)
	//{
	//	colorVec2[i] = 0.3;
	//}
	//for(size_t i = 26; i < 52; i++)
	//{
	//	colorVec2[i] = 0.6;
	//}
	//for(size_t i = 52; i < 68; i++)
	//{
	//	colorVec2[i] = 0.9;
	//}

	vtkSmartPointer<vtkTable> treeTable = STrenDModel->GenerateMST( clusAverageMat, selFeatureID, clusterNum);
	std::cout<< treeTable->GetNumberOfRows()<<"\t"<<treeTable->GetNumberOfColumns()<<std::endl;

	std::vector<std::string> headers;
	std::vector<long int> TreeOrder;
	STrenDModel->GetTableHeaders( headers);
	this->graph->setModels(data, selection);
	this->graph->SetTreeTable( treeTable, headers[0], headers[1], headers[2]/*, &colorVec2*/);
	try
	{
		this->graph->ShowGraphWindow();
	}
	catch(...)
	{
		std::cout<< "Graph window error!"<<endl;
	}
}

void STrenDkNNGModuleMatch::viewTrendAuto(bool bAuto)
{
	UpdateConnectedNum();  // update connected component
	std::cout<< connectedNum<<std::endl;

	if(connectedNum <= 1e-9)
	{
		return;
	}

	//if( this->originalHeatmap)
	//{
	//	delete this->originalHeatmap;
	//	this->originalHeatmap = NULL;
	//}

	std::string selectModulesID = "";

	if( bAuto)
	{
		selectModulesID = listbox->currentItem()->text().toStdString();
		
	}
	else
	{
		selectModulesID = psdModuleSelectBox->text().toStdString();
	}

	std::vector< unsigned int> selModuleID;
	std::vector< int> clusterSize;
	selFeatureID.clear();
	selOrder.clear();
	unselOrder.clear();

	split( selectModulesID, ',', selModuleID);
	std::cout<< "Selected Module Size:" << selModuleID.size() <<std::endl;
	STrenDModel->GetFeatureIdbyModId(selModuleID, selFeatureID);
	std::cout<< "Selected Features:" << selFeatureID.size() <<std::endl;
	STrenDModel->SaveSelectedFeatureNames("AutoSelFeatures.txt", selFeatureID);

	//for( size_t i = 0; i < selModuleID.size(); i++)
	//{
	//	std::cout<< selModuleID[i]<< "\t";
	//}
	//std::cout<<std::endl;

	// For validation:
	vnl_vector<int> validationVec;
	STrenDModel->GetValidationVec(validationVec);
	if( validationVec.max_value() > 0)
	{
		vnl_matrix<double> clusAverageMat;
		STrenDModel->GetDataMatrix(clusAverageMat);
		std::vector<int> clusterNum(1);
		clusterNum[0] = clusAverageMat.rows();
		vtkSmartPointer<vtkTable> treeTable = STrenDModel->GenerateMST( clusAverageMat, selFeatureID, clusterNum);

		std::vector<std::string> headers;
		STrenDModel->GetTableHeaders( headers);

		vnl_matrix<double> betweenDis;
		vnl_vector<double> accVec;
		vnl_vector<double> aggDegree;

		GraphWindow::GetTreeNodeBetweenDistance(treeTable, headers[0], headers[1], headers[2], betweenDis);
		double aggDegreeValue = 0;
		double averConnectionAccuracy = STrenDModel->GetConnectionAccuracy(treeTable, betweenDis, accVec, aggDegree, aggDegreeValue, 1, 0);
		std::cout<< "ConnectionAccuracy: "<< averConnectionAccuracy<<std::endl;
		std::cout<< "ClusteringAccuracy: "<< aggDegreeValue<<std::endl;
	}
	// end valdiation
	
	vtkSmartPointer<vtkTable> tableAfterFeatureSelection = STrenDModel->GetNormalizedTableAfterFeatureSelection(selFeatureID);
	ftk::SaveTable("data_selected_vis.txt",tableAfterFeatureSelection);

	heatmapButton->setEnabled(true);

	int N = tableAfterFeatureSelection->GetNumberOfRows();
	int D = tableAfterFeatureSelection->GetNumberOfColumns() - 1; // remove the id column
	double* X = (double*)calloc(N * D, sizeof(double));
	STrenDModel->tableToDoubleArray(tableAfterFeatureSelection, X, N, D);

	int no_dims = dimBox->value();
	double* Y = (double*)calloc(N * no_dims, sizeof(double));

	TSNE::run(X, N, D, Y, no_dims, perplexityBox->value(), thetaBox->value());
	tableAfterDimReduct = vtkSmartPointer<vtkTable>::New();
	STrenDModel->doubleArrayToTable(tableAfterDimReduct, Y, N, no_dims);
	ftk::SaveTable("vis_coordinates.txt",tableAfterDimReduct);

	free(X);
	free(Y);

	if( no_dims == 3)
	{
		tableAfterDimReduct->RemoveColumn(0);  // remove first column of ids
		chart = vtkSmartPointer<vtkChartXYZ>::New();
		//chart->SetAutoRotate(true);
		chart->SetFitToScene(true);
		chart->SetDecorateAxes(false);
		view = vtkSmartPointer<vtkContextView>::New();
		view->GetRenderWindow()->SetWindowName("t-SNE visualization");
		view->GetRenderWindow()->SetSize(400, 300);
		view->GetScene()->AddItem(chart);
		chart->SetDecorateAxes(false);
		chart->SetGeometry(vtkRectf(75.0, 20.0, 250, 260));
		chart->SetDecorateAxes(false);

		plot3d = vtkSmartPointer<vtkPlotPoints3D>::New();
		plot3d->SetInputData(tableAfterDimReduct);
		chart->AddPlot(plot3d);

		view->GetRenderWindow()->SetMultiSamples(0);
		view->GetInteractor()->Initialize();
		view->GetRenderWindow()->Render();

		vtkContextMouseEvent mouseEvent;
		mouseEvent.SetInteractor(view->GetInteractor());
		vtkVector2i pos;
		vtkVector2i lastPos;

		// rotate
		mouseEvent.SetButton(vtkContextMouseEvent::LEFT_BUTTON);
		lastPos.Set(114, 55);
		mouseEvent.SetLastScreenPos(lastPos);
		pos.Set(174, 121);
		mouseEvent.SetScreenPos(pos);
		chart->MouseMoveEvent(mouseEvent);

		// spin
		mouseEvent.SetButton(vtkContextMouseEvent::LEFT_BUTTON);
		mouseEvent.GetInteractor()->SetShiftKey(1);
		lastPos.Set(0, 0);
		mouseEvent.SetLastScreenPos(lastPos);
		pos.Set(10, 10);
		mouseEvent.SetScreenPos(pos);
		chart->MouseMoveEvent(mouseEvent);

		// zoom
		mouseEvent.SetButton(vtkContextMouseEvent::RIGHT_BUTTON);
		mouseEvent.GetInteractor()->SetShiftKey(0);
		lastPos.Set(0, 0);
		mouseEvent.SetLastScreenPos(lastPos);
		pos.Set(0, 10);
		mouseEvent.SetScreenPos(pos);
		chart->MouseMoveEvent(mouseEvent);

		// mouse wheel zoom
		chart->MouseWheelEvent(mouseEvent, -1);
		view->GetRenderWindow()->Render();
		view->GetInteractor()->Start();
	}
	else
	{
		if( plot)
		{
			delete plot;
		}
		plot = new PlotWindow(this);
		plot->setModels(tableAfterDimReduct, selection);
		plot->show();
	}
	
	/*GetFeatureOrder( selFeatureID, selOrder, unselOrder);

	// write graph to gdf file.
	//STrenDModel->WriteGraphToGDF(selFeatureID);

	if(bAuto)
	{
		//STrenDModel->SaveSelectedFeatureNames("AutoSelFeatures.txt", selOrder);
		STrenDModel->SaveNormalizedTableAfterFeatureSelection("AutoSelNormalizedFeatureTable", selOrder);
	}
	else
	{
		STrenDModel->SaveSelectedFeatureNames("ManualSelFeatures.txt", selOrder);
		STrenDModel->SaveNormalizedTableAfterFeatureSelection("ManualSelNormalizedFeatureTable", selOrder);
	}
	//STrenDModel->WriteKNNGConnectionMatrix( "kNNGC.txt", selFeatureID);

	vtkSmartPointer<vtkTable> tableAfterCellCluster = STrenDModel->GetDataTableAfterCellCluster();

	connect(selection, SIGNAL( thresChanged()), this, SLOT( regenerateTrendTree()));
	connect(selection, SIGNAL( ItemDeleted()), this, SLOT( ReRunSPDAnlysis()));
	//connect(HeatmapWin, SIGNAL(columnToColorChanged(int)), this, SLOT( ReColorTrendTree(int)));

	std::map< int, int> indexMap;
	STrenDModel->GetClusterMapping(indexMap);

	vnl_matrix<double> subTreeDistance;
	STrenDModel->GetComponentMinDistance(selFeatureID, connectedComponent, connectedNum, subTreeDistance);

	if( bAuto)
	{
		if( this->autoHeatmapWin)
		{
			delete this->autoHeatmapWin;
		}
		this->autoHeatmapWin = new TrendHeatmapWindow(tr("Auto Selection Heatmap"), this);
		this->autoHeatmapWin->setModelsforSPD( tableAfterCellCluster, selection, selOrder, unselOrder, &indexMap,\
										&connectedComponent, connectedNum, &subTreeDistance);
		this->autoHeatmapWin->showGraphforSPD( selOrder.size(), unselOrder.size());
	}
	else
	{
		if( this->manualHeatmapWin)
		{
			delete this->manualHeatmapWin;
		}
		this->manualHeatmapWin = new TrendHeatmapWindow(tr("Manual Selection Heatmap"), this);
		this->manualHeatmapWin->setModelsforSPD( tableAfterCellCluster, selection, selOrder, unselOrder, &indexMap,\
										&connectedComponent, connectedNum, &subTreeDistance);
		this->manualHeatmapWin->showGraphforSPD( selOrder.size(), unselOrder.size());
	}*/
}

//void STrenDkNNGModuleMatch::UpdateVisualization()
//{
//	if(tableAfterDimReduct.GetPointer() && plot3d.GetPointer())
//	{
//		std::set<long int> IDsnew = this->selection->getSelections();
//		std::set<int> selIndex;
//		STrenDModel->ConvertVertexToClusIndex(IDsnew, selIndex);
//		vtkSmartPointer<vtkIdTypeArray> selectIDs = vtkSmartPointer<vtkIdTypeArray>::New();
//		for( std::set<int>::iterator iter = selIndex.begin(); iter != selIndex.end(); iter++)
//		{
//			selectIDs->InsertNextValue(int(*iter));
//		}
//
//		theme->SetPointLookupTable(lookupTable);
//		view->ApplyViewTheme();
//
//		plot3d->SetSelection(selectIDs);
//		view->GetRenderWindow()->Render();
//	}
//}

void STrenDkNNGModuleMatch::split(std::string& s, char delim, std::vector< unsigned int>& indexVec)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim,last);
	std::vector< std::string > stringVec;
	while( index!=std::string::npos)
	{
		stringVec.push_back(s.substr(last,index-last));
		last = index+1;
		index=s.find_first_of(delim,last);
	}
	if( index-last>0)
	{
		stringVec.push_back(s.substr(last,index-last));
	}

	for( int i = 0; i < stringVec.size(); i++)
	{
		if( stringVec[i].length() > 0)
		{
			unsigned int index = atoi( stringVec[i].c_str());
			indexVec.push_back( index);
		}
	}
} 

void STrenDkNNGModuleMatch::GetFeatureOrder(std::vector< unsigned int> &selID, std::vector<int> &selIdOrder, std::vector<int> &unselIdOrder)
{
	STrenDModel->HierachicalClustering();
	std::vector< Tree> FeatureTreeData = STrenDModel->PublicTreeData;
	double **ftreedata = new double*[FeatureTreeData.size()];
	selIdOrder.clear();
	unselIdOrder.clear();

	for(int i = 0; i < FeatureTreeData.size(); i++)
	{
		ftreedata[i] = new double[4];
		ftreedata[i][0] = FeatureTreeData[i].first;
		ftreedata[i][1] = FeatureTreeData[i].second;
		ftreedata[i][2] = (1 - FeatureTreeData[i].cor + 0.01) * 100;
		ftreedata[i][3] = FeatureTreeData[i].parent;
	}

	clusclus *cc2 = new clusclus();
	cc2->Initialize(ftreedata, FeatureTreeData.size() + 1);
	cc2->GetOptimalLeafOrderD();

	for( int i = 0; i < cc2->num_samples; i++)
	{
		if( IsExist(selID, (unsigned int)cc2->optimalleaforder[i]))
		{
			selIdOrder.push_back( cc2->optimalleaforder[i]);
		}
		else
		{
			unselIdOrder.push_back( cc2->optimalleaforder[i]);
		}
	}

	//ofstream ofs("FeatureOrder.txt");
	//ofs<< "feature optimal order:"<<endl;
	//for( int i = 0; i < cc2->num_samples; i++)
	//{
	//	ofs<< cc2->optimalleaforder[i]<<"\t";
	//}
	//ofs<<endl;
	//ofs<< "Selected features optimal order:"<<endl;
	//for( int i = 0; i < selIdOrder.size(); i++)
	//{
	//	ofs<< selIdOrder[i]<<"\t";
	//}
	//ofs<<endl;
	//ofs<< "UnSelected features optimal order:"<<endl;
	//for( int i = 0; i < unselIdOrder.size(); i++)
	//{
	//	ofs<< unselIdOrder[i]<<"\t";
	//}
	//ofs<<endl;
	//ofs.close();

	for( int i = 0; i < FeatureTreeData.size(); i++)
	{
		delete ftreedata[i];
	}
	delete ftreedata;
	delete cc2;
}

bool STrenDkNNGModuleMatch::IsExist(std::vector< unsigned int> vec, unsigned int value)
{
	for( int i = 0; i < vec.size(); i++)
	{
		if( value == vec[i])
		{
			return true;
		}
	}
	return false;
}

void STrenDkNNGModuleMatch::regenerateTrendTree()
{
	heatmapButton->setEnabled(true);
	if( selection && (this->autoHeatmapWin || this->manualHeatmapWin))
	{

		std::cout<< "rerender progression view"<<endl;
		selection->clear();
		std::vector< std::vector< long int> > sampleIndex;
		selection->GetSampleIndex( sampleIndex);
        //std::vector< std::vector< long int> > clusIndex;
        //selection->GetClusterIndex( clusIndex);

		double homogeneity = STrenDModel->GetANOVA(sampleIndex, selFeatureID);
		std::cout<< "Homogeneity evaluation: "<< homogeneity<<std::endl;

		vnl_matrix<double> clusAverageMat;
		std::vector< double> colorVec;
		std::vector< double> percentVec;
        STrenDModel->GetSingleLinkageClusterAverage(sampleIndex, clusAverageMat);

		std::vector<int> clusterNum(1);
		clusterNum[0] = clusAverageMat.rows();

		vtkSmartPointer<vtkTable> newtable = STrenDModel->GenerateMST( clusAverageMat, selFeatureID, clusterNum);
        //vtkSmartPointer<vtkTable> newtable = STrenDModel->GenerateSubGraph( clusAverageMat, clusIndex, selFeatureID, clusterNum);

		/** graph window set models */
		std::vector<int> index;
		STrenDModel->GetSingleLinkageClusterMapping(sampleIndex, index);
		vtkSmartPointer<vtkTable> dataTable = vtkSmartPointer<vtkTable>::New();
		STrenDModel->GetCombinedDataTable(dataTable);
		this->graph->setModels(dataTable, selection, &index);

		std::vector<std::string> headers;
		STrenDModel->GetTableHeaders( headers);

		this->graph->SetTreeTable( newtable, headers[0], headers[1], headers[2]);
		//this->graph->SetGraphTableToPassThrough( newtable, sampleIndex.size(), headers[0], headers[1], headers[2], &colorVec, &percentVec);
		try
		{
			this->graph->ShowGraphWindow();
		}
		catch(...)
		{
			std::cout<< "Graph window error!"<<endl;
		}
	}
}

void STrenDkNNGModuleMatch::ReColorTrendTree(int nfeature)
{
	if( this->graph)
	{
		std::vector< std::vector< long int> > clusIndex;
		vnl_vector<double> featureValue;
		std::string featureName;
		selection->GetClusterIndex( clusIndex);
		STrenDModel->GetClusterFeatureValue(clusIndex, nfeature, featureValue, featureName);
		this->graph->ColorTreeAccordingToFeatures(featureValue, featureName.c_str());
	}
}

void STrenDkNNGModuleMatch::updateSelMod()   
{
	int r1 = 0;
	int r2 = 0;
	int c1 = 0;
	int c2 = 0;
	int size = optimalleaforder.size();

	this->simHeatmap->GetSelRowCol(r1, c1, r2, c2);
	//this->simHeatmap->SetSelRowCol(r1, size - 1 - r1, r2, size - 1 - r2);   // make the selection block symetric

	int num = abs(r1 - r2) + 1;
	int max = r1 > r2 ? r1 : r2;

	if( num <= size)   
	{
		if(!continSelectCheck->isChecked())
		{
			selMod.clear();
		}
		for( int i = 0; i < num; i++)
		{
			int index = size - 1 - max + i;
			if( index >= 0 && index < size)
			{
				selMod.insert(optimalleaforder[index]);
			}
		}
		QString str;
		std::set<unsigned int>::iterator iter = selMod.begin();
		for(; iter != selMod.end(); iter++)
		{
			str += QString::number(*iter)+",";
		}
		psdModuleSelectBox->setText(str);
	}
}

void STrenDkNNGModuleMatch::UpdateConnectedNum()
{
	std::string selectModulesID = this->listbox->currentItem()->text().toStdString();
	std::vector< unsigned int> selModuleID;
	split( selectModulesID, ',', selModuleID);
	STrenDModel->GetFeatureIdbyModId(selModuleID, selFeatureID);

	connectedNum = this->STrenDModel->GetConnectedComponent(selFeatureID, connectedComponent);
}

void STrenDkNNGModuleMatch::GetTrendTreeOrder(std::vector<long int> &order)
{
	this->graph->GetTrendTreeOrder(order);
}

void STrenDkNNGModuleMatch::showTrendHeatmap()
{
	//ofstream ofs("SPDHeatmapOptimalOrder.txt");
	if( this->progressionHeatmap)
	{
		delete this->progressionHeatmap;
	}
	this->progressionHeatmap = new TrendHeatmapWindow(tr("MST-ordered Heatmap"),this);
	
	std::vector<long int> TreeOrder;
	this->graph->GetTrendTreeOrder(TreeOrder);   // order of the cluster 
	if( TreeOrder.size() <=0)
	{          
		std::cout<< "Tree hasn't been built yet"<<endl;
		vnl_matrix<double> clusAverageMat;
		STrenDModel->GetDataMatrix(clusAverageMat);
		std::vector<int> clusterNum(1);
		clusterNum[0] = clusAverageMat.rows();
		std::cout<< clusterNum[0]<<std::endl;

		vtkSmartPointer<vtkTable> treeTable = STrenDModel->GenerateMST( clusAverageMat, selFeatureID, clusterNum);
		std::cout<< treeTable->GetNumberOfRows()<<"\t"<<treeTable->GetNumberOfColumns()<<std::endl;

		std::vector<std::string> headers;
		STrenDModel->GetTableHeaders( headers);
		this->graph->SetTreeTable( treeTable, headers[0], headers[1], headers[2]);
		this->graph->GetTrendTreeOrder(TreeOrder);   // order of the cluster 
	}

	std::vector< std::vector< long int> > sampleIndex;
	selection->GetSampleIndex( sampleIndex);
	std::vector< std::vector< long int> > clusIndex;
	selection->GetClusterIndex( clusIndex);
	std::vector< int> clusterOrder;
	STrenDModel->GetClusterOrder(clusIndex, TreeOrder, clusterOrder);
	//STrenDModel->GetValidationOrder(clusterOrder);

	//vtkSmartPointer<vtkTable> tableForAverModulePlot = STrenDModel->GetAverModuleTable(clusIndex, TreeOrder, selOrder, unselOrder);
	//if( plot)
	//{
	//	delete plot;
	//}
	//plot = new PlotWindow(this);
	//plot->setModels(tableForAverModulePlot, selection);
	//plot->show();

	// progression heatmap
	vtkSmartPointer<vtkTable> tableAfterCellCluster = STrenDModel->GetDataTableAfterCellCluster();

	std::map< int, int> indexMap;
	STrenDModel->GetClusterMapping(indexMap);
	GetFeatureOrder( selFeatureID, selOrder, unselOrder);
	this->progressionHeatmap->setModelsforSPD( tableAfterCellCluster, selection, clusterOrder, selOrder, unselOrder, &indexMap);
	this->progressionHeatmap->showGraphforSPD( selOrder.size(), unselOrder.size(), true);
}

void STrenDkNNGModuleMatch::closeEvent(QCloseEvent *event)
{
	closeSubWindows();
	event->accept();
}

void STrenDkNNGModuleMatch::closeSubWindows()
{
	if(originalHeatmap)
	{
		originalHeatmap->close();
		delete originalHeatmap;
		originalHeatmap = NULL;
	}
	if(graph)
	{
		graph->close();
		delete graph;
		graph = NULL;
	}
	if(simHeatmap)
	{
		simHeatmap->close();
		delete simHeatmap;
		simHeatmap = NULL;
	}
	if(progressionHeatmap)
	{
		progressionHeatmap->close();
		delete progressionHeatmap;
		progressionHeatmap = NULL;
	}
	if(autoHeatmapWin)
	{
		autoHeatmapWin->close();
		delete autoHeatmapWin;
		autoHeatmapWin = NULL;
	}
	if(manualHeatmapWin)
	{
		manualHeatmapWin->close();
		delete manualHeatmapWin;
		manualHeatmapWin = NULL;
	}
	if(plot)
	{
		plot->close();
		delete plot;
		plot = NULL;
	}
	if(histo)
	{
		histo->close();
		delete histo;
		histo = NULL;
	}
	if(STrenDModel)
	{
		delete STrenDModel;
		STrenDModel = NULL;
	}
}

void STrenDkNNGModuleMatch::ReRunSPDAnlysis()
{
	closeSubWindows();
	std::set<long int> selItems = this->selection->getSelections();
	disconnect(selection, SIGNAL( thresChanged()), this, SLOT( regenerateTrendTree()));
	disconnect(selection, SIGNAL( ItemDeleted()), this, SLOT( ReRunSPDAnlysis()));
	this->selection->clear();
	vtkSmartPointer<vtkTable> table = GetSubTableExcludeItems(data, selItems);
	setModels( table, this->selection);
}

vtkSmartPointer<vtkTable> STrenDkNNGModuleMatch::GetSubTableExcludeItems(vtkSmartPointer<vtkTable> table, std::set<long int> &IDs)
{
	excludedIds = IDs;
	vtkSmartPointer<vtkTable> newTable = vtkSmartPointer<vtkTable>::New();
	for( vtkIdType i = 0; i < table->GetNumberOfColumns(); i++)
	{
		vtkSmartPointer<vtkVariantArray> column = vtkSmartPointer<vtkVariantArray>::New();
		column->SetName( table->GetColumnName(i));
		newTable->AddColumn(column);
    }

	for( vtkIdType i = 0; i < table->GetNumberOfRows(); i++)
	{
		long int id = table->GetValue(i, 0).ToLong();
		if( IDs.find(id) == IDs.end())
		{
			newTable->InsertNextRow( table->GetRow(i));
		}
	}
	return newTable;
}

vtkSmartPointer<vtkTable> STrenDkNNGModuleMatch::NormalizeTable(vtkSmartPointer<vtkTable> table)
{
	STrenDModel->ParseTraceFile( table, false);
	vtkSmartPointer<vtkTable> normalTable = STrenDModel->GetDataTableAfterCellCluster();
	return normalTable;
}

void STrenDkNNGModuleMatch::TestKTrend()
{
	QString str = featureNum->text() + "_" + sampleNum->text() + "_" + clusterCoherenceBox->text() + "_" + kNearestNeighborBox->text();
	std::string logName = "TestLog_"+ str.toStdString() + ".txt";
	std::string accName = "Accuracy_" + str.toStdString() + ".txt";

	std::ofstream ofs(logName.c_str(), std::ofstream::out);
	std::ofstream acc(accName.c_str(), std::ofstream::out);
	acc<< "Threshold"<< "\t"<< /*"Index"<< "\t"<<*/"Feature_num"<<"\t"<<"Coonection_Accuracy"<<"\t"<<"Aggregation_Degree"<<std::endl;
	//srand((unsigned)time(0));
	for(double selThreshold = 0.9; selThreshold >= 0.3; selThreshold = selThreshold - 0.01)
	{
		std::cout<< selThreshold<<"\t";
		std::vector<std::vector<unsigned int> > modID;
		STrenDModel->GetSelectedFeaturesModulesByConnectedComponent(selThreshold, modID);
		
		//for(int count = 0; count < 22; count++)
		//{
		//	int randomN = rand() % 1291;
		//	std::cout<< randomN<<"\t";
		//	tmp.push_back(randomN);
		//}
		//std::cout<<std::endl;
		//modID.push_back(tmp);

		//std::vector<unsigned int> allMods;
		//for( size_t k = 0; k < modID.size(); k++)
		//{
		//	for( size_t i = 0; i < modID[k].size(); i++)
		//	{
		//		allMods.push_back(modID[k][i]);
		//	}
		//}
		//modID.clear();
		//modID.push_back(allMods);
	
		if( modID.size() > 0)
		{
			size_t maxSize = 0;
			size_t maxIndex = 0;
			for( size_t k = 0; k < modID.size(); k++)
			{
				if(modID[k].size() > maxSize)
				{
					maxSize = modID[k].size();
					maxIndex = k;
				}
			}
			std::vector<unsigned int > tmp = modID[maxIndex];

			ofs<< selThreshold<< /*"\t"<< k<< "\t"<< modID[k].size() <<*/std::endl;
			for(unsigned int i = 0; i < tmp.size(); i++)
			{
				ofs<< tmp[i]<<"\t";
			}
			ofs<<std::endl;

			std::vector< unsigned int> selFeatureID;
			STrenDModel->GetFeatureIdbyModId(tmp, selFeatureID);

			vnl_matrix<double> clusAverageMat;
			STrenDModel->GetDataMatrix(clusAverageMat);
			std::vector<int> clusterNum(1);
			clusterNum[0] = clusAverageMat.rows();

			vtkSmartPointer<vtkTable> treeTable = STrenDModel->GenerateMST( clusAverageMat, selFeatureID, clusterNum);

			std::vector<std::string> headers;
			STrenDModel->GetTableHeaders( headers);

			vnl_matrix<double> betweenDis;
			vnl_vector<double> accVec;
			vnl_vector<double> aggDegree;

			GraphWindow::GetTreeNodeBetweenDistance(treeTable, headers[0], headers[1], headers[2], betweenDis);
			double aggDegreeValue = 0;
			double averConnectionAccuracy = STrenDModel->GetConnectionAccuracy(treeTable, betweenDis, accVec, aggDegree, aggDegreeValue, 1, 0);
			std::cout<< averConnectionAccuracy<< "\t"<< aggDegreeValue<<std::endl;

			ofs<< averConnectionAccuracy<<std::endl;
			ofs<< accVec<<std::endl;
			ofs<< aggDegree<<std::endl<<std::endl;
			acc<< selThreshold<< "\t"<< /*k<< "\t"<<*/selFeatureID.size()<<"\t"<<averConnectionAccuracy<<"\t"<<aggDegreeValue<<std::endl;
		}
		else
		{
			acc<< selThreshold<< "\t"<< /*k<< "\t"<<*/0<<"\t"<<0<<"\t"<<0<<std::endl;
		}
	}
	ofs.close();
	acc.close();
}

void STrenDkNNGModuleMatch::TestLTrend()
{
	QString str = featureNum->text() + "_" + sampleNum->text() + "_" + clusterCoherenceBox->text() + "_" + kNearestNeighborBox->text();
	std::string logName = "LevelAcc_"+ str.toStdString() + ".txt";
	std::ofstream acc(logName.c_str(), std::ofstream::out);
	acc<< "Level"<< "\t"<< "Threshold"<< "\t"<< /*"Index"<< "\t"<<*/"Feature_num"<<"\t"<<"Coonection_Accuracy"<<"\t"<<"Aggregation_Degree"<<std::endl;

	for(unsigned int level = 100; level <= 400; level += 10)
	{
		double selThreshold = this->STrenDModel->GetAutoSimilarityThreshold(level);
		std::vector<std::vector<unsigned int> > modID;
		STrenDModel->GetSelectedFeaturesModulesByConnectedComponent(selThreshold, modID);
	
		if( modID.size() > 0)
		{
			size_t maxSize = 0;
			size_t maxIndex = 0;
			for( size_t k = 0; k < modID.size(); k++)
			{
				if(modID[k].size() > maxSize)
				{
					maxSize = modID[k].size();
					maxIndex = k;
				}
			}
			std::vector<unsigned int > tmp = modID[maxIndex];

			std::vector< unsigned int> selFeatureID;
			STrenDModel->GetFeatureIdbyModId(tmp, selFeatureID);

			vnl_matrix<double> clusAverageMat;
			STrenDModel->GetDataMatrix(clusAverageMat);
			std::vector<int> clusterNum(1);
			clusterNum[0] = clusAverageMat.rows();

			vtkSmartPointer<vtkTable> treeTable = STrenDModel->GenerateMST( clusAverageMat, selFeatureID, clusterNum);

			std::vector<std::string> headers;
			STrenDModel->GetTableHeaders( headers);

			vnl_matrix<double> betweenDis;
			vnl_vector<double> accVec;
			vnl_vector<double> aggDegree;

			GraphWindow::GetTreeNodeBetweenDistance(treeTable, headers[0], headers[1], headers[2], betweenDis);
			double aggDegreeValue = 0;
			double averConnectionAccuracy = STrenDModel->GetConnectionAccuracy(treeTable, betweenDis, accVec, aggDegree, aggDegreeValue, 1, 0);
			acc<< level<< "\t" << selThreshold<< "\t"<< /*k<< "\t"<<*/selFeatureID.size()<<"\t"<<averConnectionAccuracy<<"\t"<<aggDegreeValue<<std::endl;
		}
		else
		{
			acc<< level<< "\t" <<selThreshold<< "\t"<< /*k<< "\t"<<*/0<<"\t"<<0<<"\t"<<0<<std::endl;
		}
	}
	acc.close();
}
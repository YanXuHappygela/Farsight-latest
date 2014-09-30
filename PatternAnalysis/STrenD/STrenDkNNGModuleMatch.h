#ifndef SPDKNNGMODULEMATCH_H
#define SPDKNNGMODULEMATCH_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include "STrenDAnalysisModel.h"
#include "ftkGUI/GraphWindow.h"
#include "ftkGUI/HistoWindow.h"
#include "ftkGUI/PlotWindow.h"
#include "ftkGUI/vtk3DView.h"
#include "TrendHeatmapWindow.h"
#include "OrderedHeatmapWindow.h"
#include <QListWidget>
#include "vtkContextView.h"
#include "vtkChartXYZ.h"
#include "vtkPlotPoints3D.h"

class STrenDkNNGModuleMatch : public QWidget
{
    Q_OBJECT

public:
    STrenDkNNGModuleMatch(QWidget *parent = 0);
	void setModels(vtkSmartPointer<vtkTable> table = NULL, ObjectSelection * sels = NULL, ObjectSelection * sels2 = NULL);
    ~STrenDkNNGModuleMatch();
	void GetTrendTreeOrder(std::vector<long int> &order);
	vtkSmartPointer<vtkTable> NormalizeTable(vtkSmartPointer<vtkTable> table);

protected:
	void split(std::string& s, char delim, std::vector< unsigned int>& indexVec);
	void GetFeatureOrder(std::vector< unsigned int> &selID, std::vector< int> &selIdOrder, std::vector< int> &unselIdOrder);
	void showHeatmapAfterFeatureClustering();
	bool IsExist(std::vector< unsigned int> vec, unsigned int value);
	virtual void closeEvent(QCloseEvent *event);
	void closeSubWindows();
	vtkSmartPointer<vtkTable> GetSubTableExcludeItems( vtkSmartPointer<vtkTable> table, std::set<long int> &selItems);
	void viewTrendAuto(bool bAuto = true);

protected slots:
	void showOriginalHeatmap();
    void clusterFunction();
	void generateNSM();
	void autoSelectState();
	void manualSelectState();
	void autoSelection();
	void showNSMForManualSelection();
	void updateSelMod();
	void showTrendHeatmap();
	void regenerateTrendTree();
	void ReRunSPDAnlysis();
	void ReColorTrendTree(int nfeature);
	void UpdateConnectedNum();
	void TestKTrend();
	void TestLTrend();
	void autoClick();
	void showMSTGraph();
	//void UpdateVisualization();

private:
	STrenDAnalysisModel *STrenDModel;

	QPushButton *rawHeatmapButton;
	
    QLabel *featureNumLabel;
    QLabel *featureNum;
    QLabel *sampleNumLabel;
    QLabel *sampleNum;

	QLabel *clusterCoherenceLabel;
    QLabel *clusterCoherenceThresLabel;
    QDoubleSpinBox *clusterCoherenceBox;
	
	QLabel *nsLabel;
    QLabel *kNearestNeighborLabel;
    QSpinBox *kNearestNeighborBox;
	QLabel *nBinLabel;
    QSpinBox *nBinBox;
	QPushButton *nsmButton;

	QRadioButton *radioAuto;
	QRadioButton *radioManual;

	QLabel *autoSelList;
	QLabel *selectionLabel;
    QPushButton *autoSelButton;
	 
	QLabel *continSelectLabel;   
	QCheckBox *continSelectCheck;
	QPushButton *manualSelButton;
	QLineEdit *psdModuleSelectBox;  // select similar modules 
	
	//QPushButton *testKButton;
	//QPushButton *testLButton;
	QPushButton *heatmapButton;  // show progression heatmap  
	
	// visualization
	QLabel *treeVisLabel;   
	QPushButton *treeVisButton;

	QLabel *visualLabel;   
	QPushButton *autoTrendButton;
	QLabel *perplexLabel;
	QDoubleSpinBox *perplexityBox;
	QLabel *thetaLabel;
	QDoubleSpinBox *thetaBox;
	QLabel *dimLabel;
	QSpinBox *dimBox;
	
	//QLabel *connectedGraphLabel;
	//QLineEdit *connectedGraphEdit;
	//QPushButton *updateConnectedNumButton;  // update connected component number
	QListWidget *listbox;

	QString FileName;
	GraphWindow *graph;
	OrderedHeatmap *simHeatmap;
	HistoWindow *histo;
	TrendHeatmapWindow *originalHeatmap;
	TrendHeatmapWindow *progressionHeatmap;
	TrendHeatmapWindow *autoHeatmapWin;
	TrendHeatmapWindow *manualHeatmapWin;
	PlotWindow *plot;
	vtk3DView *plot3D;
	vnl_vector<int> optimalleaforder;

	vtkSmartPointer<vtkTable> data;
	std::set<long int> excludedIds;
	ObjectSelection *selection;  /** selection for threshold and IDs */
	ObjectSelection *selection2; 

	std::vector< unsigned int> selFeatureID;
	std::vector< int> selOrder;
	std::vector< int> unselOrder;
	vtkSmartPointer<vtkTable> tableAfterCellCluster;
	std::map< int, int> indexMap;
	std::vector<int> connectedComponent;
	int connectedNum;
	bool bconnected;
	bool bselect;
	std::set<unsigned int> selMod;

	vtkSmartPointer<vtkContextView> view;
	vtkSmartPointer<vtkChartXYZ> chart;
	vtkSmartPointer<vtkPlotPoints3D> plot3d;
	vtkSmartPointer<vtkTable> tableAfterDimReduct;
};

#endif // SPDKNNGMODULEMATCH_H

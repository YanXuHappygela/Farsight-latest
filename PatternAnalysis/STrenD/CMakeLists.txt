SET( Libraries
    ${VXL_LIBRARIES}
    ${VTK_LIBRARIES}
    ${ITK_LIBRARIES}
    vcl
    vnl
    mbl
    ${QT_LIBRARIES})

QT4_WRAP_CPP( GUI_MOC_SRCS STrenDkNNGModuleMatch.h TrendHeatmapWindow.h HeatmapWindowForNewSelection.h OrderedHeatmapWindow.h)

SET(STrenD_SRCS
	STrenDAnalysisModel.h
	STrenDAnalysisModel.cpp
	transportSimplex.h
	OrderedHeatmapWindow.h
	OrderedHeatmapWindow.cpp
	HeatmapWindowForNewSelection.h
	HeatmapWindowForNewSelection.cpp
	TrendHeatmapWindow.h
	TrendHeatmapWindow.cpp
	STrenDkNNGModuleMatch.h
	STrenDkNNGModuleMatch.cpp
    tsne.h
    tsne.cpp
    sptree.h
    sptree.cpp
    vptree.h
	clusclus.cpp
	clusclus.h)
	
ADD_LIBRARY( STrenD ${STrenD_SRCS} ${GUI_MOC_SRCS})
TARGET_LINK_LIBRARIES(STrenD ${Libraries} ftkGUI ftkCommon)
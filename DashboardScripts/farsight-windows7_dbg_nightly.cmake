set( CTEST_SITE "farsight-win_7_64" )
set( CTEST_BUILD_NAME "vs9-64-dbg-nightly" )
SET(CTEST_SOURCE_NAME "src/farsight")
SET(CTEST_BINARY_NAME "bin/farsight-nightly")
SET(CTEST_DASHBOARD_ROOT "C:/dashboard")
SET(CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/${CTEST_SOURCE_NAME}")
SET(CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/${CTEST_BINARY_NAME}")
set(farsight_data_directory "C:/dashboard/src/farsight-data")
SET (CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE)

SET(CTEST_COMMAND
  "\"c:/Program Files (x86)/CMake 2.8/bin/ctest.exe\" -V -VV -D Nightly -A \"${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}\""
  )

SET(CTEST_CMAKE_COMMAND
  "\"c:/Program Files (x86)/CMake 2.8/bin/cmake.exe\""
  )

SET( CMPLR_PATH "C:/Program Files (x86)/Microsoft Visual Studio 9.0/Common7/IDE/devenv.com" )
SET( FPROJ_PATH "C:/dashboard/bin/farsight-nightly/Farsight.sln" )
SET( PRLL_STR_CXX "/DWIN32 /D_WINDOWS /W3 /Zm1000 /EHsc /GR /MP16" )
SET( PRLL_STR_C "/DWIN32 /D_WINDOWS /W3 /Zm1000 /MP16" )

SET(CTEST_INITIAL_CACHE "
SITE:STRING=farsight-win_7_64
BUILDNAME:STRING=vs9-64-dbg-nightly
CMAKE_GENERATOR:INTERNAL=Visual Studio 9 2008 Win64
MAKECOMMAND:STRING=${CMPLR_PATH} ${FPROJ_PATH} /build Debug /project ALL_BUILD
CMAKE_CXX_FLAGS:STRING=${PRLL_STR_CXX}
CMAKE_C_FLAGS:STRING=${PRLL_STR_C}
BUILD_Batch_Distance2Device:BOOL=ON
BUILD_CLUSCLUS:BOOL=ON
BUILD_DENDROGRAM:BOOL=ON
BUILD_FTKNUCLEARSEGMENTATIONNIC:BOOL=ON
BUILD_FTKVOTING:BOOL=ON
BUILD_HARALICK:BOOL=ON
BUILD_IMAGE_MANAGER:BOOL=ON
BUILD_IMONTAGE:BOOL=ON
BUILD_Microglia:BOOL=ON
BUILD_MicrogliaRegionTracer:BOOL=ON
BUILD_OPENMP:BOOL=ON
BUILD_POWEROF2TILING:BOOL=ON
BUILD_REGISTRATION:BOOL=ON
BUILD_ROLLINGBALLFILTER:BOOL=ON
BUILD_RPITrace3D:BOOL=ON
BUILD_SAMPLE_EDITOR:BOOL=ON
BUILD_SHARED_LIBS:BOOL=OFF
BUILD_SPD:BOOL=ON
BUILD_TESTING:BOOL=ON
BUILD_TracingSystem:BOOL=ON
BUILD_VESSEL_TRACING:BOOL=ON
BUILD_XML_GENERATOR:BOOL=ON
BUILD_ZERNIKE:BOOL=ON
BUILD_image_dicer:BOOL=ON
FARSIGHT_DATA_ROOT:PATH=C:/dashboard/src/farsight-data
ITK_DIR:PATH=C:/dashboard/bin/itk-nightly
VTK_DIR:PATH=C:/dashboard/bin/vtk-nightly
VXL_DIR:PATH=C:/dashboard/bin/vxl-nightly
Boost_INCLUDE_DIR:PATH=C:/dashboard/src/boost
QT_QMAKE_EXECUTABLE:FILEPATH=C:/Qt/4.8.1/bin/qmake.exe
")

SET(CTEST_CVS_COMMAND "C:/Program Files/TortoiseSVN/bin/TortoiseProc.exe")
file( WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "${CTEST_INITIAL_CACHE}" )
ctest_start( Nightly )
ctest_update( SOURCE "${CTEST_SOURCE_DIRECTORY}" )
ctest_configure( SOURCE "${CTEST_SOURCE_DIRECTORY}" )
ctest_build( BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_test( BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_submit()

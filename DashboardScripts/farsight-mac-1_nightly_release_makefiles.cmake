SET(CTEST_SOURCE_NAME farsight-trunk)
SET(CTEST_BINARY_NAME farsight-nightly-release-makefiles)
SET(CTEST_DASHBOARD_ROOT "/projects/Dashboards")
SET(CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/${CTEST_SOURCE_NAME}")
SET(CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/${CTEST_BINARY_NAME}")

SET (CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE)

SET(CTEST_COMMAND
  "/usr/local/bin/ctest -V -VV -D Nightly -A ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}"
  )

SET(CTEST_CMAKE_COMMAND
  "/usr/local/bin/cmake"
  )

SET(CTEST_INITIAL_CACHE "
SITE:STRING=farsight-mac-1
BUILDNAME:STRING=nightly-release-makefiles
CMAKE_GENERATOR:INTERNAL=Unix Makefiles
MAKECOMMAND:STRING=/usr/bin/make -j9 -i
CMAKE_BUILD_TYPE:STRING=Release
BUILD_SHARED_LIBS:BOOL=OFF
ITK_DIR:PATH=/projects/Dashboards/ITK-3.16.0-static
VTK_DIR:PATH=/projects/Dashboards/VTK-5.4.2-static
CMAKE_OSX_ARCHITECTURES:STRING=i386
QT_QMAKE_EXECUTABLE:FILEPATH=/usr/local/Qt4.6.0-universal/bin/qmake
FARSIGHT_DATA_ROOT:PATH=/projects/Dashboards/farsight-data
")

SET(CTEST_CVS_COMMAND "/usr/bin/svn")
SET(CTEST_EXTRA_UPDATES_1 "/projects/Dashboards/farsight-data" "--non-interactive")

SET( SampleEditor_SRCS
	SampleEditor.cpp
)

SET( SampleEditor_HDERS
	SampleEditor.h
)

# and finally this will run moc:
QT4_WRAP_CPP( SampleEditor_MOC_SRCS ${SampleEditor_HDERS} )


ADD_EXECUTABLE(SampleEditor main.cpp
             ${SampleEditor_SRCS}
             ${SampleEditor_HDERS} 
             ${SampleEditor_MOC_SRCS} 
)

TARGET_LINK_LIBRARIES( SampleEditor ${QT_LIBRARIES} ftkCommon ftkGraphs STrenD ClusClus_lib)

#MAKE A GUI EXECUTABLE, NOT A CONSOLE APPLICATION
#SET_TARGET_PROPERTIES( SampleEditor PROPERTIES WIN32_EXECUTABLE 1)
#OR MAKE A CONSOLE APPLICATION
SET_TARGET_PROPERTIES( SampleEditor PROPERTIES WIN32_EXECUTABLE 0)

INSTALL( TARGETS SampleEditor DESTINATION ${INSTALL_BIN_DIR} )



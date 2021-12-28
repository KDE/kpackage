add_executable(mockhandler main.cpp)
ecm_mark_nongui_executable(mockhandler)
target_link_libraries(mockhandler Qt${QT_MAJOR_VERSION}::Core)

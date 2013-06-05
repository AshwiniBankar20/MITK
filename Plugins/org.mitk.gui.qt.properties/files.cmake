set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_properties_Activator.cpp
  QmitkPropertiesPreferencePage.cpp
  QmitkPropertyItemDelegate.cpp
  QmitkPropertyItem.cpp
  QmitkPropertyItemModel.cpp
  QmitkPropertyItemSortFilterProxyModel.cpp
  QmitkPropertyTreeView.cpp
)

set(UI_FILES
  src/internal/QmitkPropertiesPreferencePage.ui
  src/internal/QmitkPropertyTreeView.ui
)

set(MOC_H_FILES
  src/internal/QmitkPropertiesPreferencePage.h
  src/internal/org_mitk_gui_qt_properties_Activator.h
  src/internal/QmitkPropertyItemDelegate.h
  src/internal/QmitkPropertyItemModel.h
  src/internal/QmitkPropertyItemSortFilterProxyModel.h
  src/internal/QmitkPropertyTreeView.h
)

set(CACHED_RESOURCE_FILES
  resources/icon.png
  plugin.xml
)

set(QRC_FILES
)

set(CPP_FILES
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach()

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach()

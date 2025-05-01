cmake_minimum_required(VERSION 3.16)

project(VRproject VERSION 0.1 LANGUAGES CXX)

find_package(VTK REQUIRED
  COMPONENTS
    CommonCore
    RenderingCore
    RenderingOpenGL2
    IOImage
    IOGeometry
    FiltersSources
    GUISupportQt
    InteractionStyle
    RenderingOpenVR
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package( VTK REQUIRED )
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ModelPart.cpp
    ModelPart.h
    ModelPartList.cpp
    ModelPartList.h
    skyboxutils.h
    skyboxutils.cpp
    Option_Dialog.h
    Option_Dialog.cpp
    Option_Dialog.ui
    Icons.qrc
    VRRenderThread.h
    VRRenderThread.cpp
)

# Executable definition (Qt6-friendly)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(VRproject
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(VRproject SHARED ${PROJECT_SOURCES})
    else()
        add_executable(VRproject ${PROJECT_SOURCES})
    endif()
endif()

# Link all required Qt + VTK modules
target_link_libraries(VRproject PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
   Qt${QT_VERSION_MAJOR}::Core 
  Qt${QT_VERSION_MAJOR}::OpenGLWidgets
  VTK::CommonCore
  VTK::CommonColor
  VTK::RenderingCore
  VTK::RenderingOpenGL2
  VTK::IOImage
  VTK::IOGeometry
  VTK::FiltersSources
  VTK::GUISupportQt
  VTK::InteractionStyle
  VTK::RenderingOpenVR
  VTK::CommonCore
  VTK::CommonColor
)

# macOS/Windows bundle settings
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.VRproject)
endif()

set_target_properties(VRproject PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Installer
include(GNUInstallDirs)
install(TARGETS VRproject
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
  "${OpenVR_INCLUDE_DIR}/../bin/win64/openvr_api.dll"
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT RuntimeLibraries
)

# Finalize for Qt 6
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(VRproject)
endif()

set(CMAKE_AUTOMOC ON)

# Custom copy task
add_custom_target(VRBindings ALL)
add_custom_command(TARGET VRBindings PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/vrbindings
        ${CMAKE_BINARY_DIR}/vrbindings
)
add_dependencies(VRproject VRBindings)

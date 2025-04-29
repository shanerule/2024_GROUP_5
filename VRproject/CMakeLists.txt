cmake_minimum_required(VERSION 3.16)

project(VRProject VERSION 0.1 LANGUAGES CXX)

# CMake Policies
cmake_policy(SET CMP0072 NEW)

# Qt automatic processing
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core OpenGL OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core OpenGL OpenGLWidgets)

# Find VTK packages
find_package(VTK REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    FiltersCore
    FiltersGeneral
    FiltersGeometry
    RenderingCore
    RenderingOpenGL2
    IOImage
    IOGeometry
    InteractionStyle
    FiltersSources
    ViewsQt
)

message(STATUS "Using Qt version: ${Qt${QT_VERSION_MAJOR}_VERSION}")
message(STATUS "Using VTK version: ${VTK_VERSION}")

# Project Source Files
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ModelPart.cpp
    ModelPart.h
    ModelPartList.cpp
    ModelPartList.h
    Option_Dialog.cpp
    Option_Dialog.h
    Option_Dialog.ui
    Icons.qrc
)

# Create Executable
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(VRProject
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(VRProject
        ${PROJECT_SOURCES}
    )
endif()

# Link Libraries
target_link_libraries(VRProject PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets

    VTK::CommonCore
    VTK::CommonDataModel
    VTK::FiltersCore
    VTK::FiltersGeneral
    VTK::FiltersGeometry
    VTK::RenderingCore
    VTK::RenderingOpenGL2
    VTK::IOImage
    VTK::IOGeometry
    VTK::InteractionStyle
    VTK::FiltersSources
    VTK::ViewsQt
)

# Platform Properties
set_target_properties(VRProject PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Install
include(GNUInstallDirs)
install(TARGETS VRProject
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Qt6 Finalization
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(VRProject)
endif()

# VR Bindings Support
add_custom_target(VRBindings)
add_custom_command(TARGET VRBindings PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/vrbindings
    ${CMAKE_BINARY_DIR}/
)

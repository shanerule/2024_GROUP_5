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
    FiltersGeometry
    GUISupportQt
    InteractionStyle
    RenderingOpenVR
    FiltersModeling
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets Concurrent)
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
   Qt${QT_VERSION_MAJOR}::Concurrent
  VTK::CommonCore
  VTK::CommonColor
  VTK::RenderingCore
  VTK::RenderingOpenGL2
  VTK::IOImage
  VTK::IOGeometry
  VTK::FiltersSources
  VTK::FiltersGeometry
  VTK::GUISupportQt
  VTK::InteractionStyle
  VTK::RenderingOpenVR
  VTK::CommonCore
  VTK::CommonColor
  VTK::FiltersModeling
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

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
else()
    set(CPACK_GENERATOR "ZIP")
endif()

# –– Package Identity ––
set(CPACK_GENERATOR "NSIS")
set(CPACK_PACKAGE_NAME "VRproject")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VR APPLICATION")
set(CPACK_PACKAGE_VENDOR "2024_GROUP_5")
set(CPACK_PACKAGE_VERSION_MAJOR "2")

# –– Which exe(s) to shortcut ––
set(CPACK_PACKAGE_EXECUTABLES "VRproject;VRproject")

# –– Installation defaults ––
set(CPACK_PACKAGE_INSTALL_DIRECTORY "VRproject")
set(CPACK_NSIS_EXECUTABLES_DIRECTORY "bin")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "VRproject")

set(CPACK_NSIS_DISPLAY_NAME "VRproject")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_CREATE_UNINSTALLER "ON")
set(CPACK_NSIS_MENU_LINKS "${CPACK_NSIS_EXECUTABLES_DIRECTORY}/VRprojectApp.exe" "VRprojectApp" "Uninstall.exe" "Uninstall VRprojectApp")
set(CPACK_NSIS_MODIFY_PATH ON)

if(WIN32)
    get_filename_component(QT_BIN_DIR "${QT_DIR}/bin" ABSOLUTE)
    set(DEPLOY_DIR "${CMAKE_BINARY_DIR}/deploy")
    file(MAKE_DIRECTORY ${DEPLOY_DIR})
    file(GLOB QT_DLLS "${QT_BIN_DIR}/*.dll")
    file(COPY ${QT_DLLS} DESTINATION ${DEPLOY_DIR})
    file(GLOB QT_PLUGINS "${QT_BIN_DIR}/plugins/platforms/*.dll")
    file(COPY ${QT_PLUGINS} DESTINATION ${DEPLOY_DIR}/platforms)
    set(ENV{QT_QPA_PLATFORM_PLUGIN_PATH} "${DEPLOY_DIR}/platforms")
endif()

include(CPack)

install(TARGETS VRproject RUNTIME DESTINATION .
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/vrbindings/
    DESTINATION ${CMAKE_INSTALL_BINDIR}/vrbindings
    COMPONENT headers
)

add_custom_command(TARGET VRproject POST_BUILD
    COMMAND "${WINDEPLOYQT_EXECUTABLE}"
    --verbose 0
    --no-compiler-runtime
    --no-opengl-sw
    --dir "${CMAKE_BINARY_DIR}/windeployqt"
    "$<TARGET_FILE:VRproject>"
)

install(DIRECTORY "${CMAKE_BINARY_DIR}/windeployqt/" DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install VTK DLLs
install(FILES $<TARGET_RUNTIME_DLLS:VRproject>
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT RuntimeLibraries
)

# Install OpenVR DLLs
install(FILES "${OpenVR_INCLUDE_DIR}/../bin/win64/openvr_api.dll"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT RuntimeLibraries
)

cpack_add_component(libraries)
cpack_add_component(headers)


cmake_minimum_required(VERSION 3.16)
add_compile_options(-g -fsanitize=address)
add_link_options(-fsanitize=address)
project(GPRProc LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)

qt_standard_project_setup()

qt_add_executable(GPRProc
    main.cpp
    mainwindow.cpp
	qcustomplot.cpp
	tabbedworkspacewidget.cpp
	profile.cpp
	profiledocker.cpp
	proceduresdialog.cpp
	processingstepsdialog.cpp
	pugixml.cpp
)

set_target_properties(GPRProc PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build the examples" FORCE)
add_subdirectory(thirdparty/Qt-Advanced-Docking-System)
add_subdirectory(thirdparty/liquid-dsp)
include_directories(thirdparty/liquid-dsp/include/)

target_link_libraries(GPRProc PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
	Qt6::PrintSupport
	qtadvanceddocking-qt6
	fftw3f
	liquid
)


install(TARGETS GPRProc
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET GPRProc
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

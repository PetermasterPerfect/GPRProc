cmake_minimum_required(VERSION 3.25)
add_compile_options(-g -fsanitize=address)
add_link_options(-fsanitize=address)
project(GPRProc LANGUAGES CXX)

if(CMAKE_SYSTEM_NAME EQUAL "Windows")
	include_directories("${BOOST_INCLUDE}")
	include_directories("${FFTW3F_INCLUDE_DIR}")
else()
	set(FFTW3F_LIBRARY "fftw3f")
endif()


find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PrintSupport QuickWidgets Location Positioning)

set(CMAKE_AUTORCC ON)
qt_standard_project_setup()

qt_add_executable(GPRProc
    src/main.cpp
    src/mainwindow.cpp
	src/qcustomplot.cpp
	src/tabbedworkspacewidget.cpp
	src/profile.cpp
	src/profiledocker.cpp
	src/proceduresdialog.cpp
	src/processingstepsdialog.cpp
	src/loadmarksdialog.cpp
	src/pugixml.cpp
	src/qml/qml.qrc
)

set_target_properties(GPRProc PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build the examples" FORCE)
add_subdirectory(thirdparty/Qt-Advanced-Docking-System)
add_subdirectory(thirdparty/liquid-dsp)
add_subdirectory(thirdparty/nmea)
include_directories(thirdparty/liquid-dsp/include/)
include_directories(thirdparty/Qt-Advanced-Docking-System/src)
include_directories(thirdparty/nmea/include)

target_link_libraries(GPRProc PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
	Qt6::PrintSupport
	Qt6::Location Qt6::Positioning
	Qt6::QuickWidgets
	qtadvanceddocking
	"${FFTW3F_LIBRARY}"
	liquid
	nmea
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

if(CMAKE_SYSTEM_NAME EQUAL "Windows")
	install(FILES
		FFTW3F_LIBRARY
		"${CMAKE_BINARY_DIR}/libliquid.dll"
		DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()

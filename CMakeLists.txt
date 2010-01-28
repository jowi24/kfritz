# --- general setup -----------------------------------------------------------
cmake_minimum_required(VERSION 2.6)
project (kfritzbox)

set(CMAKE_VERBOSE_MAKEFILE false)
set(CMAKE_BUILD_TYPE "Debug")


# --- kde includes ------------------------------------------------------------
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

# --- support for libindicate -------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(INDICATEQT indicate-qt>=0.2.2)
macro_log_feature(INDICATEQT_FOUND "indicate-qt" "QT bindings for libindicate" 
                                   "https://launchpad.net/libindicate-qt" 
                                   FALSE 
                                   "0.2.2" 
                                   "Required for indicator support.")
if (INDICATEQT_FOUND)
	include_directories(${INDICATEQT_INCLUDE_DIRS})
	link_directories(${INDICATEQT_LIBRARY_DIRS})
endif (INDICATEQT_FOUND)

                    
# --- include local libs ------------------------------------------------------
subdirs(libtcpclient++)
subdirs(libpthread++)
subdirs(libfritz++)
INCLUDE_DIRECTORIES(libfritz++ libtcpclient++ libpthread++)


# --- compile and link --------------------------------------------------------

set(kfritzbox_SRCS KFritzBox.cpp KFritzBoxWindow.cpp KFonbookModel.cpp 
                   KFritzModel.cpp KCalllistModel.cpp LibFritzInit.cpp Log.cpp 
                   QAdaptTreeView.cpp)
set(CMAKE_CXX_FLAGS_DEBUG "-fexceptions -g -ggdb -O0 -Wall -D_GLIBCXX_DEBUG" ) 

kde4_add_kcfg_files(kfritzbox_SRCS GENERATE_MOC KSettings.kcfgc)
kde4_add_ui_files(kfritzbox_SRCS KSettingsFritzBox.ui)

configure_file(pkg-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/pkg-config.h)
kde4_add_executable(kfritzbox ${kfritzbox_SRCS})
target_link_libraries(kfritzbox ${KDE4_KDEUI_LIBS} ${KDE4_KNOTIFYCONFIG_LIBRARY} 
                                fritz++ tcpclient++ pthread++ 
                                ssl)
if (INDICATEQT_FOUND)
	target_link_libraries(kfritzbox ${INDICATEQT_LIBRARIES})
endif (INDICATEQT_FOUND)


# --- installs ----------------------------------------------------------------                                
install(TARGETS kfritzbox ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kfritzbox.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kfritzbox)
install(FILES kfritzboxui.rc DESTINATION ${DATA_INSTALL_DIR}/kfritzbox)
install(FILES kfritzbox.kcfg DESTINATION ${KCFG_INSTALL_DIR})

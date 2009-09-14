cmake_minimum_required(VERSION 2.6)
project (kfritzbox)
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include_directories(${KDE4_INCLUDES})
set(kfritzbox_SRCS KFritzBox.cpp KFritzBoxWindow.cpp KEventHandler.cpp KFonbookModel.cpp KCalllistModel.cpp Log.cpp)
set(CMAKE_CXX_FLAGS "-fexceptions -g -ggdb -O0 -Wall" ) 

subdirs(libtcpclient++)
subdirs(libpthread++)
subdirs(libfritz++)
INCLUDE_DIRECTORIES(libfritz++ libtcpclient++ libpthread++)

find_package(ssl)

kde4_add_executable(kfritzbox ${kfritzbox_SRCS})
target_link_libraries(kfritzbox ${KDE4_KDEUI_LIBS} fritz++ tcpclient++ pthread++ ssl)
install(TARGETS kfritzbox  ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kfritzbox.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kfritzbox)
install(FILES kfritzboxui.rc DESTINATION ${DATA_INSTALL_DIR}/kfritzbox)


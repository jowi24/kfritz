# --- general setup -----------------------------------------------------------
cmake_minimum_required(VERSION 2.6)
project (KFRITZ)

#set(CMAKE_VERBOSE_MAKEFILE true) # <-- enable for debugging
#set(CMAKE_BUILD_TYPE "Debug")    # <-- enable for debugging 
set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_MODULE_PATH "${KFRITZ_SOURCE_DIR}/CMakeModules")

find_package(PkgConfig REQUIRED)

# --- kde includes ------------------------------------------------------------
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

# --- libgcrypt includes ------------------------------------------------------
include("FindGcryptConfig")
gcrypt_check(GCRYPT REQUIRED gcrypt)

# --- gnu common cpp library --------------------------------------------------
pkg_check_modules(CC++ REQUIRED libccgnu2 libccext2)
include_directories(${CC++_INCLUDE_DIRS})
link_directories(${CC++_LIBRARY_DIRS})

# --- support for libindicate -------------------------------------------------
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
subdirs(libfritz++)
INCLUDE_DIRECTORIES(libfritz++ libtcpclient++)

# --- include icons -----------------------------------------------------------
subdirs(icons)

# --- i18n --------------------------------------------------------------------
find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
if(GETTEXT_MSGFMT_EXECUTABLE)
  set(catalogname kfritz)
  add_custom_target(translations ALL)
  file(GLOB PO_FILES po/*.po)
  foreach(_poFile ${PO_FILES})
    get_filename_component(_poFileName ${_poFile} NAME)
    string(REGEX REPLACE "^${catalogname}_?" "" _langCode ${_poFileName} )
    string(REGEX REPLACE "\\.po$" "" _langCode ${_langCode} )
 
    if( _langCode )
      get_filename_component(_lang ${_poFile} NAME_WE)
      set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)
 
      add_custom_command(TARGET translations
                         COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
                         DEPENDS ${_poFile})
      install(FILES ${_gmoFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/ RENAME ${catalogname}.mo)
    endif( _langCode )
  endforeach(_poFile ${PO_FILES}) 
endif(GETTEXT_MSGFMT_EXECUTABLE)

# --- compile and link --------------------------------------------------------

set(kfritzbox_SRCS DialDialog.cpp KFritz.cpp KFritzProxyModel.cpp KFritzWindow.cpp KFonbookModel.cpp 
                   KSettingsFonbooks.cpp KSettingsFritzBox.cpp KFritzModel.cpp KCalllistModel.cpp 
                   LibFritzInit.cpp Log.cpp LogDialog.cpp QAdaptTreeView.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS} ${GCRYPT_CFLAGS} ${CC++_CFLAGS_OTHER}")
set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0 -Wall -D_GLIBCXX_DEBUG") 

kde4_add_kcfg_files(kfritzbox_SRCS GENERATE_MOC KSettings.kcfgc)
kde4_add_ui_files(kfritzbox_SRCS KSettingsFritzBox.ui KSettingsFonbooks.ui DialDialog.ui)

configure_file(pkg-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/pkg-config.h)
kde4_add_executable(kfritz ${kfritzbox_SRCS})
target_link_libraries(kfritz ${KDE4_KDEUI_LIBS} ${KDE4_KNOTIFYCONFIG_LIBRARY} 
                                fritz++ tcpclient++ ${GCRYPT_LIBRARIES} ${CC++_LDFLAGS})
if (INDICATEQT_FOUND)
	target_link_libraries(kfritz ${INDICATEQT_LIBRARIES})
endif (INDICATEQT_FOUND)


# --- installs ----------------------------------------------------------------                                
install(TARGETS kfritz ${INSTALL_TARGETS_DEFAULT_ARGS})
# TODO: kfritz.notifyrc is installed in /usr/local/share/apps/kfritz/ -> does not work (use /usr/share/kde4/apps/kfritz/ instead)
install(FILES kfritz.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kfritz)
install(FILES kfritzui.rc DESTINATION ${DATA_INSTALL_DIR}/kfritz)
install(FILES kfritz.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kfritz.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})


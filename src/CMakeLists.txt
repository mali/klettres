
########### next target ###############

set(klettres_SRCS 
   klettres.cpp 
   main.cpp 
   klettresview.cpp 
   soundfactory.cpp 
   langutils.cpp
   kltheme.cpp
   timer.cpp )


kde4_add_ui_files(klettres_SRCS
    timerui.ui
    fontsdlg.ui )

kde4_add_kcfg_files(klettres_SRCS prefs.kcfgc )

#kde4_add_app_icon(klettres_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/klettres.png")
#kde4_add_app_icon(klettres_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/ox*-apps-klettres.png")
kde4_add_app_icon(klettres_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-apps-klettres.png")

kde4_add_executable(klettres ${klettres_SRCS})

target_link_libraries(klettres  ${KDE4_KNEWSTUFF3_LIBS} ${KDE4_PHONON_LIBS} ${QT_QTXML_LIBRARY})

install(TARGETS klettres  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS klettres.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES klettresui.rc  DESTINATION  ${DATA_INSTALL_DIR}/klettres )
install( FILES klettres.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES klettres.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}   )

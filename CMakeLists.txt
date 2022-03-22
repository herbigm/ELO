cmake_minimum_required(VERSION 3.5)

project(ELO-NG VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 COMPONENTS Widgets LinguistTools WebEngineWidgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools WebEngineWidgets PrintSupport REQUIRED)
if (NOT ${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
    find_package(Libssh2 CONFIG REQUIRED)
    find_package(unofficial-git2 CONFIG REQUIRED)
    find_package(OpenSSL REQUIRED)
endif()

set(TS_FILES ELO-NG_de_DE.ts)
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/elo.rc")

set(PROJECT_SOURCES
        ${APP_ICON_RESOURCE_WINDOWS}
        main.cpp
        elomainwindow.cpp
        elomainwindow.h
        ELOResources.qrc
        elowebpage.h
        elowebpage.cpp
        elocalculatorwidget.h
        elocalculatorwidget.cpp
        elosettingsdialog.h
        elosettingsdialog.cpp
        elosettings.h
        elosettings.cpp
        elometadatadialog.h
        elometadatadialog.cpp
        elofileview.h
        elofileview.cpp
        elofilemodel.h
        elofilemodel.cpp
        elodocumenthandler.h
        elodocumenthandler.cpp
        elodocument.h
        elodocument.cpp
        elolinkdialog.h
        elolinkdialog.cpp
        eloassociatedfileview.h
        eloassociatedfileview.cpp
        elouser.h
        elouser.cpp
        qblowfish_p.h
        qblowfish.h
        qblowfish.cpp
        elogitprocess.h
        elogitprocess.cpp
        eloreposettingsdialog.h
        eloreposettingsdialog.cpp
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    if(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
        qt_add_executable(ELO-NG
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
        )
    else()
        qt_add_executable(ELO-NG WIN32
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
        )
    endif()
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ELO-NG APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation


else()
    if(ANDROID)
        add_library(ELO-NG SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ELO-NG
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
qt_add_lupdate(ELO-NG TS_FILES ${TS_FILES})
qt_add_lrelease(ELO-NG
    TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE qm_files)
install(FILES ${qm_files} DESTINATION ".")

if(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(ELO-NG PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::WebEngineWidgets Qt${QT_VERSION_MAJOR}::PrintSupport ssh2 git2)
else()
    target_link_libraries(ELO-NG PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::WebEngineWidgets Qt${QT_VERSION_MAJOR}::PrintSupport unofficial::git2::git2 Libssh2::libssh2 OpenSSL::SSL OpenSSL::Crypto)
endif()

set_target_properties(ELO-NG PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ELO-NG)
endif()

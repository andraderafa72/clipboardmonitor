cmake_minimum_required(VERSION 3.16)

project(clipboardmonitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(X11 REQUIRED)

include(GNUInstallDirs)

set(SOURCES
    main.cpp
    src/clipboard-monitor-window.cpp
    src/keyboard-monitor.cpp
    src/clipboard-history-store.cpp
)

set(HEADERS
    src/clipboard-monitor-window.h
    src/keyboard-monitor.h
)

qt_standard_project_setup()
qt_wrap_cpp(MOC_HEADERS ${HEADERS})

set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME} ${SOURCES} ${MOC_HEADERS} resources.qrc)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets X11)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/clipboard-monitor.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/clipboard-monitor.service
    @ONLY)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES dist/icon.svg DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
        RENAME clipboard-monitor.svg)
install(FILES data/clipboard-monitor.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/clipboard-monitor.service
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/user)

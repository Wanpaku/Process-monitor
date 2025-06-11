cmake_minimum_required(VERSION 3.16)

project(Process_monitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools)

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES en uk)

qt_add_executable(${PROJECT_NAME}
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        resources.qrc
        dataloader.h
        dataloader.cpp
        logger.h logger.cpp
        usedmemorywidget.h
        usedmemorywidget.cpp
)

qt_add_translations(${PROJECT_NAME}
    TS_FILES translations/Process_monitor_en_US.ts
             translations/Process_monitor_uk_UA.ts
)

add_dependencies(${PROJECT_NAME} update_translations)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
)

install(TARGETS ${PROJECT_NAME}
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}

)

cmake_minimum_required(VERSION 3.16)

project(GravityChess VERSION 0.1 LANGUAGES CXX)

# 1. 开启自动处理
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. 查找 Qt
find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

# 3. 定义源文件
set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    images.qrc
)

# 4. 创建目标
if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(GravityChess
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        sounds.qrc
    )
else()
    if(ANDROID)
        add_library(GravityChess SHARED ${PROJECT_SOURCES})
    else()
        add_executable(GravityChess ${PROJECT_SOURCES}
            sounds.qrc)
    endif()
endif()

# 5. 链接库 (修改点：统一使用变量，并把 Multimedia 加在这里)
# 删除了重复的链接行，合并为一行
target_link_libraries(GravityChess PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia)

# 6. 设置属性
if(QT_VERSION VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.GravityChess)
endif()
set_target_properties(GravityChess PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 7. 安装
include(GNUInstallDirs)
install(TARGETS GravityChess
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 8. Finalization
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(GravityChess)
endif()
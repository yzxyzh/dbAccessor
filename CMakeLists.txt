cmake_minimum_required(VERSION 2.8)

project(dbAccessor)


set(moc_header mainwindow.h)
set(source main.cpp mainwindow.cpp myMacro.h)
set(ui mainwindow.ui)

find_package(OpenCV REQUIRED)
find_package(ITK REQUIRED)
find_package(QT4 REQUIRED)

INCLUDE(${ITK_USE_FILE})
INCLUDE(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/tixml)

add_definitions(-DTIXML_USE_STL)

file(GLOB tixmlSrc ${PROJECT_SOURCE_DIR}/tixml/*.cpp)

QT4_WRAP_CPP(MOCSrcs ${moc_header})
QT4_WRAP_UI(MOCUI ${ui})

message(${MOCUI})

add_executable(dbAccessor ${source} ${MOCSrcs} ${moc_header} ${MOCUI} ${tixmlSrc})

target_link_libraries(dbAccessor ${ITK_LIBRARIES} ${OpenCV_LIBS} ${QT_LIBRARIES})
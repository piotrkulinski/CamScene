cmake_minimum_required(VERSION 3.16)

project(CamScene VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH C:/msys64/mingw64)

# Znajdź Qt6
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia Network)

# Ścieżka do OpenCV
set(OpenCV_Version 4110)
#set(OpenCV_Version -411)
set(OpenCV_INSTALL_DIR C:/msys64/home/piotr/opencv_install)
set(OpenCV_INSTALL_LIB ${OpenCV_INSTALL_DIR}/x64/mingw)
#set(OpenCV_INSTALL_DIR c:/msys64/mingw64)
#set(OpenCV_INSTALL_LIB c:/msys64/mingw64)

# Include'y OpenCV
include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${OpenCV_INSTALL_DIR}/include)
include_directories(${OpenCV_INSTALL_DIR}/include/opencv4)

# Ścieżki do bibliotek OpenCV
link_directories(${OpenCV_INSTALL_LIB}/lib)
link_directories(${OpenCV_INSTALL_LIB}/bin)

# Biblioteki OpenCV
set(OpenCV_LIBS
    opencv_core${OpenCV_Version}
    opencv_imgproc${OpenCV_Version}
    opencv_highgui${OpenCV_Version}
    opencv_imgcodecs${OpenCV_Version}
    opencv_videoio${OpenCV_Version}
)

# Źródła projektu
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    IFrameAnalyzer.h
    MotionDetector.h
    CarLicensePlateDetector.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CamScene
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        RegionA.h
        polygonitem.h polygonitem.cpp
        Resources.qrc
        cameraview.h cameraview.cpp
        regioncaption.h regioncaption.cpp
        CameraSetup.ui
        IFrameAnalyzer.h
        MotionDetector.h
        CarLicensePlateDetector.h
    )
else()
    if(ANDROID)
        add_library(CamScene SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(CamScene
            ${PROJECT_SOURCES}
        )
    endif()
endif()
# target_link_libraries(twój_target PRIVATE winmm)
# Linkowanie z Qt i OpenCV
target_link_libraries(CamScene PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Network
    winmm
    ${OpenCV_LIBS}  # Dodanie bibliotek OpenCV
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CamScene)
endif()
set_target_properties(CamScene PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Instalacja
include(GNUInstallDirs)
install(TARGETS CamScene
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Finalizacja Qt6
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CamScene)
endif()

# Zdefiniowanie pełnych ścieżek do plików DLL
set(DEPENDENT_DLLSX
    "${OpenCV_INSTALL_LIB}/bin/libopencv_core${OpenCV_Version}.dll"
    "${OpenCV_INSTALL_LIB}/bin/libopencv_imgproc${OpenCV_Version}.dll"
    "${OpenCV_INSTALL_LIB}/bin/libopencv_highgui${OpenCV_Version}.dll"
    "${OpenCV_INSTALL_LIB}/bin/libopencv_imgcodecs${OpenCV_Version}.dll"
    "${OpenCV_INSTALL_LIB}/bin/libopencv_videoio${OpenCV_Version}.dll"
    "${CMAKE_PREFIX_PATH}/bin/Qt6Core.dll"
    "${CMAKE_PREFIX_PATH}/bin/Qt6Widgets.dll"
    "${CMAKE_PREFIX_PATH}/bin/Qt6Gui.dll"

    "${CMAKE_PREFIX_PATH}/bin/libglib-2.0-0.dll"         # Dodano bibliotekę glib
    "${CMAKE_PREFIX_PATH}/bin/libwinpthread-1.dll"
    "${CMAKE_PREFIX_PATH}/bin/libgstreamer-1.0-0.dll"    # Dodano bibliotekę gstreamer
    "${CMAKE_PREFIX_PATH}/bin/libgobject-2.0-0.dll"      # Dodano bibliotekę gobject
    "${CMAKE_PREFIX_PATH}/bin/libtbb12.dll"              # Dodano bibliotekę tbb
    "${CMAKE_PREFIX_PATH}/bin/libgstbase-1.0-0.dll"      # Dodano bibliotekę gstbase
    "${CMAKE_PREFIX_PATH}/bin/libgstapp-1.0-0.dll"       # Dodano bibliotekę gstapp
    "${CMAKE_PREFIX_PATH}/bin/libgstpbutils-1.0-0.dll"   # Dodano bibliotekę gstpbutils
    "${CMAKE_PREFIX_PATH}/bin/libgstvideo-1.0-0.dll"     # Dodano bibliotekę gstvideo
    "${CMAKE_PREFIX_PATH}/bin/libgstrtp-1.0-0.dll"       # Dodano bibliotekę gstrtp
    "${CMAKE_PREFIX_PATH}/bin/libgstaudio-1.0-0.dll"     # Dodano bibliotekę gstaudio
    "${CMAKE_PREFIX_PATH}/bin/libgstriff-1.0-0.dll"      # Dodano bibliotekę gstriff
    "${CMAKE_PREFIX_PATH}/bin/libffi-8.dll"              # Dodano bibliotekę ffi
    "${CMAKE_PREFIX_PATH}/bin/libpcre2-8-0.dll"          # Dodano bibliotekę pcre2
    "${CMAKE_PREFIX_PATH}/bin/libOpenEXR-3_3.dll"        # Dodano bibliotekę OpenEXR
    "${CMAKE_PREFIX_PATH}/bin/libOpenEXRCore-3_3.dll"        # Dodano bibliotekę OpenEXR
    "${CMAKE_PREFIX_PATH}/bin/libgcc_s_seh-1.dll"
    "${CMAKE_PREFIX_PATH}/bin/liborc-0.4-0.dll"
    "${CMAKE_PREFIX_PATH}/bin/libgsttag-1.0-0.dll"
    #"C:/msys64/mingw64/bin/liblmath-3_1.dll"
    #"C:/msys64/mingw64/bin/liblex-3_3.dll"
    #"C:/msys64/mingw64/bin/liblmThread-3_3.dll"
)

# Dodanie polecenia do kopiowania DLL do folderu wynikowego
foreach(dll ${DEPENDENT_DLLSX})
    add_custom_command(TARGET CamScene POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${dll}
        $<TARGET_FILE_DIR:CamScene>  # Kopiowanie do katalogu, w którym znajduje się plik wykonywalny
    )
endforeach()

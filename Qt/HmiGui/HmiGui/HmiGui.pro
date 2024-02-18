#-------------------------------------------------
#
# Project created by QtCreator 2013-05-01T13:46:52
#
#-------------------------------------------------

#CONFIG += release

RC_FILE = HMIUI.rc

QT       += core gui network svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG		+= plugin
TARGET		= HmiGui
TEMPLATE 	= lib
DESTDIR		= ./release/lib

#--
#_LINUX或WIN32
#QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT 这个禁止调试信息
#--
DEFINES += _UNICODE WIN32 WIN32SCALE

#普通发布版默认开启的宏：WIN32 WIN32SCALE
#WIN32SCALE  #缩放宏
#pro文件编写参考以下链接：
#https://www.cnblogs.com/Braveliu/p/5107550.html
QMAKE_CXXFLAGS += -fPIC
QMAKE_CFLAGS += -fPIC

contains(DEFINES, _LINUX) {
    DEFINES -= WIN32
    DEFINES -= WIN32SCALE
    DEFINES += _LINUX_
    DEFINES += _ARM_LINUX_
}

contains(DEFINES, WIN32) {
    DEFINES += _CRT_SECURE_NO_WARNINGS
}

CONFIG(release, debug|release):{
    UI_DIR = $$PWD/tmp/release/ui
    MOC_DIR = $$PWD/tmp/release/moc
    OBJECTS_DIR = $$PWD/tmp/release/obj
    RCC_DIR = $$PWD/tmp/release/rcc
}

CONFIG(debug, debug|release):{
    UI_DIR = $$PWD/tmp/debug/ui
    MOC_DIR = $$PWD/tmp/debug/moc
    OBJECTS_DIR = $$PWD/tmp/debug/obj
    RCC_DIR = $$PWD/tmp/debug/rcc
}


CONFIG(debug, debug|release){
    contains(DEFINES, board_a7) {
        LIBS += -L"${PWD}"/../../PIBin/Hmi_bin/ARM_HF_Debug
    }
    contains(DEFINES, board_a8) {
        LIBS += -L"${PWD}"/../../PIBin/Hmi_bin/ARM_Debug
    }
	contains(DEFINES, board_a35) {
        LIBS += -L"${PWD}"/../../PIBin/Hmi_bin/ARM_HF35_Debug
    }
}else{
    contains(DEFINES, board_a7) {
        LIBS += -L"${PWD}"/../../PIBin/Hmi_bin/ARM_HF_Release
    }
    contains(DEFINES, board_a8) {
        LIBS += -L"${PWD}"/../../PIBin/Hmi_bin/ARM_Release
    }
	contains(DEFINES, board_a35) {
        LIBS += -L"${PWD}"/../../PIBin/Hmi_bin/ARM_HF35_Release
    }
}

contains(DEFINES, board_a7) {
    INCLUDEPATH += \
	"${PWD}"/../../PIBin/Include \
	/opt/hmi/toolchain/sysroot_hf/usr/include
}

contains(DEFINES, board_a35) {
    INCLUDEPATH += \
	"${PWD}"/../../PIBin/Include \
}

contains(DEFINES, board_a8) {
    INCLUDEPATH += \
		"${PWD}"/../../PIBin/Include
}


SOURCES += commonpart.cpp \
	custompart.cpp \
	generalpart.cpp \
	hmimgr.cpp \
	hmipart.cpp \
    hmiscreen.cpp \
	hmisendevent.cpp \
	hmitext.cpp \
    hmiutility.cpp	\
	imgcachemgr.cpp \
	keyBoard.cpp \
	parttype2.cpp \
	qhmilabel.cpp \
	qrlabel.cpp \
    GuiThread.cpp \
    tooltip.cpp \
	slider.cpp	\
	msgtip.cpp \
	HmiGui\HmiGui.cpp
    
    
    

HEADERS  += \
    commonpart.h \
    custompart.h \
    generalpart.h \
    hmidef.h \
    hmimgr.h \
    hmipart.h \
    hmiscreen.h \
    hmisendevent.h \
    hmitext.h \
    hmiutility.h \
    imgcachemgr.h \
	keyBoard.h \
    parttype2.h \
    qhmilabel.h \
    qrencode.h \
    qrlabel.h \
    GuiThread.h \
	tooltip.h \
	slider.h	\
	msgtip.h \
	HmiGui\HmiGui.h
	

contains(DEFINES, WIN32) {
    SOURCES -=  qrlabel.cpp
    HEADERS -=  qrlabel.h
}




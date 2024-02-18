# cmake required version
CMAKE_MINIMUM_REQUIRED(VERSION 3.5.1)

STRING (REGEX REPLACE ".*/" "" TARGET_NAME ${CMAKE_CURRENT_LIST_DIR})
project(${TARGET_NAME})

if(CMAKE_SYSTEM_NAME MATCHES "Windows") 
    #if is in windows
	ADD_DEFINITIONS(-D_WIN32)
	ADD_DEFINITIONS(-DWIN32SCALE)
    set(IsLinux OFF)
else()
	set(IsLinux ON)
endif()
#MESSAGE("current platform: ${CMAKE_SYSTEM_NAME}")

# set xmake config root
if(IsLinux)
	SET (CMAKE_CONFIG_ROOT /etc/cmake)
	add_definitions(-D_LINUX_)
else() 
	set(CMAKE_CONFIG_ROOT $ENV{XMAKE}/etc)
	set(BUILD_PLATFORM "WIN32")
endif()

# include xmake common
INCLUDE (${CMAKE_CONFIG_ROOT}/common.cmake)

STRING (REGEX REPLACE ".*/" "" TARGET_NAME ${CMAKE_CURRENT_LIST_DIR})

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

set(BUILD_WITHOUTUI OFF)
if(DEV_MODE STREQUAL "BUILD-WITHOUTUI")
    set(BUILD_WITHOUTUI ON)
    add_definitions(-DWITHOUT_UI)
else()
    set(DEV_MODE "BUILD-WITHUI")
    add_definitions(-DWITH_UI)
endif()
message("${TARGET_NAME} code is ${DEV_MODE}")

# set project type, a8 use qt4
IF(a8 STREQUAL ${BUILD_PLATFORM})
	SET(PROJECT_TYPE Qt4)
ELSEIF(a8plus STREQUAL ${BUILD_PLATFORM})
	SET(PROJECT_TYPE Qt4)
ELSEIF(a7 STREQUAL ${BUILD_PLATFORM})
	SET(PROJECT_TYPE Qt5)
ELSEIF(rk3308 STREQUAL ${BUILD_PLATFORM})
	SET(PROJECT_TYPE Qt5)
elseif(NOT IsLinux)
	set(PROJECT_TYPE "WIN32")
ELSE()
	SET(PROJECT_TYPE Qt5)
ENDIF()

# enable cmake options
#SET(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set compiler options
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -rdynamic")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

# if qt project
IF(${PROJECT_TYPE} STREQUAL Qt4 OR ${PROJECT_TYPE} STREQUAL Qt5)
	IF(${PROJECT_TYPE} STREQUAL Qt4)
		IF(a7 STREQUAL ${BUILD_PLATFORM})
			SET(QT_QMAKE_EXECUTABLE /opt/toolchain/a7/qt4.8.4/bin/qmake)
		ELSEIF(a8 STREQUAL ${BUILD_PLATFORM} OR a8plus STREQUAL ${BUILD_PLATFORM})
			SET(QT_QMAKE_EXECUTABLE /opt/toolchain/a8/qt4.8.4/bin/qmake)
		ELSEIF(rk3308 STREQUAL ${BUILD_PLATFORM})
			SET(QT_QMAKE_EXECUTABLE /opt/toolchain/rk3308/qt4.8.4/bin/qmake)
		ENDIF()
		
		SET(QT_USE_QTCORE TRUE)
		SET(QT_USE_QTGUI TRUE)
		SET(QT_USE_QTSVG TRUE)
		#SET(QT_USE_QTNETWORK TRUE)
		FIND_PACKAGE(Qt4 REQUIRED)
		INCLUDE(${QT_USE_FILE})
		ADD_DEFINITIONS(-D_UNICODE -DQT_PLUGIN -DQT_SHARED ${QT_DEFINITIONS})
	ELSE()
		IF(a7 STREQUAL ${BUILD_PLATFORM})
			SET(Qt5_DIR /opt/toolchain/a7/qt5.12.11/lib/cmake/Qt5)
		ELSEIF(rk3308 STREQUAL ${BUILD_PLATFORM})
			SET(Qt5_DIR /opt/toolchain/rk3308/qt5.12.11/lib/cmake/Qt5)
		ELSEIF(ssd212 STREQUAL ${BUILD_PLATFORM})
			SET(Qt5_DIR /opt/toolchain/ssd212/qt5.12.11/lib/cmake/Qt5)
		ENDIF()

		FIND_PACKAGE(Qt5 COMPONENTS Widgets Gui Core Svg REQUIRED)
	ENDIF()
	
	SET(CMAKE_AUTOMOC ON)
	#SET(CMAKE_AUTOUIC ON)
	#SET(CMAKE_AUTORCC ON)
ENDIF()

if("WIN32" STREQUAL PROJECT_TYPE)

	SET(QT_QMAKE_EXECUTABLE D:/Qt5.12.11/5.12.11/msvc2017/bin/qmake.exe)
	SET(QT_USE_WIDGETS TRUE)
	SET(QT_USE_QTCORE TRUE)
	SET(QT_USE_QTGUI TRUE)
	SET(QT_USE_QTSVG TRUE)
	FIND_PACKAGE(Qt5 COMPONENTS Widgets Gui Core Svg REQUIRED)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)

endif()

#middleware directory
if(IsLinux)
	SET(MIDDLEWARE_ROOT "$ENV{MIDDLEWARE_ROOT}")
	SET(MIDDLEWARE_I2T "$ENV{MIDDLEWARE_I2T}")
else()
	SET(MIDDLEWARE_ROOT "$ENV{part1middle}")
	SET(MIDDLEWARE_I2T "")
endif()
if(NOT MIDDLEWARE_ROOT)
	SET (MIDDLEWARE_ROOT /opt/middleware)
ENDIF ()

if(NOT IsLinux)

	if(IsDebug)
		set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${MIDDLEWARE_ROOT}/Hmi_bin/Win_Debug/)
		set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${MIDDLEWARE_ROOT}/Hmi_bin/Win_Debug/)
	else()
		set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${MIDDLEWARE_ROOT}/Hmi_bin/Win_Release/)
		set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RLEASE ${MIDDLEWARE_ROOT}/Hmi_bin/Win_Release/)
	endif()
endif()

if(IsLinux)
	SET (DIR_MIDDLEWARE "${MIDDLEWARE_ROOT}/${BUILD_PLATFORM}")
	SET (DIR_MIDDLEWARE_I2T "${MIDDLEWARE_I2T}/${BUILD_PLATFORM}")
else()
	SET (DIR_MIDDLEWARE "${MIDDLEWARE_ROOT}")
	SET (DIR_MIDDLEWARE_I2T "${MIDDLEWARE_I2T}/${BUILD_PLATFORM}")
endif()
message(STATUS "DIR_MIDDLEWARE = ${DIR_MIDDLEWARE}")

#user definations
IF(x86 STREQUAL ${BUILD_PLATFORM} OR rk3288 STREQUAL ${BUILD_PLATFORM})
	ADD_DEFINITIONS(-fopenmp)
ENDIF()

ADD_DEFINITIONS(-D${BUILD_PURPOSE})
if(IsLinux)
	ADD_DEFINITIONS(-Wno-deprecated)
	if(ssd212 STREQUAL ${BUILD_PLATFORM})
		ADD_DEFINITIONS(-Wno-stringop-overflow)
	endif()
	ADD_DEFINITIONS(-D_LINUX_ -D_ARM_LINUX_)
	ADD_DEFINITIONS(-D_LINUX)
endif()

#include directories
INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${DIR_MIDDLEWARE}/include)
INCLUDE_DIRECTORIES(${DIR_MIDDLEWARE_I2T}/include)
IF(a8plus STREQUAL ${BUILD_PLATFORM})
	INCLUDE_DIRECTORIES(${ADDITIONAL_INCLUDE_DIRECTORIES})
	message("${ADDITIONAL_INCLUDE_DIRECTORIES}")
ENDIF()
#INCLUDE_DIRECTORIES(<directory> ...)

#link directories
if(IsLinux)
	LINK_DIRECTORIES(${DIR_MIDDLEWARE}/lib)
	LINK_DIRECTORIES(${DIR_MIDDLEWARE_I2T}/lib)
else()
	LINK_DIRECTORIES(${DIR_MIDDLEWARE}/Hmi_bin/Win_${CMAKE_BUILD_TYPE})
	LINK_DIRECTORIES(${DIR_MIDDLEWARE}/Hmi_bin/Win_${CMAKE_BUILD_TYPE}/lib)
	LINK_DIRECTORIES(${DIR_MIDDLEWARE_I2T}/lib)
endif()
#LINK_DIRECTORIES(<directory> ...)
IF(a8plus STREQUAL ${BUILD_PLATFORM})
	LINK_DIRECTORIES(${ADDITIONAL_LINK_DIRECTORIES})
	message("${ADDITIONAL_LINK_DIRECTORIES}")
ENDIF()

#source sub directory
SET (CMAKE_SOURCE_SUBDIR EventMgr HmiGui msgTip/msgTip msgTip/lineShap msgTip/tipWidget screenSaver screenSaver/blinds screenSaver/bubbles screenSaver/common screenSaver/screenSaver screenSaver/lattice formula config formula/AddFormulaGroupWnd inputControls common common/MessageTip)
SET(UI_FILES formula/AddFormulaGroupWnd/AddFormulaGroupWnd.ui common/MessageTip/messagetip.ui)

IF(${PROJECT_TYPE} STREQUAL Qt4)
	QT4_WRAP_UI(WRAP_FILES ${UI_FILES})
ELSE()
	QT5_WRAP_UI(WRAP_FILES ${UI_FILES})
ENDIF()

#source files
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_LIST_DIR} SOURCE_FILES)
FOREACH (_VAR ${CMAKE_SOURCE_SUBDIR})
	AUX_SOURCE_DIRECTORY (${CMAKE_CURRENT_LIST_DIR}/${_VAR} SOURCE_FILES)
ENDFOREACH ()

#Generate a filter for the solution
if (NOT IsLinux)
	#add other .h to sln
	#set(SOURCE_FILES ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/HmiBase.h)

	foreach(file_path ${SOURCE_FILES})
		string(REGEX REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" relative_path ${file_path})
		string(REGEX MATCH ".cpp" is_match ${relative_path})
		if(is_match)
			if(NOT cpp_relative_dir STREQUAL "main")
				#add .cpp Correspondence .h
				string (REGEX REPLACE ".cpp" ".h" headPath ${relative_path})
				if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${headPath}) 

					set(SOURCE_FILES ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/${headPath})
				endif()

				string(REGEX REPLACE ".*/" "" srcFileName ${relative_path})
				string(REGEX REPLACE "/${srcFileName}" "" cpp_relative_dir ${relative_path})
				list(APPEND cpp_groups /${cpp_relative_dir}/)
			
				string(FIND ${relative_path} "/" needParengDir)
				if(NOT needParengDir LESS 0)
					
					list(FIND cpp_groups /${cpp_relative_dir}/ index)
					if(NOT index LESS 0)
						set(groupFiles 
						${relative_path} 
						${headPath})
						source_group(${cpp_relative_dir} FILES ${groupFiles})
					endif()
				endif()
			endif()
		endif(is_match)
	endforeach(file_path)

	#copy head to middleware
	set(CMAKE_SOURCE_SUBDIR ${CMAKE_SOURCE_SUBDIR} .)
	foreach(head_dir ${CMAKE_SOURCE_SUBDIR})
		
		if(${head_dir} STREQUAL ".")
			string(REGEX REPLACE ".*/" "" head_dir ${CMAKE_CURRENT_LIST_DIR})
			file(GLOB head_files *.h)
		else()
			file(GLOB head_files ${head_dir}/*.h)
			string(REGEX REPLACE ".*/" "" head_tempdir ${CMAKE_CURRENT_LIST_DIR})
			set(head_dir ${head_tempdir}/${head_dir})
		endif()

		#if dir is not exists�� to create dir
		set(dev_dir ${DIR_MIDDLEWARE}/include/${head_dir})
		if(NOT EXISTS dev_dir)
			file(MAKE_DIRECTORY ${dev_dir})
		endif()
		
		foreach(head_path ${head_files})

			if(IsLinux)
				file(COPY ${head_path} DESTINATION ${dev_dir})
			else()
				string(REGEX REPLACE ".*/" "" file_name ${head_path})
				file(COPY_FILE ${head_path} ${dev_dir}/${file_name})
			endif()
		endforeach()

	endforeach(head_dir)
endif()

#check exe mode and main.cpp file
SET(FILE_MAIN_CPP "${CMAKE_CURRENT_LIST_DIR}/main.cpp")
IF(ENABLE_EXE_MODE)
	IF(NOT EXISTS "${FILE_MAIN_CPP}")
		SET(ENABLE_EXE_MODE OFF)
	ENDIF()
ELSE()
	IF(EXISTS "${FILE_MAIN_CPP}")
		LIST(REMOVE_ITEM SOURCE_FILES "${FILE_MAIN_CPP}")
	ENDIF()	
ENDIF()

#add target
IF(ENABLE_EXE_MODE)
	ADD_EXECUTABLE(${TARGET_NAME} ${SOURCE_FILES})
ELSE()
	ADD_LIBRARY(${TARGET_NAME} SHARED ${SOURCE_FILES} ${WRAP_FILES})
ENDIF()

#link libraries
IF(${PROJECT_TYPE} STREQUAL Qt5)
	message("link qt5")
	TARGET_LINK_LIBRARIES(${TARGET_NAME} PRIVATE Qt5::Widgets Qt5::Gui Qt5::Core Qt5::Svg)
ENDIF()

IF(a8plus STREQUAL ${BUILD_PLATFORM})
	TARGET_LINK_LIBRARIES(${TARGET_NAME} rt libstdc++.a)
ENDIF()

if(NOT IsLinux)

	if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")

		set(WIN_LINK_LIB cbld)
	else()
		set(WIN_LINK_LIB cbl)
	endif()

endif()

if(NOT IsLinux)
	TARGET_LINK_LIBRARIES(${TARGET_NAME} Qt5::Widgets Qt5::Gui Qt5::Core Qt5::Svg ProtoXML qrencode platform ${WIN_LINK_LIB} Register RegisterOpt secure GlobalModule HmiBase utility)
endif()

set(PROJECT_NAME ${TARGET_NAME})

#install target
IF (DIR_MIDDLEWARE)
	SET (CMAKE_INSTALL_PREFIX ${DIR_MIDDLEWARE})
	if(IsLinux)
		INCLUDE (${CMAKE_CONFIG_ROOT}/install_lib.cmake)
	else()
		INCLUDE (${CMAKE_CONFIG_ROOT}/install.cmake)
	endif()
ENDIF ()

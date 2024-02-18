cls
@echo off
color 0a

echo ********************
echo Windows Install ".h"
echo ********************

echo 获取当前项目名称（文件夹名）
pushd %1 & for %%i in (.) do set ProjectName=%%~ni

echo 设置环境变量...
set SourceCodePath=%~dp0
cd ..\..\..\
set PIBinPath=%cd%\PIBin\
set PIBinIncludePath=%PIBinPath%Include\
set DestCodePath=%PIBinIncludePath%%ProjectName%\

if exist %PIBinPath% (
	echo %PIBinPath% 存在
) else (
	echo %PIBinPath% 不存在
	echo 创建%PIBinPath%
	mkdir %PIBinPath%
)

if exist %PIBinIncludePath% (
	echo %PIBinIncludePath% 存在
) else (
	echo %PIBinIncludePath% 不存在
	echo 创建%PIBinIncludePath%
	mkdir %PIBinIncludePath%
)

setlocal enabledelayedexpansion

if exist %DestCodePath% (
	echo %DestCodePath% 存在
	echo 清空%DestCodePath%
	rd /S /Q %DestCodePath%
	echo 创建%DestCodePath%
	mkdir %DestCodePath%
	
	rem 拷贝源文件中的头文件到目标文件夹下
	xcopy %SourceCodePath%*.h %DestCodePath% /S
) else (
	echo %DestCodePath% 不存在
	echo 创建%DestCodePath%
	mkdir %DestCodePath%
	
	rem 拷贝源文件中的头文件到目标文件夹下
	xcopy %SourceCodePath%*.h %DestCodePath% /S
)

pause


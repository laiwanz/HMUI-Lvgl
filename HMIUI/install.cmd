cls
@echo off
color 0a

echo ********************
echo Windows Install ".h"
echo ********************

echo ��ȡ��ǰ��Ŀ���ƣ��ļ�������
pushd %1 & for %%i in (.) do set ProjectName=%%~ni

echo ���û�������...
set SourceCodePath=%~dp0
cd ..\..\..\
set PIBinPath=%cd%\PIBin\
set PIBinIncludePath=%PIBinPath%Include\
set DestCodePath=%PIBinIncludePath%%ProjectName%\

if exist %PIBinPath% (
	echo %PIBinPath% ����
) else (
	echo %PIBinPath% ������
	echo ����%PIBinPath%
	mkdir %PIBinPath%
)

if exist %PIBinIncludePath% (
	echo %PIBinIncludePath% ����
) else (
	echo %PIBinIncludePath% ������
	echo ����%PIBinIncludePath%
	mkdir %PIBinIncludePath%
)

setlocal enabledelayedexpansion

if exist %DestCodePath% (
	echo %DestCodePath% ����
	echo ���%DestCodePath%
	rd /S /Q %DestCodePath%
	echo ����%DestCodePath%
	mkdir %DestCodePath%
	
	rem ����Դ�ļ��е�ͷ�ļ���Ŀ���ļ�����
	xcopy %SourceCodePath%*.h %DestCodePath% /S
) else (
	echo %DestCodePath% ������
	echo ����%DestCodePath%
	mkdir %DestCodePath%
	
	rem ����Դ�ļ��е�ͷ�ļ���Ŀ���ļ�����
	xcopy %SourceCodePath%*.h %DestCodePath% /S
)

pause


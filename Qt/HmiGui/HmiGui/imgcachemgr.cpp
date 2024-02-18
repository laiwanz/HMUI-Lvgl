#include "GlobalModule/GlobalModule.h"
#include "imgcachemgr.h"
#include <QImage>
#include <QMovie>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QPainter>
#include <QDataStream>
#include <QtSvg/QSvgRenderer>
#include <QPixmap>
#include <list>
#include <mutex>
#include "platform/platform.h"
#include <utility/utility.h>
#include "hmiutility.h"
#ifdef WIN32
#include <QLibrary>
#include "WECDecryptFile.h"
#include <cbl/cbl.h>
QLibrary mylib("WECDecryptDLL.dll");
static bool bLoad = false;
typedef char* ( *DecryptFun )( char*, char* ); //定义函数指针，以备调用
#endif
#ifndef WIN32
#include <zlib.h>
#define DEFAULT_ERR_IMG     "/mnt/wecon/wecon/run/project/img/400.png"  // 位图预览默认显示图片 xqh 20190903
#endif
#include "macrosplatform/path.h"

using namespace UTILITY_NAMESPACE;
using namespace PLATFORM_NAMESPACE;

#define MAX_IMG_WIDTH   1920    // 图片最大分辨率1920*1080
#define MAX_IMG_HEIGHT  1080    // 图片最大分辨率1920*1080

#define SVGEXTNAME  ".svg"  //svg扩展名
#define PIXEXTNAME  ".pix"  //pix扩展名
#define GIFEXTNAME  ".gif"
#define PNGEXTNAME  ".png"  //png扩展名
#define JPGEXTNAME  ".jpg"  //jpg扩展名
#define JPEGEXTNAME ".jpeg" //jpeg扩展名
#define BMPEXTNAME  ".bmp"  //bmp扩展名
#define IMAGE_PATH  "/mnt/data/prj/IMAGE"//zlib压缩完文件路径

#define LoadImgTaskNmae "LoadImgToCache"    // 加载图片任务名称

std::mutex g_imageMutex;
QList<IMG_T *> ImgCacheMgr::m_ImgList;

static void IMGRead(const QString &strFile, QByteArray  &array) {
	QFile file(strFile);

	if (file.open(QIODevice::ReadOnly)) {
		array = file.readAll();
	}

	file.close();
}

static void IMGWrite(const QString &strFile, const QByteArray  &array) {
	QFileInfo	info(strFile);

	//校验文件
	if (info.exists()) {
		QByteArray	desArray;
		IMGRead(strFile, desArray);
		if (desArray == array) {
			return;
		}
	}

	QFile		file(strFile);

	//打开文件，只写方式
	if (file.open(QIODevice::WriteOnly)) {
		file.write(array);
	}
	file.flush();
#ifdef _LINUX_
	fsync(file.handle());
#endif
	file.close();
}

//缩放
static bool IMGZoom(QImage &img, const int &nWidth, const int &nHeight) {
	if (img.isNull()) {
		return false;
	}

	if (nWidth > 0 &&
		nHeight > 0 &&
		img.width() > nWidth &&
		img.height() > nHeight) {
		QSize  size(nWidth, nHeight);

		img = img.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		return true;
	}

	return false;
}

static std::string  NormalIMGDIR() {
	std::string		strDir;
	char			szPath[MAX_PATH] = { 0 };
#ifdef WIN32
	{
		char		szTmp[MAX_PATH] = { 0 };

		CState::GetTempPathCos(MAX_PATH, szTmp);
		_snprintf(szPath, MAX_PATH, "%s%s\\%s", szTmp, TMPDIR, "crypt\\");
	}
#elif _LINUX_
	{
		_snprintf(szPath, MAX_PATH, "%s%s", IMAGE_PATH, "/");
	}
#endif

	strDir = szPath;
	return strDir;
}

static void LoadNormalIMG(QList<IMG_T *> *pList) {
	unsigned int		nPhyMemorySize = CPlatFlash::getPhyMemorySize();
	std::string			strDir = NormalIMGDIR();

	if (pList == NULL) {
		return;
	}
	std::unique_lock<mutex> lock(g_imageMutex);
	foreach(IMG_T *pImg, *pList) {

		//普通图片
		if (pImg == NULL ||
			pImg->nImgType != ImgCacheMgr::NORMALIMG) {
			continue;
		}

		QImage image(pImg->strImgPath);

		if (image.isNull()) {
			continue;
		}

		//缩放
		IMGZoom(image, pImg->nScaleWidth, pImg->nScaleHeight);

		if (512 == nPhyMemorySize)		//大内存，直接缓冲
		{
			if (pImg->pix == NULL) {
				pImg->pix = new QPixmap;
				pImg->pix->convertFromImage(image);
			}
		}
		else//小内存
		{
			BitmapInfo *pImgInfo = (BitmapInfo *) malloc(sizeof(BitmapInfo));
			if (!pImgInfo) {
				continue;
			}

			memset(pImgInfo, 0, sizeof(BitmapInfo));
			pImgInfo->nWidth = image.width();
			pImgInfo->nHeight = image.height();
			pImgInfo->imgF = image.format();

			//对于位深小于24的位图，需要保存颜色表
			if (image.depth() < 24) {
				pImgInfo->pColorTable = new QVector<QRgb>;
				*pImgInfo->pColorTable = image.colorTable();
			}

			if (64 == nPhyMemorySize) {
				if (!cbl::isDir(strDir)) {
					cbl::makeDir(strDir);
				}

				QByteArray  array = qCompress(image.bits(), image.byteCount(), 1);
				QFileInfo	srcFile(pImg->strImgPath);

				IMGWrite(strDir.c_str() + srcFile.fileName(), array);
			}
			else {
				pImgInfo->imgArray = new QByteArray;
				QByteArray array = qCompress(image.bits(), image.byteCount(), 1);
				pImgInfo->imgArray->append(qCompress(image.bits(), image.byteCount(), 1));
			}

			pImg->pBmpInfo = pImgInfo;
		}
	}
}

QByteArray ImgCacheMgr::m_ImgArray;
ImgCacheMgr::ImgCacheMgr() {

}

void *LoadImgToCache(void* pListImg) {
	QList<IMG_T *>	*pImgList = ( QList<IMG_T *> * )pListImg;

	if (!pListImg) {
		return NULL;
	}

#ifndef WIN32
	for (int nI = 0; nI < 40; nI++)//4秒后开始。前面的时间先给工程加载、其他初始化等
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
#endif

	LoadNormalIMG(pImgList);

	qDebug() << "[QT  ]:LoadImgToCache Finish";
	CState::ClearSystemCache(1);
	return NULL;
}

int ImgCacheMgr::Init(const void *pListImg, const QString &sPath) {

	std::list<IMGNAME_T> *plistImage = ( std::list<IMGNAME_T> * )pListImg;

	if (!plistImage) {

		return -1;
	}
	int		nIndex = 0;

	for (auto &iter : *plistImage) {

		IMG_T *pImgInfo = new IMG_T;
		pImgInfo->pix = NULL;
		pImgInfo->svg = NULL;
		pImgInfo->pBmpInfo = NULL;

		//图片序号
		pImgInfo->nImgIndex = nIndex;

		// 图片路径
		pImgInfo->strImgPath = sPath;
		pImgInfo->strImgPath.append("img/");
		pImgInfo->strImgPath.append(iter.sFileName.c_str());

		// 图片类型
		pImgInfo->nImgType = NORMALIMG;
		pImgInfo->nScaleHeight = iter.nZoomHeight;
		pImgInfo->nScaleWidth = iter.nZoomWidth;

		// svg类型
		if (pImgInfo->strImgPath.right(4).compare(SVGEXTNAME, Qt::CaseInsensitive) == 0) {

			pImgInfo->nImgType = SVGIMG;
		}
		else {

			QFile imgFile(pImgInfo->strImgPath);
			if (imgFile.open(QIODevice::ReadOnly)) {

				char head[3];
				imgFile.read(head, 3);
				if (head[0] == 0x47 &&
					head[1] == 0x49 &&
					head[2] == 0x46) {

					pImgInfo->nImgType = GIFIMG;
				}
				imgFile.close();
			}
		}

		// 加入链表
		std::unique_lock<mutex> lock(g_imageMutex);
		m_ImgList.append(pImgInfo);
		nIndex++;
	}

	int		nWidth = 0, nHeight = 0, nBit = 0;

#ifdef WIN32
	nWidth = 1920;
	nHeight = 1080;
	nBit = 32;
#else
	cbl::CLcd devLcd;
	devLcd.getResolution(nWidth, nHeight, nBit);
#endif 
	m_ImgArray.fill('\0', nWidth*nHeight*nBit / 8);

	{//创建一个线程去加载图片到缓存中
		CUtiThreadpool::get_instance().commit((int) ( &LoadImgToCache ), LoadImgToCache, (void*) &m_ImgList);
	}
	return 0;
}

IMG_T* ImgCacheMgr::GetImgInfo(int nImgIndex) {
	//请求范围不合法
	if (nImgIndex < 0 || nImgIndex > m_ImgList.size() || m_ImgList.size() <= 0) {
		return NULL;
	}

	if (m_ImgList[nImgIndex]->nImgType == GIFIMG) {
		return m_ImgList[nImgIndex];
	}

	LoadImg(m_ImgList[nImgIndex]);

	return m_ImgList[nImgIndex];
}

void ImgCacheMgr::Clear() {

	foreach(IMG_T *pImg, m_ImgList) {
		if (pImg == NULL) {
			continue;
		}

		pImg->mutex.lock();

		if (pImg->pix != NULL) {
			delete pImg->pix;
			pImg->pix = NULL;
		}

		if (pImg->svg != NULL) {
			delete pImg->svg;
			pImg->svg = NULL;
		}

		pImg->mutex.unlock();
	}
}

void ImgCacheMgr::Destroy() {
	std::unique_lock<mutex> lock(g_imageMutex);
	foreach(IMG_T *pImg, m_ImgList) {
		if (pImg == NULL) {
			continue;
		}

		pImg->mutex.lock();
		if (pImg->pix != NULL) {
			delete pImg->pix;
			pImg->pix = NULL;
		}

		if (pImg->svg != NULL) {
			delete pImg->svg;
			pImg->svg = NULL;
		}

		if (pImg->pBmpInfo) {
			if (pImg->pBmpInfo->pColorTable) {
				delete pImg->pBmpInfo->pColorTable;
				pImg->pBmpInfo->pColorTable = NULL;
			}

			if (pImg->pBmpInfo->imgArray) {
				delete pImg->pBmpInfo->imgArray;
				pImg->pBmpInfo->imgArray = NULL;
			}

			free(pImg->pBmpInfo);
			pImg->pBmpInfo = NULL;
		}
		pImg->mutex.unlock();

		delete pImg;
		pImg = NULL;
	}

	m_ImgList.clear();
}

void ImgCacheMgr::LoadImg(IMG_T *pImgInfo) {
	if (pImgInfo->nImgType == NORMALIMG && pImgInfo->pix == NULL) {
		if (checkPixFormat(pImgInfo->strImgPath)) {
			QFile piximg(pImgInfo->strImgPath);
			if (!piximg.open(QIODevice::ReadOnly)) {
				qDebug() << "open file error:" << pImgInfo->strImgPath;
				return;
			}
			QByteArray bytes = piximg.readAll();
			QDataStream in(bytes);
			pImgInfo->pix = new QPixmap();
			in >> *( pImgInfo->pix );
			piximg.close();
		}
		else {
			if (!pImgInfo->pBmpInfo)//没有缓冲起来
			{
				QImage img(pImgInfo->strImgPath);
				if (img.isNull()) {
					qDebug() << "img:" << pImgInfo->strImgPath << " error!!!!!!";
					return;
				}
				else {
					pImgInfo->pix = new QPixmap();
					pImgInfo->pix->convertFromImage(img);
				}
			}
			else {
				QImage img(pImgInfo->pBmpInfo->nWidth, pImgInfo->pBmpInfo->nHeight, pImgInfo->pBmpInfo->imgF);

				if (!pImgInfo->pBmpInfo->imgArray ||
					pImgInfo->pBmpInfo->imgArray->isEmpty()) {
					std::string		strPath = NormalIMGDIR();

					QFileInfo srcFile(pImgInfo->strImgPath);
					QFileInfo desFile(strPath.c_str() + srcFile.fileName());

					if (!desFile.exists()) {
						return;
					}

					m_ImgArray.clear();
					IMGRead(desFile.filePath(), m_ImgArray);

					m_ImgArray = qUncompress(m_ImgArray);
					memcpy((char *) img.bits(), m_ImgArray.data(), m_ImgArray.size());
				}
				else {
					m_ImgArray.clear();
					m_ImgArray = qUncompress(*pImgInfo->pBmpInfo->imgArray);
					memcpy((char *) img.bits(), m_ImgArray.data(), m_ImgArray.size());
				}

				if (img.depth() < 24) {
					img.setColorTable(*pImgInfo->pBmpInfo->pColorTable);
				}

				pImgInfo->pix = new QPixmap;
				pImgInfo->pix->convertFromImage(img);
			}
		}
	}
	else if (pImgInfo->nImgType == SVGIMG && pImgInfo->svg == NULL) {
		LoadSvg(pImgInfo);
	}

}

bool ImgCacheMgr::checkPixFormat(QString fileName) {
	bool bRet = false;

	if (fileName.right(4).compare(PIXEXTNAME, Qt::CaseInsensitive) == 0) {
		bRet = true;
	}

	return bRet;
}

int ImgCacheMgr::loadSvg(const std::string &sPath, QByteArray &bytearrays) {
#ifdef WIN32
	static DecryptFun funcDecrypt = nullptr;
	if (nullptr == funcDecrypt) {
		if (mylib.load()) {
			funcDecrypt = (DecryptFun)mylib.resolve("DES_DecryptFile");
		}
	}
	if (nullptr != funcDecrypt) {
		char    szFile_mbs[MAX_PATH] = { 0 };
		char    szKey[32] = { "we_con_svg_key" };
		CUnicode::utf8_to_mbs(sPath.c_str(), szFile_mbs, MAX_PATH);
		char *pbuffer = funcDecrypt((char*)szFile_mbs, szKey);
		if (nullptr != pbuffer) {
			bytearrays.append(pbuffer);
		}
	}
#endif
	return 0;
}

void ImgCacheMgr::LoadSvg(IMG_T *pImgInfo) {
#ifdef _LINUX_
	pImgInfo->svg = new QSvgRenderer(pImgInfo->strImgPath);
#else
	static DecryptFun fun;
	std::string str = pImgInfo->strImgPath.toStdString();
	const char* szFile = str.c_str();
	if (bLoad == false) {
		if (mylib.load())
			fun = (DecryptFun) mylib.resolve("DES_DecryptFile");
	}
	if (fun) {
		char    szFile_mbs[MAX_PATH] = { 0 };
		char    szKey[32] = { "we_con_svg_key" };

		CUnicode::utf8_to_mbs(szFile, szFile_mbs, MAX_PATH);
		char *pbuffer = fun((char*) szFile_mbs, szKey);
		if (nullptr != pbuffer) {
			QByteArray byteArray(pbuffer);
			pImgInfo->svg = new QSvgRenderer(byteArray);
		}
	}
#endif
}

void ImgCacheMgr::LoadCustomIMG(const QString &strImg, QImage &Image, const RECT &rect) {
	bool		bRet = false;

	if (0 == strImg.right(strlen(PNGEXTNAME)).compare(PNGEXTNAME, Qt::CaseInsensitive) ||
		0 == strImg.right(strlen(BMPEXTNAME)).compare(BMPEXTNAME, Qt::CaseInsensitive) ||
		0 == strImg.right(strlen(JPGEXTNAME)).compare(JPGEXTNAME, Qt::CaseInsensitive) ||
		0 == strImg.right(strlen(JPEGEXTNAME)).compare(JPEGEXTNAME, Qt::CaseInsensitive)) {
		QImageReader    reader;

		reader.setFileName(strImg);   // 不直接让Image去load图片是为了防止图片太大加载后导致内存不足让进程崩溃 xqh 20210112
		if (true == reader.canRead()) {
			QSize   imgSize = reader.size();    // 获取图片尺寸

			if (0 == imgSize.width() ||
				0 == imgSize.height() ||
				imgSize.width() > MAX_IMG_WIDTH ||
				imgSize.height() > MAX_IMG_HEIGHT)   // 位图预览显示的图片像素超过1920*1080时不显示
			{
				bRet = false;
			}
			else {
				Image = reader.read();
				if (!Image.isNull()) {
					IMGZoom(Image, rect.right - rect.left, rect.bottom - rect.top);
					bRet = true;
				}
			}
		}
	}

	if (!bRet) {
		QString		strPath;
#ifndef WIN32
		strPath = DEFAULT_ERR_IMG;
#else
		char    szTmpPath[MAX_PATH] = { 0 };

		CState::GetTempPathCos(MAX_PATH, szTmpPath);
		strPath.append(szTmpPath);
		strPath.append("HMIEmulation/img/400.png");
#endif
		Image.load(strPath);
	}
}
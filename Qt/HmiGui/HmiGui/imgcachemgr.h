#ifndef IMGCACHEMGR_H
#define IMGCACHEMGR_H
#include <QString>
#include <QList>
#include <QStack>
#include <QMutex>
#include <QImage>
#include "hmidef.h"
#include <QtSvg/QSvgRenderer>
#include "macrosplatform/macrosplatform.h"

typedef struct _tagBitmapInfo {
	int				nWidth;			// 图片宽
	int				nHeight;		// 图片高
	QByteArray		*imgArray;
	QImage::Format	imgF;		    // 图片格式
	QVector<QRgb>	*pColorTable;	// 保存颜色表
} BitmapInfo;

typedef struct _tagIMAGE_T {
	int                 nImgIndex;
	QString             strImgPath;
	QPixmap             *pix;
	QSvgRenderer        *svg;
	QMutex              mutex;
	int					nImgType;
	int					nScaleWidth;	// 图片缩放宽度
	int					nScaleHeight;	// 图片缩放高度
	BitmapInfo			*pBmpInfo;
} IMG_T;

class ImgCacheMgr {
public:
	ImgCacheMgr();
	static int Init(const void *pListImg, const QString &sPath);
	static void Destroy();
	static void Clear();
	//获取失败，将返回空
	static IMG_T* GetImgInfo(int nImgIndex);
	static void LoadCustomIMG(const QString &strImg, QImage &Image, const RECT &rect);
	static int loadSvg(const std::string &sPath, QByteArray &bytearrays);
private:
	static void inline LoadSvg(IMG_T *pImgInfo);
	static void inline LoadImg(IMG_T *pImgInfo);
	static bool inline checkPixFormat(QString fileName);
private:
	static QList<IMG_T *> m_ImgList;
	static QByteArray m_ImgArray;
public:
	enum IMGTYPE {
		NORMALIMG,
		GIFIMG,
		SVGIMG
	};
};
#endif // IMGCACHEMGR_H
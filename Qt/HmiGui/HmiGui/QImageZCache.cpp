#include "QImageZCache.h"
#include <cbl/cbl.h>

/* construct
**/
QImageZCache::QImageZCache(){

}

/* destruct
**/
QImageZCache::~QImageZCache(){

}

/* create cache from image
**/
int QImageZCache::createFrom(const QImage& image){
    QIMAGE_INFO_T info;
    string sGzipData;

    if(this->getImageInfo(image, info) < 0){
        return -1;
    }
    if(cbl::gzipBuffer(string((char *)image.bits(), info.nSizeInBytes), sGzipData) < 0){
        return -2;
    }
    m_nCacheSize = int(sGzipData.size());
    m_ptrCache.reset(new unsigned char[m_nCacheSize]);
    memcpy(m_ptrCache.get(), sGzipData.data(), m_nCacheSize);
    m_imageInfo = info;

    return 0;
}

/* release cache to image
**/
int QImageZCache::releaseTo(QImage& image){

    string sGunzipData;

    if(m_nCacheSize <= 0){
        return -1;
    }
    if(cbl::gunzipBuffer(string((char *)m_ptrCache.get(), m_nCacheSize), sGunzipData) < 0){
        return -2;
    }
    image = QImage((const uchar *)sGunzipData.data(), m_imageInfo.nWidth, m_imageInfo.nHeight, m_imageInfo.format).copy();

    return 0;
}

/* check cache whether valid
**/
bool QImageZCache::isValid(){
    return (m_nCacheSize > 0);
}

/* get image info
**/
int QImageZCache::getImageInfo(QIMAGE_INFO_T& info){
    if(m_nCacheSize <= 0){
        return -1;
    }
    info = m_imageInfo;

    return 0;
}

/* get cache size
**/
int QImageZCache::getCacheSize(){
    return m_nCacheSize;
}

/* get image info inter
**/
int QImageZCache::getImageInfo(const QImage& image, QIMAGE_INFO_T& info){
    int nSizeInBytes;

    if((NULL == image.bits()) || ((nSizeInBytes = image.byteCount()) <= 0)){
        return -1;
    }
    info.nWidth = image.width();
    info.nHeight = image.height();
    info.format = image.format();
    info.nBytesPerLine = image.bytesPerLine();
    info.nSizeInBytes = nSizeInBytes;

    return 0;
}

#ifndef QIMAGEZCACHE_H
#define QIMAGEZCACHE_H

#include <string>
#include <QImage>
#include <memory>

using namespace std;

class QImageZCache
{
public:
    typedef struct{
      int nWidth;
      int nHeight;
      QImage::Format format;
      int nBytesPerLine;
      int nSizeInBytes;
    }QIMAGE_INFO_T;

public:
    QImageZCache();
    virtual ~QImageZCache();

    int createFrom(const QImage& image);
    int releaseTo(QImage& image);
    bool isValid();
    int getImageInfo(QIMAGE_INFO_T& info);
    int getCacheSize();

private:
    int getImageInfo(const QImage& image, QIMAGE_INFO_T& info);

private:
    QIMAGE_INFO_T m_imageInfo;
    unique_ptr<unsigned char []> m_ptrCache{nullptr};
    int m_nCacheSize{-1};
};

#endif // QIMAGEZCACHE_H

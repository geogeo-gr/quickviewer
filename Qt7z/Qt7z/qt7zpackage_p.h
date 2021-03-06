#ifndef QT7ZPACKAGE_P_H
#define QT7ZPACKAGE_P_H

#include <QtCore>
#include "qt7zpackage.h"
#include "qt7zfileinfo.h"


#ifdef Q_OS_WIN
#include "7zip/CPP/Common/MyWindows.h"
#include "7zip/CPP/Common/MyCom.h"
#include "7zip/CPP/Common/MyInitGuid.h"
#include "7zip/CPP/7zip/Bundles/Alone7z/StdAfx.h"
#include "7zip/CPP/7zip/Common/RegisterArc.h"
#include "7zip/CPP/7zip/Common/RegisterCodec.h"
#include "7zip/CPP/7zip/UI/Common/OpenArchive.h"
#include "7zip/CPP/7zip/UI/Common/Extract.h"
#include "7zip/CPP/Windows/PropVariantConv.h"
#else
#include "p7zip/CPP/include_windows/windows.h"
#include "p7zip/CPP/include_windows/basetyps.h"
#include "p7zip/CPP/include_windows/tchar.h"
#include "p7zip/CPP/myWindows/StdAfx.h"
#include "p7zip/CPP/Common/MyWindows.h"
#include "p7zip/CPP/Common/MyCom.h"
#include "p7zip/CPP/Common/MyInitGuid.h"
#include "p7zip/CPP/7zip/Common/RegisterArc.h"
#include "p7zip/CPP/7zip/Common/RegisterCodec.h"
#include "p7zip/CPP/7zip/UI/Common/OpenArchive.h"
#include "p7zip/CPP/7zip/UI/Common/Extract.h"
#include "p7zip/CPP/Windows/PropVariantConv.h"
#endif

class ExtractCallback;
class Qt7zPackagePrivate;

struct CListUInt64Def
{
  UInt64 Val;
  bool Def;

  CListUInt64Def(): Val(0), Def(false) {}
  void Add(UInt64 v) { Val += v; Def = true; }
  void Add(const CListUInt64Def &v) { if (v.Def) Add(v.Val); }
};



class SequentialStreamAdapter : public ISequentialOutStream, public IOutStreamFinish, public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP2(ISequentialOutStream, IOutStreamFinish)
    STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
    STDMETHOD(OutStreamFinish)();

    SequentialStreamAdapter(QIODevice *device, UInt32 index, ExtractCallback *callback)
        : CMyUnknownImp()
        , m_device(device)
        , m_index(index)
        , m_callback(callback) {}

    UInt32 m_index;
    ExtractCallback* m_callback;

private:
    QIODevice *m_device;
};

class OpenCallback : public IOpenCallbackUI, public CMyUnknownImp
{
public:
    OpenCallback(Qt7zPackage::Client *client) :
        m_client(client)
    {
    }

    INTERFACE_IOpenCallbackUI(override;)

private:
    Qt7zPackage::Client *m_client;
};

class ExtractCallback : public IArchiveExtractCallback, public ICryptoGetTextPassword, public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP2(IArchiveExtractCallback, ICryptoGetTextPassword)
    INTERFACE_IArchiveExtractCallback(;)
    STDMETHOD(CryptoGetTextPassword)(BSTR *password) override;

    ExtractCallback(Qt7zPackagePrivate *qt7zprivate, const Qt7zFileInfo fileInfo, QIODevice *outStream);

    int opRes() const
    {
        return m_opRes;
    }
    void WriteFileFinished(UInt32 index, QIODevice* device);

private:
    Qt7zPackagePrivate *m_p;
    Qt7zPackage::Client *m_client;
    Qt7zFileInfo m_fileInfo;
    QIODevice *m_outStream;
    int m_opRes;
    bool m_isCreateTemporary;
    int m_unpackFinished;

#ifndef _7ZIP_ST
    struct BufferCheck {
        QBuffer* buffer;
        bool finished;
    };
    QMutex m_mutex;
    QMap<UInt32, BufferCheck> m_unpackCache;
#endif
};


class Qt7zPackagePrivate
{
    friend class Qt7zPackage;
public:
    Qt7zPackagePrivate(Qt7zPackage *q);
    Qt7zPackagePrivate(Qt7zPackage *q, const QString &packagePath);
    ~Qt7zPackagePrivate();

    Qt7zPackage::Client *m_client;
    Qt7zPackage *m_q;
    QList<Qt7zFileInfo> m_fileInfoList;

private:
    void init();
    void reset();

    QString m_packagePath;
    bool m_isOpen;
    QStringList m_fileNameList;
    QHash<QString, UInt32> m_fileNameToIndex;

//    QScopedPointer<CCodecs> m_codecs;
    CCodecs* m_codecs;
    CArchiveLink m_arcLink;
};


















#endif // QT7ZPACKAGE_P_H

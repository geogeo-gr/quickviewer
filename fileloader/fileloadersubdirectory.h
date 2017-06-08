#ifndef FILELOADERSUBDIRECTORY_H
#define FILELOADERSUBDIRECTORY_H

#include "fileloaderdirectory.h"

class FileLoaderSubDirectory : public FileLoaderDirectory
{
public:
    FileLoaderSubDirectory(QObject* parent, QString path);

    ~FileLoaderSubDirectory() {}
protected:
    void initialize() override;
    void getFilesWithSubdirs(QString path, QString subpath);
};

#endif // FILELOADERSUBDIRECTORY_H

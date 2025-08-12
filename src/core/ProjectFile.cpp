#include "ProjectFile.h"

#include "DataSources.h"
#include "Project.h"

#include <zip.h>

#include <QBuffer>
#include <QJsonDocument>

QJsonObject ProjectFile::writeProject(const Project *p)
{
    return QJsonObject({
        { "version", "7.0" },
        { "nextDiagramIndex", p->_nextDiagramIndex },
        { "nextDiagramColorIndex", p->_nextDiagramColorIndex },
    });
}

QJsonObject ProjectFile::writeDiagram(const Diagram *d)
{
    return QJsonObject({
        { "title", d->title() },
        { "color", d->color().name() },
    });
}

QJsonObject ProjectFile::writeGraph(const Graph *g)
{
    QJsonObject dataSourceJson;
    g->dataSource()->save(dataSourceJson);

    return QJsonObject({
        { "title", g->title() },
        { "autoTitle", g->_autoTitle },
        { "color", g->color().name() },
        { "pointsCount", g->pointsCount() },
        { "dataSource", dataSourceJson },
    });
}

QByteArray ProjectFile::writeGraphData(const Graph *g)
{
    QByteArray data;
    QBuffer buf(&data);
    buf.open(QIODevice::WriteOnly);
    QDataStream stream(&buf);
    stream.setVersion(QDataStream::Qt_5_12);
    for (const auto &v : g->data().xs)
        stream << v;
    for (const auto &v : g->data().ys)
        stream << v;
    return data;
}

namespace {

struct ZipPrj
{
    ZipPrj(const QString &fileName)
    {
        auto fn = fileName.toStdString();
        int errCode;
        zp = zip_open(fn.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errCode);
        if (!zp) {
            zip_error_t err;
            zip_error_init_with_code(&err, errCode);
            error = QString("Failed to create target file: %1").arg(zip_error_strerror(&err));
            zip_error_fini(&err);
            return;
        }
    }
    
    ~ZipPrj()
    {
        if (zp)
            zip_discard(zp);
    }
    
    bool addFile(const QString &fileName, const QJsonObject &json)
    {
        return addFile(fileName, QJsonDocument(json).toJson());
    }
    
    bool addFile(const QString &fileName, const QByteArray &data)
    {
        if (!zp)
            return false;
        QString path = curDir.isEmpty() ? fileName : (curDir % '/' % fileName);
        zip_source_t *src = zip_source_buffer(zp, data.constData(), data.size(), 0);
        if (!src) {
            error = QString("Failed to prepare project data %1: %2").arg(path).arg(zip_error_strerror(zip_get_error(zp)));
            return false;
        }
        auto fn = path.toStdString();
        if (zip_file_add(zp, fn.c_str(), src, ZIP_FL_ENC_UTF_8) < 0) {
            error = QString("Failed to save project data %1: %2").arg(path).arg(zip_error_strerror(zip_get_error(zp)));
            return false;
        }
        return true;
    }
    
    bool save()
    {
        if (!zp)
            return false;
        if (zip_close(zp) < 0) {
            error = QString("Failed to save target file: %1").arg(zip_error_strerror(zip_get_error(zp)));
            return false;
        }
        zp = nullptr;
        return true;
    }
    
    QString error;
    QString curDir;
    zip_t *zp = nullptr;
};

} // namespace

#define FILE_PROPS QStringLiteral("props.json")
#define FILE_FORMAT QStringLiteral("format.json")
#define FILE_DATA QStringLiteral("data.bin")

QString ProjectFile::saveProject(const StorableData &data)
{
    ZipPrj zp(data.fileName);
    if (!zp.error.isEmpty())
        return zp.error;
        
    if (!zp.addFile(FILE_PROPS, writeProject(data.project)))
        return zp.error;

    for (auto d : std::as_const(data.diagrams)) {
        zp.curDir = d->id();

        if (!zp.addFile(FILE_PROPS, writeDiagram(d)))
            return zp.error;
        
        if (data.formats.contains(d)) {
            if (!zp.addFile(FILE_FORMAT, data.formats[d]))
                return zp.error;
        }
        
        for (auto it = d->_graphs.cbegin(); it != d->_graphs.cend(); it++) {
            const Graph *g = it.value();

            zp.curDir = d->id() + '/' + g->id();

            if (!zp.addFile(FILE_PROPS, writeGraph(g)))
                return zp.error;
            
            if (data.formats.contains(g)) {
                if (!zp.addFile(FILE_FORMAT, data.formats[g]))
                    return zp.error;
            }

            if (!zp.addFile(FILE_DATA, writeGraphData(g)))
                return zp.error;
        }
    }

    if (!zp.save())
        return zp.error;

    return {};
}

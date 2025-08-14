#include "ProjectFile.h"

#include "DataSources.h"
#include "Modifiers.h"
#include "Project.h"

#include <zip.h>

#include <QJsonArray>
#include <QJsonDocument>

#define PROJECT_VERSION "7.0"
#define FILE_PROPS QStringLiteral("props.json")
#define FILE_FORMAT QStringLiteral("format.json")
#define FILE_DATA QStringLiteral("data.bin")

static QColor jsonToColor(const QJsonValue& val, const QColor& def)
{
    QColor color(val.toString());
    return color.isValid() ? color : def;
}

QJsonObject ProjectFile::writeProject(const Project *p)
{
    return QJsonObject({
        { "version", PROJECT_VERSION },
        { "zipVersion", zip_libzip_version() },
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

    QJsonArray modifiersJson;
    for (auto m : std::as_const(g->_modifiers)) {
        QJsonObject modifierJson;
        m->save(modifierJson);
        modifiersJson.append(modifierJson);
    }
    
    return QJsonObject({
        { "title", g->title() },
        { "autoTitle", g->_autoTitle },
        { "color", g->color().name() },
        { "dataSource", dataSourceJson },
        { "modifiers", modifiersJson },
    });
}

QByteArray ProjectFile::writeGraphData(const Graph *g)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    stream << quint32(g->pointsCount());
    for (const auto &v : g->data().xs)
        stream << v;
    for (const auto &v : g->data().ys)
        stream << v;
    return data;
}

QString ProjectFile::readProject(const QJsonObject &obj, Project *p)
{
    if (obj["version"] != PROJECT_VERSION)
        return "Unsupported project version";
    p->_nextDiagramIndex = obj["nextDiagramIndex"].toInt();
    p->_nextDiagramColorIndex = obj["nextDiagramColorIndex"].toInt();
    return {};
}

QString ProjectFile::readDiagram(const QJsonObject &obj, Diagram *d)
{
    d->_title = obj["title"].toString();
    d->_color = jsonToColor(obj["color"], Qt::red);
    return {};
}

QString ProjectFile::readGraph(const QJsonObject &obj, Graph *g)
{
    g->_title = obj["title"].toString();
    g->_autoTitle = obj["autoTitle"].toBool();
    g->_color = jsonToColor(obj["color"], Qt::red);
    g->_dataSource = new ClipboardDataSource;
    // TODO: read datasource
    // TODO: read modifiers
    return {};
}

QString ProjectFile::readGraphData(const QByteArray &data, Graph *g)
{
    QDataStream stream(data);
    quint32 pointCount;
    stream >> pointCount;
    double v;
    Values xs;
    for (quint32 i = 0; i < pointCount && !stream.atEnd(); i++) {
        stream >> v;
        xs << v;
    }
    if (xs.size() < (int)pointCount)
        return QString("Not all X values read %1 / %2.").arg(xs.size()).arg(pointCount);
    Values ys;
    for (quint32 i = 0; i < pointCount && !stream.atEnd(); i++) {
        stream >> v;
        ys << v;
    }
    if (ys.size() < (int)pointCount)
        return QString("Not all Y values read %1 / %2.").arg(ys.size()).arg(pointCount);
    g->_data = { xs, ys };
    return {};
}

namespace {

struct ZipWriter
{
    ZipWriter(const QString &fileName)
    {
        auto fn = fileName.toStdString();
        int errCode;
        zip = zip_open(fn.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errCode);
        if (!zip) {
            zip_error_t err;
            zip_error_init_with_code(&err, errCode);
            error = QString("Failed to create target file: %1").arg(zip_error_strerror(&err));
            zip_error_fini(&err);
            return;
        }
    }
    
    ~ZipWriter()
    {
        if (zip)
            zip_discard(zip);
    }
    
    bool addFile(const QString &fileName, const QJsonObject &json)
    {
        return addFile(fileName, QJsonDocument(json).toJson());
    }
    
    bool addFile(const QString &fileName, const QByteArray &data)
    {
        if (!zip)
            return false;

        // Docs say:
        // https://libzip.org/documentation/zip_source_buffer.html
        // > data must remain valid for the lifetime of the created source.
        // https://libzip.org/documentation/zip_file_add.html
        // > zip_source_free should not be called on a source after it was used successfully in a zip_file_add
        //
        // Since we don't know when zip-source is auto-freed
        // we have to keep all added data during the whole saving process
        // (which happens in zip_close)
        savingData << data;
        
        QString path = curDir.isEmpty() ? fileName : (curDir % '/' % fileName);
        zip_source_t *src = zip_source_buffer(zip, data.constData(), data.size(), 0);
        if (!src) {
            error = QString("Failed to prepare project data %1: %2").arg(path).arg(zip_error_strerror(zip_get_error(zip)));
            return false;
        }
        auto fn = path.toStdString();
        if (zip_file_add(zip, fn.c_str(), src, ZIP_FL_ENC_UTF_8) < 0) {
            error = QString("Failed to save project data %1: %2").arg(path).arg(zip_error_strerror(zip_get_error(zip)));
            return false;
        }
        return true;
    }
    
    bool save()
    {
        if (!zip)
            return false;
        if (zip_close(zip) < 0) {
            error = QString("Failed to save target file: %1").arg(zip_error_strerror(zip_get_error(zip)));
            return false;
        }
        zip = nullptr;
        return true;
    }
    
    QString error;
    QString curDir;
    QVector<QByteArray> savingData;
    zip_t *zip = nullptr;
};

} // namespace

QString ProjectFile::saveProject(const StorableData &data)
{
    ZipWriter zw(data.fileName);
    if (!zw.error.isEmpty())
        return zw.error;
        
    if (!zw.addFile(FILE_PROPS, writeProject(data.project)))
        return zw.error;
        
    auto diagrams = data.diagrams.isEmpty() ? data.project->diagrams() : data.diagrams;

    for (auto d : std::as_const(diagrams)) {
        zw.curDir = d->id();

        if (!zw.addFile(FILE_PROPS, writeDiagram(d)))
            return zw.error;
        
        if (data.formats.contains(d)) {
            if (!zw.addFile(FILE_FORMAT, data.formats[d]))
                return zw.error;
        }
        
        for (auto it = d->_graphs.cbegin(); it != d->_graphs.cend(); it++) {
            const Graph *g = it.value();

            zw.curDir = d->id() + '/' + g->id();

            if (!zw.addFile(FILE_PROPS, writeGraph(g)))
                return zw.error;
            
            if (data.formats.contains(g)) {
                if (!zw.addFile(FILE_FORMAT, data.formats[g]))
                    return zw.error;
            }

            if (!zw.addFile(FILE_DATA, writeGraphData(g)))
                return zw.error;
        }
    }

    if (!zw.save())
        return zw.error;

    return {};
}

namespace {

struct ZipReader
{
    ZipReader(const QString &fileName)
    {
        auto fn = fileName.toStdString();
        int errCode;
        zip = zip_open(fn.c_str(), ZIP_RDONLY, &errCode);
        if (!zip) {
            zip_error_t err;
            zip_error_init_with_code(&err, errCode);
            error = QString("Failed to open file: %1").arg(zip_error_strerror(&err));
            zip_error_fini(&err);
            return;
        }

        auto num = zip_get_num_entries(zip, ZIP_FL_UNCHANGED);
        for (int i = 0; i < num; i++) {
            struct zip_stat fi;
            zip_stat_init(&fi);
            if (zip_stat_index(zip, i, ZIP_FL_UNCHANGED, &fi) < 0) {
               error = QString("Failed to get info for #%1: %2").arg(i).arg(zip_error_strerror(zip_get_error(zip)));
               return;
            }
            if (!(fi.valid & ZIP_STAT_NAME)) {
               error = QString("Failed to get name for #%1: %2").arg(i).arg(zip_error_strerror(zip_get_error(zip)));
               return;
            }
            auto path = QString::fromUtf8(fi.name).split('/');
            if (path.length() >= 2) {
                // e.g. a58b3d3617c94d37a0823fe622b0cacf/props.json
                // e.g. a58b3d3617c94d37a0823fe622b0cacf/0eb5369d853b4a52909648e4e2a99f19/props.json
                QString diagramId = path.at(0);
                if (!ids.contains(diagramId))
                    ids.insert(diagramId, {});
                if (path.length() > 2) {
                    QString graphId = path.at(1);
                    if (!ids[diagramId].contains(graphId))
                        ids[diagramId].insert(graphId);
                }
            }
        }
    }
    
    ~ZipReader()
    {
        zip_discard(zip);
    }
    
    QString error;
    QHash<QString, QSet<QString>> ids;
    zip_t *zip = nullptr;
};

struct ZipFile
{
    ZipFile(zip *z, const QString &name): name(name)
    {
        struct zip_stat fi;
        zip_stat_init(&fi);
        auto fn = name.toStdString();
        if (zip_stat(z, fn.c_str(), ZIP_FL_UNCHANGED, &fi) < 0) {
            error = QString("Failed to get info for %1: %2").arg(name).arg(zip_error_strerror(zip_get_error(z)));
            return;
        }
        if (!(fi.valid & ZIP_STAT_SIZE)) {
            error = QString("Unable to get size of %1").arg(name);
        }
        zf = zip_fopen(z, fn.c_str(), ZIP_FL_UNCHANGED);
        if (!zf) {
            error = QString("Failed to open %1: %2").arg(name).arg(zip_error_strerror(zip_get_error(z)));
            return;
        }
        data = QByteArray(fi.size, 0);
        auto bytesRead = zip_fread(zf, data.data(), fi.size);
        if (bytesRead < 0) {
            error = QString("Failed to read %1: %2").arg(name).arg(zip_error_strerror(zip_file_get_error(zf)));
            return;
        }
        if (bytesRead != fi.size) {
            error = QString("Failed to read %1: expected %2 bytes but read %3 bytes").arg(name).arg(fi.size).arg(bytesRead);
            return;
        }
    }
    
    bool asJson()
    {
        QJsonParseError err;
        auto doc = QJsonDocument::fromJson(data, &err);
        if (doc.isNull()) {
            error = QString("Failed to parse %1: %2").arg(name).arg(err.errorString());
            return false;
        }
        if (!doc.isObject()) {
            error = QString("Unsupported json type of %1: is not an object").arg(name);
            return false;
        }
        json = doc.object();
        return true;
    }

    ~ZipFile()
    {
        if (zf)
            zip_fclose(zf);
    }
    
    QString error;
    QByteArray data;
    QJsonObject json;
    QString name;
    zip_file *zf = nullptr;
};

} // namespace

QString ProjectFile::loadProject(const QString &fileName, Project *project)
{
    // Loading is called on empty projects
    // Empty project contains one empty dialgram 
    // that is automatically created after app started
    for (auto it = project->_diagrams.cbegin(); it != project->_diagrams.cend(); it++)
        BusEvent::DiagramDeleted::send({{"id", it.key()}});
    qDeleteAll(project->_diagrams);
    project->_diagrams.clear();

    ZipReader zr(fileName);
    if (!zr.error.isEmpty())
        return zr.error;
        
    {
        ZipFile zf(zr.zip, FILE_PROPS);
        if (!zf.error.isEmpty())
            return zf.error;
        if (!zf.asJson())
            return zf.error;
        QString err = readProject(zf.json, project);
        if (!err.isEmpty())
            return err;
    }
    
    for (auto it = zr.ids.cbegin(); it != zr.ids.cend(); it++) {
        QString diagramId = it.key();
        std::unique_ptr<Diagram> diagram(new Diagram(project));
        {
            ZipFile zf(zr.zip, diagramId + '/' + FILE_PROPS);
            if (!zf.error.isEmpty())
                return zf.error;
            if (!zf.asJson())
                return zf.error;
            QString err = readDiagram(zf.json, diagram.get());
            if (!err.isEmpty()) {
                return QString("Failed to read diagram %1: %2").arg(diagramId, err);
            }
        }
        QJsonObject diagramFormat;
        {
            ZipFile zf(zr.zip, diagramId + '/' + FILE_FORMAT);
            if (!zf.error.isEmpty())
                return zf.error;
            if (!zf.asJson())
                return zf.error;
            diagramFormat = zf.json;
        }
        diagram->_id = diagramId;
        project->_diagrams.insert(diagramId, diagram.release());
        BusEvent::DiagramAdded::send({{"id", diagramId}});
        BusEvent::DiagramFormatLoaded::send({{"id", diagramId}, {"format", diagramFormat}});
        
        for (const QString &graphId : it.value()) {
            std::unique_ptr<Graph> graph(new Graph);
            {
                ZipFile zf(zr.zip, diagramId + '/' + graphId + '/' + FILE_PROPS);
                if (!zf.error.isEmpty())
                    return zf.error;
                if (!zf.asJson())
                    return zf.error;
                QString err = readGraph(zf.json, graph.get());
                if (!err.isEmpty())
                    return QString("Failed to read props of graph %1: %2").arg(graphId, err);
            }
            {
                ZipFile zf(zr.zip, diagramId + '/' + graphId + '/' + FILE_DATA);
                if (!zf.error.isEmpty())
                    return zf.error;
                QString err = readGraphData(zf.data, graph.get());
                if (!err.isEmpty())
                    return QString("Failed to read data of graph %1: %2").arg(graphId, err);
            }
            QJsonObject graphFormat;
            {
                ZipFile zf(zr.zip, diagramId + '/' + graphId + '/' + FILE_FORMAT);
                if (!zf.error.isEmpty())
                    return zf.error;
                if (!zf.asJson())
                    return zf.error;
                graphFormat = zf.json;
            }
            graph->_id = graphId;
            project->_diagrams[diagramId]->_graphs.insert(graphId, graph.release());
            BusEvent::GraphLoaded::send({{"id", graphId}, {"format", graphFormat}});
        }

        BusEvent::DiagramLoaded::send({{"id", diagramId}});
    }
    
    return {};
}
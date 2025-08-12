#include "Project.h"

#include "BaseTypes.h"
#include "FileUtils.h"

#include "helpers/OriDialogs.h"
#include "tools/OriMessageBus.h"
#include "tools/OriSettings.h"

#include "qcpl_colors.h"

#include <QDebug>
#include <QFileDialog>
#include <QPainter>
#include <QUuid>

using Ori::MessageBus;

//------------------------------------------------------------------------------
//                                 Project
//------------------------------------------------------------------------------


Project::Project(QObject *parent) : QObject(parent)
{
}

void Project::newDiagram()
{
    auto dia = new Diagram(this);
    dia->_id = QUuid::createUuid().toString(QUuid::Id128);
    dia->_title = tr("Diagram %1").arg(++_nextDiagramIndex);
    dia->_color = nextDiagramColor();
    _items.insert(dia->id(), dia);
    qDebug() << "Project::newDiagram" << dia->id();
    MessageBus::send((int)BusEvent::DiagramAdded, {{"id", dia->id()}});
    // The first empty diagram doesn't modify the project
    if (_items.size() > 1)
        markModified("Project::newDiagram");
}

void Project::deleteDiagram(const QString &id)
{
    auto dia = _items.value(id);
    if (!dia) {
        qWarning() << "Project::deleteDiagram: diagram not found" << id;
        return;
    }
    _items.remove(id);
    delete dia;
    MessageBus::send((int)BusEvent::DiagramDeleted, {{"id", id}});
    markModified("Project::deleteDiagram " + id);
}

Diagram* Project::diagram(const QString &id)
{
    return _items.value(id);
}

QColor Project::nextDiagramColor()
{
    if (_nextDiagramColorIndex == QCPL::defaultColorSet().size())
        _nextDiagramColorIndex = 0;
    return QCPL::defaultColorSet().at(_nextDiagramColorIndex++);
}

void Project::markModified(const QString &reason)
{
    _modified = true;
    qDebug() << "Project::modified" << reason;
    MessageBus::send((int)BusEvent::ProjectModified);
}

QString Project::getSaveFileName()
{
    Ori::Settings s;
    s.beginGroup("Recent");

    QString recentPath = s.strValue("prj_save_path");
    QString recentFilter = s.strValue("prj_save_filter");

    auto fileName = QFileDialog::getSaveFileName(qobject_cast<QWidget*>(parent()),
                                                 tr("Save Project", "Dialog title"),
                                                 recentPath,
                                                 FileUtils::filtersForSave(),
                                                 &recentFilter);
    if (fileName.isEmpty()) return QString();

    s.setValue("prj_save_path", fileName);
    s.setValue("prj_save_filter", recentFilter);

    return FileUtils::refineFileName(fileName, recentFilter);
}

//------------------------------------------------------------------------------
//                                 Diagram
//------------------------------------------------------------------------------

Diagram::Diagram(Project *project): QObject(project), _prj(project)
{
}

static QIcon makeGraphIcon(QColor color)
{
    int H, S, L;
    color.getHsl(&H, &S, &L);
    QColor backColor = QColor::fromHsl(H, int(float(S)*0.8f), int(float(L)*1.2f));
    QColor borderColor = QColor::fromHsl(H, int(float(S)*0.5f), L);

    QPixmap px(16, 16);
    px.fill(Qt::transparent);

    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing, true);

    QPen borderPen(borderColor);
    borderPen.setWidthF(1.5);
    p.setPen(borderPen);

    p.setBrush(backColor);
    p.drawEllipse(px.rect().adjusted(1, 1, -1, -1));

    // TODO draw gradient gloss

    return QIcon(px);
}

const QIcon& Diagram::icon()
{
    if (_icon.isNull())
        _icon = makeGraphIcon(_color);
    return _icon;
}

void Diagram::markModified(const QString &reason)
{
    _prj->markModified(reason);
}

void Diagram::doRename()
{
    QString newTitle = Ori::Dlg::inputText(tr("Diagram title:"), _title);
    if (newTitle.isEmpty() || newTitle == _title) return;
    _title = newTitle;
    MessageBus::send((int)BusEvent::DiagramRenamed, {{"id", _id}});
    _prj->markModified("Diagram::doRename");
}

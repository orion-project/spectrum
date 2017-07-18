#include "Operations.h"
#include "core/GraphBuilder.h"
#include "helpers/OriDialogs.h"

#include <QApplication>
#include <QClipboard>

Operations::Operations(QObject *parent) : QObject(parent)
{

}

void Operations::makeRandomSample() const
{
    auto g = GraphBuilder::makeRandomSample();
    if (!g) return;

    emit graphCreated(g);
}

void Operations::makeGraphFromClipboard() const
{
    QString text = qApp->clipboard()->text();
    if (text.isEmpty())
        return Ori::Dlg::info(tr("Clipboard does not contain suitable data."));

    auto g = GraphBuilder::makeFromTextData(text);
    if (!g) return;
}

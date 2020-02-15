#include "DataExporters.h"

#include <QApplication>
#include <QClipboard>
#include <QTextStream>

namespace DataExporters
{

void copyToClipboard(const GraphPoints& data)
{
    QString text;
    QTextStream stream(&text);
    int count = data.xs.size();
    for (int i = 0; i < count; i++)
        stream << QString::number(data.xs.at(i), 'g', 10)
               << '\t'
               << QString::number(data.ys.at(i), 'g', 10)
               << '\n';
    qApp->clipboard()->setText(text);
}

} // namespace DataExporters

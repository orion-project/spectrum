#ifndef FuncPlotTextFile_H
#define FuncPlotTextFile_H

#include "FuncPlotText.h"

class FuncPlotTextFile : public FuncPlotText
{
public:
    FuncPlotTextFile(const QString& fileName) :
        FuncPlotText(QString()), _fileName(fileName) {}

    bool process() override;

private:
    QString _fileName;
};

#endif // FuncPlotTextFile_H

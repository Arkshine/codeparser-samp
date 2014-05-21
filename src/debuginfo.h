#ifndef DEBUGINFO_H
#define DEBUGINFO_H

#include <QWidget>

#include <set>
#include <string>

#include "codeparser.h"

namespace Ui {
class DebugInfo;
}

class DebugInfo : public QWidget
{
    Q_OBJECT

public:
    explicit DebugInfo(QWidget *parent = 0);
    ~DebugInfo();

    DebugInfo *setFunctions(std::set<CodeParser::Function> functions);
    DebugInfo *setDefinitions(std::set<std::string> definitions);

    DebugInfo *setSkippedFunctions(QString skipped);
    DebugInfo *setSkippedDefinitions(QString skipped);

private:
    Ui::DebugInfo *ui;
};

#endif // DEBUGINFO_H

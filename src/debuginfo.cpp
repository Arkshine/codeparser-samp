#include "debuginfo.h"
#include "ui_debuginfo.h"

DebugInfo::DebugInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugInfo)
{
    ui->setupUi(this);
}

DebugInfo::~DebugInfo()
{
    delete ui;
}

DebugInfo *DebugInfo::setDefinitions(std::set<std::string> definitions)
{
    QString definitions_str;

    for(std::set<std::string>::iterator it = definitions.begin(); it != definitions.end(); ++it)
    {
        definitions_str.append(QString::fromStdString(*it + "\n"));
    }

    ui->definitionsText->setText(definitions_str);
    ui->definitionCountText->setText(QString("Total: %1").arg(definitions.size()));
    return this;
}

DebugInfo *DebugInfo::setFunctions(std::set<CodeParser::Function> functions)
{
    QString functions_str;

    for(std::set<CodeParser::Function>::iterator it = functions.begin(); it != functions.end(); ++it)
    {
        functions_str.append(QString::fromStdString((*it).function) + "\n");
    }

    ui->functionsText->setText(functions_str);
    ui->functionCountLabel->setText(QString("Total: %1").arg(functions.size()));
    return this;
}

DebugInfo *DebugInfo::setSkippedFunctions(QString skipped)
{
    int count = skipped.count("\n");
    ui->skippedFunctionsText->setText(skipped);
    ui->skippedFunctionsCountLabel->setText(QString("Total: %1").arg(count));
    return this;
}

DebugInfo *DebugInfo::setSkippedDefinitions(QString skipped)
{
    int count = skipped.count("\n");
    ui->skippedDefinitionsText->setText(skipped);
    ui->skippedDefinitionsCountText->setText(QString("Total: %1").arg(count));
    return this;
}

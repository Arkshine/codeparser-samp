#ifndef CODEPARSER_H
#define CODEPARSER_H

#include <QString>
#include <QProcess>
#include <QPair>

#include <string>
#include <vector>

class CodeParser
{
public:
    CodeParser();

    struct Function
    {
        std::string type;
        std::string return_type;
        std::string function;
        std::vector<std::string> params;
    };

    QString runPreprocessor();
    QString parse(QString path);

    CodeParser *setSourceFile(QString path);
    CodeParser *setPawnccFile(QString path);
    CodeParser *setSublimeOutputFile(QString path);
    CodeParser *setNotepadUserDefFile(QString path);
    CodeParser *setNotepadPawnFile(QString path);

    int getParsedLineCount();

    QString getSourceFile();
    QString getPawnccFile();
    QString getSublimeOutputFile();
    QString getNotepadUserDefFile();
    QString getNotepadPawnFile();

    std::vector<Function> getFunctions();
    QString getSublimeOutput();
    QPair<QString, QString> getNotepadOutput();

    static QString limitStringLengthFromEnd(QString string, int limit);

private:
    QString source_file_path;
    QString pawncc_file_path;
    QString sublime_output_path;
    QString notepad_user_def_path;
    QString notepad_pawn_path;

    int lines_parsed = 0;
    std::vector<Function> functions;

private slots:
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // CODEPARSER_H

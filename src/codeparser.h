#ifndef CODEPARSER_H
#define CODEPARSER_H

#include <QString>
#include <QProcess>
#include <QPair>
#include <QSet>
#include <QVector>

#include <string>
#include <vector>
#include <set>

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

    void combineFiles(QString path);
    void parse();
    void reset();

    CodeParser *setSourceFile(QString path);
    CodeParser *setPawnccFile(QString path);
    CodeParser *setSublimeOutputFile(QString path);
    CodeParser *setNotepadUserDefFile(QString path);
    CodeParser *setNotepadPawnFile(QString path);

    QString getSourceFile();
    QString getPawnccFile();
    QString getSublimeOutputFile();
    QString getNotepadUserDefFile();
    QString getNotepadPawnFile();

    std::set<Function> getFunctions();
    std::set<std::string> getDefinitions();

    QString getSkippedFunctions();
    QString getSkippedDefinitions();

    QString getSublimeOutput();
    QPair<QString, QString> getNotepadOutput();

    int getParsedLineCount();

    static QString limitStringLengthFromEnd(QString string, int limit);

private:
    QString source_file_path;
    QString pawncc_file_path;
    QString sublime_output_path;
    QString notepad_user_def_path;
    QString notepad_pawn_path;

    std::vector<std::string> combined_output;
    QSet<QString> combined_files;

    int lines_parsed = 0;

    std::set<Function> functions;
    std::set<std::string> definitions;

    QString skipped_functions;
    QString skipped_definitions;

    friend bool operator<(const Function &lhs, const Function &rhs);
};

#endif // CODEPARSER_H

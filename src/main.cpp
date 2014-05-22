#include <QApplication>
#include <QMessageBox>
#include <QStringList>
#include <QMap>
#include <QFile>

#include "mainwindow.h"
#include "codeparser.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QStringList arguments_raw = a.arguments();
    if(arguments_raw.contains("--noGui"))
    {
        // Get the arguments into a map, splitting by an equal sign
        QMap<QString, QString> arguments;
        for(QStringList::iterator it = arguments_raw.begin(); it != arguments_raw.end(); ++it)
        {
            QStringList arg_list = it->split("=");
            if(arg_list.count() != 2)
                continue;

            arguments.insert(arg_list.at(0), arg_list.at(1));
        }

        QString type = arguments.value("--type", "");
        QString pawncc = arguments.value("--pawncc", "");
        QString source = arguments.value("--in", "");

        if(type == "sublime")
        {
            QString outfile = arguments.value("--out");

            if(outfile.isEmpty() || pawncc.isEmpty() || source.isEmpty())
            {
                QMessageBox error;
                error.setIcon(QMessageBox::Warning);
                error.setText("Please define --pawncc, --in and --out paths");
                error.exec();
                return a.exit();
            }

            CodeParser *parser = new CodeParser();
            parser
                    ->setPawnccFile(pawncc)
                    ->setSourceFile(source)
                    ->setSublimeOutputFile(outfile);

            QFile output_file(parser->getSublimeOutputFile());

            if(!output_file.open(QIODevice::WriteOnly))
            {
                QMessageBox alert;
                alert.setIcon(QMessageBox::Warning);
                alert.setWindowTitle("Save configuration");
                alert.setText("The completion file for Sublime Text is not writable.");
                alert.exec();
                return a.exit();
            }

            parser->reset();
            parser->combineFiles(parser->getSourceFile());
            parser->parse();

            output_file.write(parser->getSublimeOutput().toUtf8().constData());
            output_file.close();

            delete parser;
        }
        else if(type == "notepad")
        {
            QString pawnFile = arguments.value("--pawnFile", "");
            QString userDefFile = arguments.value("--userDefFile", "");

            if(pawnFile.isEmpty() || userDefFile.isEmpty() || pawncc.isEmpty() || source.isEmpty())
            {
                QMessageBox error;
                error.setIcon(QMessageBox::Warning);
                error.setText("Please define --pawncc, --in, --pawnFile and --userDefFile paths");
                error.exec();
                return a.exit();
            }

            CodeParser *parser = new CodeParser();
            parser
                    ->setPawnccFile(pawncc)
                    ->setSourceFile(source)
                    ->setNotepadPawnFile(pawnFile)
                    ->setNotepadUserDefFile(userDefFile);

            QFile pawn_file(parser->getNotepadPawnFile());
            if(!pawn_file.open(QIODevice::WriteOnly))
            {
                QMessageBox alert;
                alert.setIcon(QMessageBox::Warning);
                alert.setWindowTitle("Save configuration");
                alert.setText("The pawn language file is not writable. Make sure you have sufficient permissions to write to the file.");
                alert.exec();
                return a.exit();
            }

            QFile user_def_file(parser->getNotepadUserDefFile());
            if(!user_def_file.open(QIODevice::WriteOnly))
            {
                QMessageBox alert;
                alert.setIcon(QMessageBox::Warning);
                alert.setWindowTitle("Save configuration");
                alert.setText("The userDefLang file is not writeable. Make sure you have sufficient permissions to write to the file.");
                alert.exec();
                return a.exit();
            }

            parser->reset();
            parser->combineFiles(parser->getSourceFile());
            parser->parse();

            QString pawn_string = parser->getNotepadOutput().first;
            QString user_def_string = parser->getNotepadOutput().second;

            pawn_file.write(pawn_string.toUtf8().constData());
            user_def_file.write(user_def_string.toUtf8().constData());

            pawn_file.close();
            user_def_file.close();
        }

        return a.exit();
    }
    else
    {
        MainWindow w;
        w.show();
        return a.exec();
    }
}

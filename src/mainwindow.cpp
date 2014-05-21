#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>

#include <sstream>
#include <iostream>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->hide();
    this->resize(640, 0);

    parser = new CodeParser();
    config = new QSettings("codeparser.config.ini", QSettings::IniFormat);

    if(config->value("source_file").toString().length())
    {
        parser->setSourceFile(config->value("source_file").toString());
        ui->sourceFileLabel->setText(CodeParser::limitStringLengthFromEnd(parser->getSourceFile(), 60));
    }

    if(config->value("pawncc_path").toString().length())
    {
        parser->setPawnccFile(config->value("pawncc_path").toString());
        ui->pawnccFileLabel->setText(CodeParser::limitStringLengthFromEnd(parser->getPawnccFile(), 60));
    }

    if(config->value("Sublime/output_file").toString().length())
    {
        parser->setSublimeOutputFile(config->value("Sublime/output_file").toString());
        ui->sublimeOutputLabel->setText(CodeParser::limitStringLengthFromEnd(parser->getSublimeOutputFile(), 60));
    }

    if(config->value("Notepad/userDefLang_file").toString().length())
    {
        parser->setNotepadUserDefFile(config->value("Notepad/userDefLang_file").toString());
        ui->notepadUserDefLabel->setText(CodeParser::limitStringLengthFromEnd(parser->getNotepadUserDefFile(), 60));
    }

    if(config->value("Notepad/pawn_file").toString().length())
    {
        parser->setNotepadPawnFile(config->value("Notepad/pawn_file").toString());
        ui->notepadPawnLabel->setText(CodeParser::limitStringLengthFromEnd(parser->getNotepadPawnFile(), 60));
    }
}

MainWindow::~MainWindow()
{
    delete debug_info;
    delete parser;
    delete config;
    delete ui;
}

void MainWindow::on_sourceBrowseBtn_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Pawn source files (*.pwn *.inc)"));
    dialog.setDirectory(QDir::homePath());

    QStringList file_names;
    if(dialog.exec())
    {
        file_names = dialog.selectedFiles();
        parser->setSourceFile(file_names.takeFirst());
        ui->sourceFileLabel->setText(CodeParser::limitStringLengthFromEnd(parser->getSourceFile(), 60));
    }
}

void MainWindow::on_pawnccBrowseBtn_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Pawn compiler (pawncc.exe pawncc)"));
    dialog.setDirectory(QDir::homePath());

    QStringList file_names;
    if(dialog.exec())
    {
        file_names = dialog.selectedFiles();
        parser->setPawnccFile(file_names.takeFirst());
        ui->pawnccFileLabel->setText(CodeParser::limitStringLengthFromEnd(parser->getPawnccFile(), 60));
    }
}

void MainWindow::on_sublimeOutputBtn_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("*.sublime-completions"));
    dialog.setDirectory(QDir::homePath());

    QStringList file_names;
    if(dialog.exec())
    {
        file_names = dialog.selectedFiles();
        parser->setSublimeOutputFile(file_names.takeFirst());
        ui->sublimeOutputLabel->setText(CodeParser::limitStringLengthFromEnd(parser->getSublimeOutputFile(), 60));
    }
}

void MainWindow::on_notepadUserDefBtn_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("userDefineLang.xml"));
    dialog.setDirectory(QDir::homePath());

    QStringList file_names;
    if(dialog.exec())
    {
        file_names = dialog.selectedFiles();
        parser->setNotepadUserDefFile(file_names.takeFirst());
        ui->notepadUserDefLabel->setText(CodeParser::limitStringLengthFromEnd(parser->getNotepadUserDefFile(), 60));
    }
}

void MainWindow::on_notepadPawnBtn_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("*.xml"));
    dialog.setDirectory(QDir::homePath());

    QStringList file_names;
    if(dialog.exec())
    {
        file_names = dialog.selectedFiles();
        parser->setNotepadPawnFile(file_names.takeFirst());
        ui->notepadPawnLabel->setText(CodeParser::limitStringLengthFromEnd(parser->getNotepadPawnFile(), 60));
    }
}

void MainWindow::on_saveConfigBtn_clicked()
{
    if(config->isWritable())
    {
        config->setValue("source_file", parser->getSourceFile());
        config->setValue("pawncc_path", parser->getPawnccFile());
        config->setValue("Sublime/output_file", parser->getSublimeOutputFile());
        config->setValue("Notepad/userDefLang_file", parser->getNotepadUserDefFile());
        config->setValue("Notepad/pawn_file", parser->getNotepadPawnFile());

        QMessageBox info;
        info.setIcon(QMessageBox::Information);
        info.setWindowTitle("Save configuration");
        info.setText("The configuration was successfully saved.");
        info.exec();
    }
    else
    {
        QMessageBox alert;
        alert.setIcon(QMessageBox::Warning);
        alert.setWindowTitle("Save configuration");
        alert.setText("Cannot write to the configuration file.");
        alert.exec();
    }
}

void MainWindow::on_generateSublimeBtn_clicked()
{
    QFile output_file(parser->getSublimeOutputFile());

    if(!output_file.open(QIODevice::WriteOnly))
    {
        QMessageBox alert;
        alert.setIcon(QMessageBox::Warning);
        alert.setWindowTitle("Save configuration");
        alert.setText("The completion file for Sublime Text is not writable.");
        alert.exec();
        return;
    }

    ui->statusBar->show();
    ui->statusBar->showMessage("Parsing...");

    parser->reset();
    parser->combineFiles(parser->getSourceFile());
    parser->parse();

    std::stringstream ss;
    ss << "Parsed " << parser->getParsedLineCount() << " lines, found " << parser->getFunctions().size() << " functions and " << parser->getDefinitions().size() << " definitions.";
    ui->statusBar->showMessage(QString::fromStdString(ss.str()));

    if(ui->showDetailedResults->isChecked())
    {
        debug_info = new DebugInfo();
        debug_info
                ->setDefinitions(parser->getDefinitions())
                ->setFunctions(parser->getFunctions())
                ->setSkippedDefinitions(parser->getSkippedDefinitions())
                ->setSkippedFunctions(parser->getSkippedFunctions());

        debug_info->show();
    }

    output_file.write(parser->getSublimeOutput().toUtf8().constData());
    output_file.close();
}

void MainWindow::on_generateNotepadBtn_clicked()
{
    QFile pawn_file(parser->getNotepadPawnFile());

    if(!pawn_file.open(QIODevice::WriteOnly))
    {
        QMessageBox alert;
        alert.setIcon(QMessageBox::Warning);
        alert.setWindowTitle("Save configuration");
        alert.setText("The pawn language file is not writable. Make sure you have sufficient permissions to write to the file.");
        alert.exec();
        return;
    }

    QFile user_def_file(parser->getNotepadUserDefFile());

    if(!user_def_file.open(QIODevice::WriteOnly))
    {
        QMessageBox alert;
        alert.setIcon(QMessageBox::Warning);
        alert.setWindowTitle("Save configuration");
        alert.setText("The userDefLang file is not writeable. Make sure you have sufficient permissions to write to the file.");
        alert.exec();
        return;
    }

    ui->statusBar->show();
    ui->statusBar->showMessage("Parsing...");

    parser->reset();
    parser->combineFiles(parser->getSourceFile());
    parser->parse();

    std::stringstream ss;
    ss << "Parsed " << parser->getParsedLineCount() << " lines, found " << parser->getFunctions().size() << " functions and " << parser->getDefinitions().size() << " definitions.";
    ui->statusBar->showMessage(QString::fromStdString(ss.str()));

    if(ui->showDetailedResults->isChecked())
    {
        debug_info = new DebugInfo();
        debug_info
                ->setDefinitions(parser->getDefinitions())
                ->setFunctions(parser->getFunctions())
                ->setSkippedDefinitions(parser->getSkippedDefinitions())
                ->setSkippedFunctions(parser->getSkippedFunctions());

        debug_info->show();
    }

    QString pawn_string = parser->getNotepadOutput().first;
    QString user_def_string = parser->getNotepadOutput().second;

    pawn_file.write(pawn_string.toUtf8().constData());
    user_def_file.write(user_def_string.toUtf8().constData());

    pawn_file.close();
    user_def_file.close();
}

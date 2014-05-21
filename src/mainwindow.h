#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

#include "codeparser.h"
#include "debuginfo.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_sourceBrowseBtn_clicked();

    void on_pawnccBrowseBtn_clicked();

    void on_saveConfigBtn_clicked();

    void on_generateSublimeBtn_clicked();

    void on_generateNotepadBtn_clicked();

    void on_sublimeOutputBtn_clicked();

    void on_notepadUserDefBtn_clicked();

    void on_notepadPawnBtn_clicked();

private:
    Ui::MainWindow *ui;
    CodeParser *parser;
    QSettings *config;

    DebugInfo *debug_info;
};

#endif // MAINWINDOW_H

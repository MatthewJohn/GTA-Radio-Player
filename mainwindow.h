#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDirIterator>
#include <QDebug>
#include <QMediaPlayer>

#define MAX_STATIONS 20

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    bool playing;
    QMediaPlayer *player;
    // List of stations
    QString stationFiles[MAX_STATIONS];
    int stationFileCount;
    void PopulateFileList();

    // Index of current stations
    int currentStation;
};
#endif // MAINWINDOW_H

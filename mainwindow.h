#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDirIterator>
#include <QDebug>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QDial>

#define MAX_STATIONS 20
#define INITIAL_VOLUME 50

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

    // Playing status
    bool playing;

    // Index of current stations
    int currentStation;

    // player object
    QMediaPlayer *player;

    // List of stations
    QString stationFiles[MAX_STATIONS];
    int stationFileCount;

    // Populate list of station files
    void PopulateFileList();

    // Play
    void Play();
    void Pause();
    void SetVolume(qint64 new_volume);
    bool IsPlayAvailable();
    void SelectStation(int station_index);
    void NextStation();
    void PreviousStation();

    QDial* GetVolumeDial();

    void DisplayError(QString err);

};
#endif // MAINWINDOW_H

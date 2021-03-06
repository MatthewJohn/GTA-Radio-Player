#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDirIterator>
#include <QDebug>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QMessageBox>
#include <QDial>
#include <QDateTime>
#include <QPushButton>
#include <QTextBrowser>
#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QSettings>

#define MAX_STATIONS 20
#define INITIAL_VOLUME 40
#define STATION_CHANGE_DRAMATIC_PAUSE_DURATION 300
#define MEDIA_LOAD_WAIT_PERIOD 100
#define PLAY_PAUSE_BUTTON_TEXT_PLAY "Play"
#define PLAY_PAUSE_BUTTON_TEXT_PAUSE "Pause"
#define SETTINGS_KEY_VOLUME "player/volume"
#define SETTINGS_KEY_DIRECTORY "player/directory"
#define SETTINGS_KEY_ALWAYS_ON_TOP "window/always_on_top"
#define SETTINGS_KEY_START_EPOC "player/start_epoc"
#define ORGANISATION "MatthewJohn"
#define APP_NAME "GTA Radio Player"
#define DEFAULT_ALWAYS_ON_TOP 0

#ifdef _WIN32
#define INITIAL_DIRECTORY "."
#else
#define INITIAL_DIRECTORY "./"
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    // Slots for controls
    void PlayPauseButtonSlot();
    void NextStation();
    void PreviousStation();
    void VolumeDialChangeSlot();
    // Slots for menu items
    void OpenChangeDirectory();
    void ToggleAlwaysOnTop(bool new_value);

private:
    Ui::MainWindow *ui;

    // Playing status
    bool playing;

    // Index of current stations
    int currentStation;

    // Menu items
    QMenuBar *menu_bar;
    QMenu *file_menu;
    QAction *change_directory_action;
    QAction *always_on_top_action;

    // Settings
    QSettings *settings;

    // player object
    QMediaPlayer *players[2];
    int currentPlayerItx;
    QMediaPlayer* GetCurrentPlayer();
    QMediaPlayer* GetNextPlayer();
    void FlipPlayer();

    // List of stations
    QString stationFiles[MAX_STATIONS];
    int stationFileCount;
    // Directory to scan for MP3s
    QString scan_directory;

    // Populate list of station files
    void PopulateFileList();
    void DisablePlayer();
    void UpdateDirectory(QString new_directory);

    qint64 startupTime;

    // Play
    void Play();
    void Pause();
    void SetVolume(qint64 new_volume);
    bool IsPlayAvailable();
    bool IsPlaying();
    void SelectStation(int station_index);
    QString GetMediaName();

    void SetDisplay(QString text);

    QDial* GetVolumeDial();
    QPushButton* GetPlayPauseButton();
    QPushButton* GetNextButton();
    QPushButton* GetPreviousButton();

    void DisplayError(QString err);
    void DisplayInfo(QString info);

};
#endif // MAINWINDOW_H

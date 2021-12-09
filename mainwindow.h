#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>

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
#include <QTimer>

#include "player.h"

#define MAX_STATIONS 20
#define INITIAL_VOLUME 40
#define STATION_CHANGE_DRAMATIC_PAUSE_DURATION 300
#define MEDIA_LOAD_WAIT_PERIOD 100
#define MUTE_BUTTON_TEXT_MUTE "Mute"
#define MUTE_BUTTON_TEXT_UNMUTE "Unmute"
#define SETTINGS_KEY_VOLUME "player/volume"
#define SETTINGS_KEY_DIRECTORY "player/directory"
#define SETTINGS_KEY_ALWAYS_ON_TOP "window/always_on_top"
#define SETTINGS_KEY_START_EPOC "player/start_epoc"
#define SETTINGS_KEY_CURRENT_STATION_INDEX "player/station_index"
#define SETTINGS_KEY_THEME "player/theme"
#define ORGANISATION "MatthewJohn"
#define APP_NAME "GTA Radio Player"
#define DEFAULT_ALWAYS_ON_TOP 0

#define THEME_VICE "VICE"
#define THEME_SA "SA"
#define THEME_DEFAULT THEME_VICE

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

    QLabel* GetPositionLabel();
    qint64 GetStartupTime();
    void DisplayError(QString err);

public slots:
    // Slots for controls
    void MuteButtonSlot();
    void NextStation();
    void PreviousStation();
    void VolumeDialChangeSlot();
    void PlayPauseButtonSlot();
    // Slots for menu items
    void OpenChangeDirectory();
    void ResetGlobalTimer();
    void ToggleAlwaysOnTop(bool new_value);
    void ViceThemeSelectSlot();
    void SaThemeSelectSlot();

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
    QAction *reset_global_timer;
    QMenu* theme_menu;
    QAction* vice_theme_action;
    QAction* sa_theme_action;

    // Settings
    QSettings *settings;

    // player object
    Player *players[2];
    int currentPlayerItx;
    Player* GetCurrentPlayer();
    Player* GetNextPlayer();

    // List of stations
    QString stationFiles[MAX_STATIONS];
    int stationFileCount;
    // Directory to scan for MP3s
    QString scan_directory;

    // Populate list of station files
    void PopulateFileList();
    void DisablePlayer();
    void UpdateDirectory(QString new_directory, int station_index);

    qint64 startupTime;
    void SetStartupTime(bool force_reset, qint64 new_time);

    // Play
    bool is_playing;
    qint64 pause_time;
    void Play();
    void Pause();
    void SetMute(bool muted);
    void SetVolume(qint64 new_volume);
    bool IsPlayAvailable();
    bool IsPlaying();
    void SelectStation(int station_index);
    QString GetMediaName();

    int LoadCurrentStation();
    void SaveCurrentStation();

    void DisableMediaButtons();
    void EnableMediaButtons();

    void DisableMediaInterupts();
    void EnableMediaInterupts();

    void SetTheme(QString theme_name);
    void UpdateUiTheme(QString theme_name);

    QDial* GetVolumeDial();
    QPushButton* GetMuteButton();
    QPushButton* GetPlayPauseButton();
    QPushButton* GetNextButton();
    QPushButton* GetPreviousButton();
    QLabel* GetDisplay();
    QWidget* GetDisplayBackgroundWidget();
    QWidget* GetBackgroundWidget();

    void SetDisplay(QString text);
    void DisplayInfo(QString info);

};
#endif // MAINWINDOW_H

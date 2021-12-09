#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QCoreApplication::setApplicationName("GTA Radio Player");
    this->setWindowTitle("GTA Radio Player");

    this->settings = new QSettings(ORGANISATION, APP_NAME);

    // Obtain config for 'always on top' and, if set, enable QT
    // window flag for always on top
    bool always_on_top_set = this->settings->value(SETTINGS_KEY_ALWAYS_ON_TOP, DEFAULT_ALWAYS_ON_TOP).toInt() == 1;
    if (always_on_top_set)
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);


    // Create player objects and set current
    // player index to first player
    this->players[0] = new Player;
    this->players[0]->Setup(this, 1);
    this->players[1] = new Player;
    this->players[1]->Setup(this, 2);
    this->currentPlayerItx = 0;
    this->pause_time = 0;
    this->is_playing = false;

    this->GetVolumeDial()->setValue(this->settings->value(SETTINGS_KEY_VOLUME, INITIAL_VOLUME).toInt());
    this->VolumeDialChangeSlot();

    // Set startup time
    this->SetStartupTime(false, 0);

    this->change_directory_action = new QAction(0);
    this->change_directory_action->setText("Change Directory");

    this->reset_global_timer = new QAction(0);
    this->reset_global_timer->setText("Reset global timer");

    this->always_on_top_action = new QAction(0);
    this->always_on_top_action->setText("Always on top");
    this->always_on_top_action->setCheckable(true);
    this->always_on_top_action->setChecked(always_on_top_set);

    this->file_menu = new QMenu();
    this->file_menu->setTitle("File");
    this->file_menu->addAction(this->change_directory_action);
    this->file_menu->addAction(this->reset_global_timer);
    this->file_menu->addAction(this->always_on_top_action);

    this->vice_theme_action = new QAction(0);
    this->vice_theme_action->setText("Vice City");
    this->vice_theme_action->setCheckable(true);

    this->sa_theme_action = new QAction(0);
    this->sa_theme_action->setText("San Andreas");
    this->sa_theme_action->setCheckable(true);

    this->theme_menu = new QMenu();
    this->theme_menu->setTitle("Theme");
    this->theme_menu->addAction(this->vice_theme_action);
    this->theme_menu->addAction(this->sa_theme_action);

    this->menu_bar = new QMenuBar(0);
    this->menu_bar->setNativeMenuBar(false);
    this->menu_bar->addMenu(this->file_menu);
    this->menu_bar->addMenu(this->theme_menu);
    this->setMenuBar(this->menu_bar);

    // Update UI theme to saved value (or default).
    // Must be performed after menu setup, as it select/de-selects
    // the menu actions.
    this->UpdateUiTheme(this->settings->value(SETTINGS_KEY_THEME, THEME_DEFAULT).toString());

    // Bind knobs
    QObject::connect(this->GetMuteButton(), SIGNAL(clicked()), this, SLOT(MuteButtonSlot()));
    QObject::connect(this->GetPreviousButton(), SIGNAL(clicked()), this, SLOT(NextStation()));
    QObject::connect(this->GetNextButton(), SIGNAL(clicked()), this, SLOT(PreviousStation()));
    QObject::connect(this->GetVolumeDial(), SIGNAL(valueChanged(int)), this, SLOT(VolumeDialChangeSlot()));
    QObject::connect(this->GetPlayPauseButton(), SIGNAL(clicked()), this, SLOT(PlayPauseButtonSlot()));

    // Menu item
    QObject::connect(this->change_directory_action, SIGNAL(triggered(bool)), this, SLOT(OpenChangeDirectory()));
    QObject::connect(this->reset_global_timer, SIGNAL(triggered(bool)), this, SLOT(ResetGlobalTimer()));
    QObject::connect(this->always_on_top_action, SIGNAL(toggled(bool)), this, SLOT(ToggleAlwaysOnTop(bool)));
    QObject::connect(this->vice_theme_action, SIGNAL(triggered(bool)), this, SLOT(ViceThemeSelectSlot()));
    QObject::connect(this->sa_theme_action, SIGNAL(triggered(bool)), this, SLOT(SaThemeSelectSlot()));

    // Select initial station.
    // This must be done after initial startup as MediaPlayer objects do not full function till
    // application is running (e.g. get duration of song).
    this->UpdateDirectory(
        this->settings->value(SETTINGS_KEY_DIRECTORY, INITIAL_DIRECTORY).toString(),
        this->LoadCurrentStation());
    this->Play();
}

void MainWindow::PlayPauseButtonSlot() {
    if (this->IsPlaying()) {
        this->Pause();
    } else {
        this->Play();
    }
}

void MainWindow::ViceThemeSelectSlot()
{
    this->SetTheme(THEME_VICE);
}
void MainWindow::SaThemeSelectSlot()
{
    this->SetTheme(THEME_SA);
}

void MainWindow::SetTheme(QString theme_name)
{
    // Update theme in settings.
    this->settings->setValue(SETTINGS_KEY_THEME, theme_name);
    this->UpdateUiTheme(theme_name);
}

void MainWindow::UpdateUiTheme(QString theme_name)
{
    std::cout << "Setting theme to: " << theme_name.toStdString() << std::endl;

    // Deselect all UI Theme buttons
    this->vice_theme_action->setChecked(false);
    this->sa_theme_action->setChecked(false);

    if (theme_name.toStdString() == THEME_VICE)
    {
        this->vice_theme_action->setChecked(true);

        // Set background colour of display label
        this->GetDisplay()->setStyleSheet("QLabel {"
                                            "background-color: #000012;"
                                            "margin: 1px;"
                                            "color: #ff4df0;"
                                          "}");
        this->GetPositionLabel()->setStyleSheet("QLabel {"
                                          "background-color: #000012;"
                                          "margin: 1px;"
                                          "color: #ff4df0;"
                                          "}");
        this->GetDisplayBackgroundWidget()->setStyleSheet("QWidget {"
                                                            "background-color: #000012;"
                                                          "}");
        this->GetBackgroundWidget()->setStyleSheet(
            "QWidget {"
              "background-color: #1d269b;"
            "}"
            "QPushButton {"
              "background-color: #9d4dff;"
              "color: #70ffdf;"
            "}"
            "QDial {"
              "background-color: #ff4df0;"
            "}"
        );
    }
    else if (theme_name.toStdString() == THEME_SA)
    {
        this->sa_theme_action->setChecked(true);

        // Set background colour of display label
        this->GetDisplay()->setStyleSheet("QLabel {"
                                            "background-color: #000000;"
                                            "margin: 1px;"
                                            "color: #20d633;"
                                          "}");
        this->GetPositionLabel()->setStyleSheet("QLabel {"
                                          "background-color: #000012;"
                                          "margin: 1px;"
                                          "color: #20d633;"
                                          "}");
        this->GetDisplayBackgroundWidget()->setStyleSheet("QWidget {"
                                                            "background-color: #000012;"
                                                          "}");
        this->GetBackgroundWidget()->setStyleSheet(
            "QWidget {"
              "background-color: #000000;"
            "}"
            "QPushButton {"
              "background-color: #000000;"
              "color: #eeeeee;"
    //          "border-style: solid;"
    //          "border-width: 1px;"
    //          "border-color: red;"
            "}"
            "QDial {"
              "background-color: #000000;"
            "}"
        );
    } else {
        this->DisplayError("Unkown theme");
    }
}


qint64 MainWindow::GetStartupTime()
{
    // If currently paused and pause time has been set,
    // report startup time with offset since start of pause.
    if (! this->IsPlaying() && this->pause_time != 0)
        return this->startupTime + (QDateTime::currentMSecsSinceEpoch() - this->pause_time);
    return this->startupTime;
}

void MainWindow::UpdateDirectory(QString new_directory, int station_index)
{
    this->settings->setValue(SETTINGS_KEY_DIRECTORY, new_directory);
    this->scan_directory = new_directory;
    this->PopulateFileList();

    if (station_index >= this->stationFileCount)
    {
        std::cout << "Warning: Requested station index: " << station_index <<
                     " new availabe. Stations found: " << this->stationFileCount << std::endl;
        station_index = 0;
    }

    if (this->IsPlayAvailable())
        this->SelectStation(station_index);
    else
        this->DisablePlayer();
}

void MainWindow::DisablePlayer()
{
    this->SetDisplay("No tracks found...");
    this->Pause();
    this->DisableMediaButtons();
}

void MainWindow::Pause()
{
    this->GetPlayPauseButton()->setText("Play");
    this->is_playing = false;
    this->pause_time = QDateTime::currentMSecsSinceEpoch();
    this->GetCurrentPlayer()->Pause();
}

void MainWindow::OpenChangeDirectory()
{
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.selectFile(this->scan_directory);
    dialog.exec();
    QStringList selected = dialog.selectedFiles();
    if (selected.count())
    {
        this->UpdateDirectory(selected[0], 0);
    }
}

void MainWindow::ResetGlobalTimer()
{
    this->SetStartupTime(true, 0);

    // Restart current station
    if (! this->IsPlayAvailable())
        return;

    this->GetCurrentPlayer()->SetPosition();
}

void MainWindow::SetStartupTime(bool force_reset, qint64 new_time)
{
    qint64 current_epoc = QDateTime::currentMSecsSinceEpoch();  // Get current EPOC
    this->startupTime = this->settings->value(SETTINGS_KEY_START_EPOC, 0).toLongLong();  // Obtain stored value, using current epoc as default
    std::cout << "Loading startupTime from config: " << this->startupTime << std::endl;

    if (this->startupTime == 0 || force_reset)
    {
        std::cout << "Resetting startupTime to epoc: " << current_epoc << std::endl;
        this->startupTime = new_time == 0 ? current_epoc : new_time;
        this->settings->setValue(SETTINGS_KEY_START_EPOC, this->startupTime);  // If setting used the deafult, save it.
    }
}

void MainWindow::ToggleAlwaysOnTop(bool new_value)
{
    this->settings->setValue(SETTINGS_KEY_ALWAYS_ON_TOP, new_value ? 1 : 0);
    this->DisplayInfo("Application must be restarted for changes to take effect.");
}

Player* MainWindow::GetCurrentPlayer()
{
    return this->players[this->currentPlayerItx];
}
Player* MainWindow::GetNextPlayer()
{
    return this->players[this->currentPlayerItx ? 0 : 1];
}


void MainWindow::MuteButtonSlot()
{
    if (this->GetCurrentPlayer()->GetMediaPlayer()->isMuted())
    {
        this->SetMute(false);
        this->GetMuteButton()->setText(MUTE_BUTTON_TEXT_MUTE);
    } else {
        this->SetMute(true);
        this->GetMuteButton()->setText(MUTE_BUTTON_TEXT_UNMUTE);
    }
}

bool MainWindow::IsPlayAvailable()
{
    return (this->stationFileCount > 0);
}

bool MainWindow::IsPlaying()
{
    if (! this->IsPlayAvailable())
        return false;

    return this->is_playing;
}

void MainWindow::DisableMediaButtons()
{
    this->GetMuteButton()->setEnabled(false);
    this->GetPlayPauseButton()->setEnabled(false);
    this->GetPreviousButton()->setEnabled(false);
    this->GetNextButton()->setEnabled(false);
    this->GetVolumeDial()->setEnabled(false);
}

void MainWindow::EnableMediaButtons()
{
    this->GetMuteButton()->setEnabled(true);
    this->GetPlayPauseButton()->setEnabled(true);
    this->GetPreviousButton()->setEnabled(true);
    this->GetNextButton()->setEnabled(true);
    this->GetVolumeDial()->setEnabled(true);
}

void MainWindow::Play()
{
    // Cannot play if already playing
    if (this->IsPlaying())
        return;

    // No stations available
    if (! this->IsPlayAvailable())
        return;

    if (this->pause_time)
    {
        // If previous paused by user and pause_time was set,
        // add the amount of time paused to the global timer,
        // so the position of tracks won't have changed.
        this->SetStartupTime(true, this->GetStartupTime());
        this->pause_time = 0;
    }

    this->is_playing = true;
    this->GetCurrentPlayer()->Play();

    this->GetPlayPauseButton()->setText("Pause");
}

void MainWindow::SetMute(bool muted)
{
    this->GetCurrentPlayer()->GetMediaPlayer()->setMuted(muted);
    this->GetNextPlayer()->GetMediaPlayer()->setMuted(muted);
}

void MainWindow::NextStation()
{
    if (! this->IsPlayAvailable())
        return;

    // If end of station index, start from 0
    this->SelectStation((this->currentStation == (this->stationFileCount - 1)) ? 0 : this->currentStation + 1);
}

void MainWindow::PreviousStation()
{
    if (! this->IsPlayAvailable())
        return;

    this->SelectStation(
        (this->currentStation == 0) ? (this->stationFileCount - 1) : (this->    currentStation - 1)
    );

}

void MainWindow::DisplayError(QString err)
{
    QMessageBox messageBox;
    messageBox.critical(0, "Error", err);
    messageBox.setFixedSize(500, 200);
}

void MainWindow::DisplayInfo(QString info)
{
    QMessageBox messageBox;
    messageBox.information(0, "Info", info);
    messageBox.setFixedSize(500, 200);
}

int MainWindow::LoadCurrentStation()
{
    return this->settings->value(SETTINGS_KEY_CURRENT_STATION_INDEX, 0).toInt();
}

void MainWindow::SaveCurrentStation()
{
    this->settings->setValue(SETTINGS_KEY_CURRENT_STATION_INDEX, this->currentStation);
}

void MainWindow::SelectStation(int station_index)
{
    this->DisableMediaButtons();

    if (station_index >= this->stationFileCount)
    {
        this->DisplayError("Station ID out of range");
        this->EnableMediaButtons();
        return;
    }

    this->currentStation = station_index;
    this->SaveCurrentStation();

    this->SetDisplay("Re-tuning...");
    // Set start time before performing any media swapping, so that
    // if the media loading takes some time, the amount of time
    // held in artificial 're-tuning' loop compensates for this.
    qint64 start_pause = QDateTime::currentMSecsSinceEpoch();

    this->GetNextPlayer()->PrepareFlipTo(QUrl::fromLocalFile(this->stationFiles[station_index]));

    // Pause old player, start new one and flip
    bool was_playing = this->IsPlaying();
    this->GetCurrentPlayer()->FlipFrom(was_playing);

    // Pause for dramatic effect!
    while (QDateTime::currentMSecsSinceEpoch() < (start_pause + STATION_CHANGE_DRAMATIC_PAUSE_DURATION))
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    this->currentPlayerItx = this->currentPlayerItx ? 0 : 1;

    // Flip to new player (note now GetCurrentPlayer since currentPlayerItx has now been updated).
    this->GetCurrentPlayer()->FlipTo(was_playing);

    this->SetDisplay(this->GetMediaName());
    this->EnableMediaButtons();
}

QString MainWindow::GetMediaName()
{
    QString name = this->GetCurrentPlayer()->GetMediaPlayer()->metaData(QMediaMetaData::Title).toString();
    if (name.isEmpty()) {
        name = this->GetCurrentPlayer()->GetMediaPlayer()->currentMedia().canonicalUrl().fileName();

        // Check if name contains a dot and attempt to remove
        if (name.indexOf('.') != -1) {
            for (int itx = name.length() - 1; itx > 1;  itx -- ) {
                bool extension_removed = name[itx] == '.';
                name.truncate(itx);
                if (extension_removed)
                    break;
            }
        }
    }

    return name;
}

QLabel* MainWindow::GetDisplay() {
    return this->findChild<QLabel *>("display");
}

void MainWindow::SetDisplay(QString text)
{
    this->GetDisplay()->setText(text);
}

QDial* MainWindow::GetVolumeDial()
{
    return this->findChild<QDial *>("volumeDial");
}

void MainWindow::VolumeDialChangeSlot()
{
    int new_volume = this->GetVolumeDial()->value();

    // Save new volume value
    this->settings->setValue(SETTINGS_KEY_VOLUME, QVariant(new_volume));

    // Set volume of both players
    this->GetCurrentPlayer()->GetMediaPlayer()->setVolume(new_volume);
    this->GetNextPlayer()->GetMediaPlayer()->setVolume(new_volume);
}

QPushButton* MainWindow::GetMuteButton()
{
    return this->findChild<QPushButton *>("muteButton");
}

QPushButton* MainWindow::GetPlayPauseButton()
{
    return this->findChild<QPushButton *>("playPauseButton");
}

QPushButton* MainWindow::GetNextButton()
{
    return this->findChild<QPushButton *>("nextButton");
}


QPushButton* MainWindow::GetPreviousButton()
{
    return this->findChild<QPushButton *>("prevButton");
}

QLabel* MainWindow::GetPositionLabel()
{
    return this->findChild<QLabel *>("positionLabel");
}

QWidget* MainWindow::GetDisplayBackgroundWidget()
{
    return this->findChild<QWidget *>("displayBackground");
}

QWidget* MainWindow::GetBackgroundWidget()
{
    return this->findChild<QWidget *>("centralwidget");
}

void MainWindow::PopulateFileList()
{
    // Clear old files
    this->stationFileCount = 0;
    for (int itx = 0; itx < MAX_STATIONS; itx ++)
        this->stationFiles[itx] = "";

    // Setup directory iterator
    QDirIterator it(this->scan_directory, QStringList() << "*.mp3", QDir::Files, QDirIterator::Subdirectories);
    QDir dir = QDir::currentPath();
    while (it.hasNext())
    {
        this->stationFiles[stationFileCount] = dir.cleanPath(dir.absoluteFilePath(it.next()));
        this->stationFileCount ++;
        // Check if reached array limit for stations
        if (this->stationFileCount == MAX_STATIONS)
        {
            this->DisplayError("Reached maximum number of stations.");
            return;
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


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

    this->playing = false;

    // Create player objects and set current
    // player index to first player
    this->players[0] = new QMediaPlayer;
    this->players[1] = new QMediaPlayer;
    this->currentPlayerItx = 0;

    // Disable interupts
    this->mediaStateChangeInteruptEnabled = false;

    this->GetVolumeDial()->setValue(this->settings->value(SETTINGS_KEY_VOLUME, INITIAL_VOLUME).toInt());
    this->VolumeDialChangeSlot();

    // Set startup time
    this->SetStartupTime(false);

    // Select initial station
    this->UpdateDirectory(
        this->settings->value(SETTINGS_KEY_DIRECTORY, INITIAL_DIRECTORY).toString(),
        this->LoadCurrentStation());
    this->SetCurrentPlayerPosition();
    this->Play();

    // Set background colour of display label
    this->GetDisplay()->setStyleSheet("QLabel { background-color: white; margin: 1px; }");

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

    this->menu_bar = new QMenuBar(0);
    this->menu_bar->setNativeMenuBar(false);
    this->menu_bar->addMenu(this->file_menu);
    this->setMenuBar(this->menu_bar);

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

    // Listen to media events
    QObject::connect(this->players[0], SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(OnMediaStateChange(QMediaPlayer::State)));
    QObject::connect(this->players[1], SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(OnMediaStateChange(QMediaPlayer::State)));
}

void MainWindow::OnMediaStateChange(QMediaPlayer::State newState) {
    std::cout << "New State: " << newState << std::endl;
    if (this->mediaStateChangeInteruptEnabled && newState == QMediaPlayer::StoppedState) {
        std::cout << "Interupts enabled, restarting current player" << std::endl;
        this->GetCurrentPlayer()->play();
    }
}

void MainWindow::PlayPauseButtonSlot() {

}

void MainWindow::DisableMediaInterupts() {
    this->mediaStateChangeInteruptEnabled = false;
}
void MainWindow::EnableMediaInterupts() {
    this->mediaStateChangeInteruptEnabled = true;
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
    this->GetCurrentPlayer()->pause();
    this->GetNextPlayer()->pause();
    this->DisableMediaButtons();
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
    this->SetStartupTime(true);

    // Restart current station
    if (! this->IsPlayAvailable())
        return;

    // Pause current song
    this->GetCurrentPlayer()->pause();
    this->SetCurrentPlayerPosition();
    this->GetCurrentPlayer()->play();
}

void MainWindow::SetStartupTime(bool force_reset)
{
    qint64 current_epoc = QDateTime::currentMSecsSinceEpoch();  // Get current EPOC
    this->startupTime = this->settings->value(SETTINGS_KEY_START_EPOC, 0).toLongLong();  // Obtain stored value, using current epoc as default
    std::cout << "Loading startupTime from config: " << this->startupTime << std::endl;

    if (this->startupTime == 0 || force_reset)
    {
        std::cout << "Resetting startupTime to epoc: " << current_epoc << std::endl;
        this->startupTime = current_epoc;
        this->settings->setValue(SETTINGS_KEY_START_EPOC, this->startupTime);  // If setting used the deafult, save it.
    }
}

void MainWindow::ToggleAlwaysOnTop(bool new_value)
{
    this->settings->setValue(SETTINGS_KEY_ALWAYS_ON_TOP, new_value ? 1 : 0);
    this->DisplayInfo("Application must be restarted for changes to take effect.");
}

QMediaPlayer* MainWindow::GetCurrentPlayer()
{
    return this->players[this->currentPlayerItx];
}
QMediaPlayer* MainWindow::GetNextPlayer()
{
    return this->players[this->currentPlayerItx ? 0 : 1];
}

void MainWindow::FlipPlayer()
{
    this->currentPlayerItx = this->currentPlayerItx ? 0 : 1;

    // Update slot for mediaChanged
    if (this->position_change_connection != nullptr) {
        disconnect(this->position_change_connection);
    }
    QObject::connect(this->GetCurrentPlayer(), SIGNAL(positionChanged(qint64)), this, SLOT(OnPositionChanged(qint64)));
}

void MainWindow::OnPositionChanged(qint64 new_position)
{

    qint64 duration = this->GetCurrentPlayer()->duration();

    if (duration >= 1000)
    {

        duration = duration / 1000;

        char label_text[59];
        long long dur_mins = std::floor(duration / 60);
        long long dur_hrs = std::floor(dur_mins / 60);
        long long new_pos_mins = 0;
        long long new_pos_hrs = 0;

        if (new_position >= 1000) {
            new_position = new_position / 1000;
            new_pos_mins = std::floor(new_position / 60);
            new_pos_hrs = std::floor(new_pos_mins / 60);
        }

        snprintf(
            label_text,
            sizeof(label_text),
            "%lld:%02lld:%02lld / %lld:%02lld:%02lld",
            new_pos_hrs,
            new_pos_mins % 60,
            new_position % 60,
            dur_hrs,
            dur_mins % 60,
            duration % 60);
        this->GetPositionLabel()->setText(label_text);
    } else {
        this->GetPositionLabel()->setText("0:00:00 / 0:00:00");
    }
}

void MainWindow::MuteButtonSlot()
{
    if (this->GetCurrentPlayer()->isMuted())
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

    return (this->GetCurrentPlayer()->state() == QMediaPlayer::PlayingState);
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

    this->GetCurrentPlayer()->play();
    if (this->GetCurrentPlayer()->error())
        this->DisplayError(this->GetCurrentPlayer()->errorString());
    else if (this->GetCurrentPlayer()->state() != QMediaPlayer::PlayingState)
        this->DisplayError("Not playing");

    // Enable interupts, as no longer expecting media changes.
    this->EnableMediaInterupts();
}

void MainWindow::SetMute(bool muted)
{
    this->GetCurrentPlayer()->setMuted(muted);
    this->GetNextPlayer()->setMuted(muted);
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

    // Update file path of next player
    this->GetNextPlayer()->setMedia(QUrl::fromLocalFile(this->stationFiles[station_index]));

    // Wait for player to load media
    while (this->GetNextPlayer()->mediaStatus() == QMediaPlayer::LoadingMedia)
        QCoreApplication::processEvents(QEventLoop::AllEvents, MEDIA_LOAD_WAIT_PERIOD);

    this->DisableMediaInterupts();

    // Pause old player, start new one and flip
    bool was_playing = this->IsPlaying();
    if (was_playing)
    {
        this->GetCurrentPlayer()->pause();
        this->SetDisplay("Re-tuning...");

        // Pause for dramatic effect!
        qint64 start_pause = QDateTime::currentMSecsSinceEpoch();
        while (QDateTime::currentMSecsSinceEpoch() < (start_pause + STATION_CHANGE_DRAMATIC_PAUSE_DURATION))
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    this->FlipPlayer();

    if (was_playing)
    {
        this->SetCurrentPlayerPosition();
        this->GetCurrentPlayer()->play();
    }

    this->EnableMediaInterupts();

    this->SetDisplay(this->GetMediaName());
    this->EnableMediaButtons();
}

void MainWindow::SetCurrentPlayerPosition()
{
    // Set position based on time since application startup, using modulus of track length.
    // Ignore tts less than 0, maybe due to time change or race condition
    qint64 tts = (QDateTime::currentMSecsSinceEpoch() - this->startupTime);
    if (tts > 0)
    {
        qint64 dur = this->GetCurrentPlayer()->duration();
        if (dur > 0)
            this->GetCurrentPlayer()->setPosition(tts % this->GetCurrentPlayer()->duration());
    }
}

QString MainWindow::GetMediaName()
{
    QString name = this->GetCurrentPlayer()->metaData(QMediaMetaData::Title).toString();
    if (name.isEmpty()) {
        name = this->GetCurrentPlayer()->currentMedia().canonicalUrl().fileName();

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
    this->GetCurrentPlayer()->setVolume(new_volume);
    this->GetNextPlayer()->setVolume(new_volume);
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


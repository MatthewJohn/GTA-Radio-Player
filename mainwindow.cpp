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

    bool always_on_top_set = (this->settings->value(SETTINGS_KEY_ALWAYS_ON_TOP, DEFAULT_ALWAYS_ON_TOP).toInt() == 1);
    if (always_on_top_set)
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    this->playing = false;

    this->players[0] = new QMediaPlayer;
    this->players[1] = new QMediaPlayer;
    this->currentPlayerItx = 0;

    this->GetVolumeDial()->setValue(this->settings->value(SETTINGS_KEY_VOLUME, INITIAL_VOLUME).toInt());
    this->VolumeDialChangeSlot();

    // Set startup time
    qint64 current_epoc = QDateTime::currentMSecsSinceEpoch();  // Get current EPOC
    this->startupTime = this->settings->value(SETTINGS_KEY_START_EPOC, current_epoc).toLongLong();  // Obtain stored value, using current epoc as default
    if (this->startupTime == current_epoc)
        this->settings->setValue(SETTINGS_KEY_START_EPOC, current_epoc);  // If setting used the deafult, save it.

    // Select initial station
    this->UpdateDirectory(this->settings->value(SETTINGS_KEY_DIRECTORY, INITIAL_DIRECTORY).toString());
    this->PlayPauseButtonSlot();

    this->change_directory_action = new QAction(0);
    this->change_directory_action->setText("Change Directory");
    this->always_on_top_action = new QAction(0);
    this->always_on_top_action->setText("Always on top");
    this->always_on_top_action->setCheckable(true);
    this->always_on_top_action->setChecked(always_on_top_set);
    this->file_menu = new QMenu();
    this->file_menu->setTitle("File");
    this->file_menu->addAction(this->change_directory_action);
    this->file_menu->addAction(this->always_on_top_action);
    this->menu_bar = new QMenuBar(0);
    this->menu_bar->setNativeMenuBar(false);
    this->menu_bar->addMenu(this->file_menu);
    this->setMenuBar(this->menu_bar);

    // Bind knobs
    QObject::connect(this->GetPlayPauseButton(), SIGNAL(clicked()), this, SLOT(PlayPauseButtonSlot()));
    QObject::connect(this->GetPreviousButton(), SIGNAL(clicked()), this, SLOT(NextStation()));
    QObject::connect(this->GetNextButton(), SIGNAL(clicked()), this, SLOT(PreviousStation()));
    QObject::connect(this->GetVolumeDial(), SIGNAL(valueChanged(int)), this, SLOT(VolumeDialChangeSlot()));

    // Menu item
    QObject::connect(this->change_directory_action, SIGNAL(triggered(bool)), this, SLOT(OpenChangeDirectory()));
    QObject::connect(this->always_on_top_action, SIGNAL(toggled(bool)), this, SLOT(ToggleAlwaysOnTop(bool)));

}

void MainWindow::UpdateDirectory(QString new_directory)
{
    this->settings->setValue(SETTINGS_KEY_DIRECTORY, new_directory);
    this->scan_directory = new_directory;
    this->PopulateFileList();
    if (this->IsPlayAvailable())
        this->SelectStation(0);
    else
        this->DisablePlayer();
}

void MainWindow::DisablePlayer()
{
    this->SetDisplay("No tracks found...");
    this->GetCurrentPlayer()->pause();
    this->GetNextPlayer()->pause();
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
        this->UpdateDirectory(selected[0]);
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
}

void MainWindow::PlayPauseButtonSlot()
{
    if (this->IsPlaying() || ! this->IsPlayAvailable())
    {
        this->GetCurrentPlayer()->pause();
        this->GetPlayPauseButton()->setText(PLAY_PAUSE_BUTTON_TEXT_PLAY);
    } else {
        this->Play();
        this->GetPlayPauseButton()->setText(PLAY_PAUSE_BUTTON_TEXT_PAUSE);
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
}

void MainWindow::Pause()
{
    if (! this->IsPlaying())
        return;

    this->GetCurrentPlayer()->pause();
}

void MainWindow::NextStation()
{
    if (! this->IsPlayAvailable())
        return;

    // If end of station index, start from 0
    this->SelectStation((currentStation == (this->stationFileCount - 1)) ? 0 : currentStation + 1);
}

void MainWindow::PreviousStation()
{
    if (! this->IsPlayAvailable())
        return;

    this->SelectStation(
        (currentStation == 0) ? (this->stationFileCount - 1) : (currentStation - 1)
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

void MainWindow::SelectStation(int station_index)
{
    if (station_index >= this->stationFileCount)
    {
        this->DisplayError("Station ID out of range");
        return;
    }

    this->currentStation = station_index;

    // Update file path of next player
    this->GetNextPlayer()->setMedia(QUrl::fromLocalFile(this->stationFiles[station_index]));

    // Wait for player to load media
    while (this->GetNextPlayer()->mediaStatus() == QMediaPlayer::LoadingMedia)
        QCoreApplication::processEvents(QEventLoop::AllEvents, MEDIA_LOAD_WAIT_PERIOD);

    // Set position based on time since application startup, using modulus of track length.
    // Ignore tts less than 0, maybe due to time change or race condition
    qint64 tts = (QDateTime::currentMSecsSinceEpoch() - this->startupTime);
    if (tts > 0)
    {
        qint64 dur = this->GetNextPlayer()->duration();
        if (dur > 0)
            this->GetNextPlayer()->setPosition(tts % this->GetNextPlayer()->duration());
    }

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
        this->GetNextPlayer()->play();
    }
    this->FlipPlayer();
    this->SetDisplay(this->GetMediaName());
}

QString MainWindow::GetMediaName()
{
    QString name = this->GetCurrentPlayer()->metaData(QMediaMetaData::Title).toString();
    if (name.isEmpty())
        name = this->GetCurrentPlayer()->currentMedia().canonicalUrl().fileName();
    return name;
}

void MainWindow::SetDisplay(QString text)
{
    this->findChild<QTextBrowser *>("display")->setText(text);
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
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


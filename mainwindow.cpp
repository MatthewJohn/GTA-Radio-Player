#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->playing = false;

    this->stationFileCount = 0;
    this->PopulateFileList();
    this->players[0] = new QMediaPlayer;
    this->players[1] = new QMediaPlayer;
    this->currentPlayerItx = 0;

    this->GetVolumeDial()->setValue(INITIAL_VOLUME);
    this->VolumeDialChangeSlot();

    // Set startup time
    this->startupTime = QDateTime::currentMSecsSinceEpoch();

    // Select initial station
    if (this->IsPlayAvailable())
        this->SelectStation(0);
    this->PlayPauseButtonSlot();

    // Bind knobs
    QObject::connect(this->GetPlayPauseButton(), SIGNAL(clicked()), this, SLOT(PlayPauseButtonSlot()));
    QObject::connect(this->GetPreviousButton(), SIGNAL(clicked()), this, SLOT(NextStation()));
    QObject::connect(this->GetNextButton(), SIGNAL(clicked()), this, SLOT(PreviousStation()));
    QObject::connect(this->GetVolumeDial(), SIGNAL(valueChanged()), this, SLOT(VolumeDialChangeSlot()));
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
    if (this->IsPlaying())
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
    if (this->stationFileCount == 0)
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

    int new_station_id;
    // If end of station index, start from 0
    if (currentStation == (this->stationFileCount - 1))
        new_station_id = 0;
    else
        new_station_id = currentStation + 1;
    this->SelectStation(new_station_id);
}

void MainWindow::PreviousStation()
{
    if (! this->IsPlayAvailable())
        return;

    int new_station_id;
    if (currentStation == 0)
        new_station_id = (this->stationFileCount - 1);
    else
        new_station_id = currentStation - 1;
    this->SelectStation(new_station_id);

}

void MainWindow::DisplayError(QString err)
{
    QMessageBox messageBox;
    messageBox.critical(0, "Error", err);
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
    this->GetCurrentPlayer()->setVolume(this->GetVolumeDial()->value());
    this->GetNextPlayer()->setVolume(this->GetVolumeDial()->value());
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
    QString dir_path = "./";
    QDirIterator it(dir_path, QStringList() << "*.mp3", QDir::Files, QDirIterator::Subdirectories);
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


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
    this->player = new QMediaPlayer;

    this->SetVolume(INITIAL_VOLUME);

    // Select initial station
    if (this->IsPlayAvailable())
        this->SelectStation(0);
    this->Play();
}

bool MainWindow::IsPlayAvailable()
{
    return (this->stationFileCount > 0);
}

void MainWindow::Play()
{
    // Cannot play if already playing
    if (this->player->state() == QMediaPlayer::PlayingState)
        return;

    // No stations available
    if (this->stationFileCount == 0)
        return;

    this->player->play();
    if (this->player->error())
        this->DisplayError(this->player->errorString());
    else if (this->player->state() != QMediaPlayer::PlayingState)
        this->DisplayError("Not playing");
}

void MainWindow::Pause()
{
    if (this->player->state() != QMediaPlayer::PlayingState)
        return;

    this->player->pause();
}

void MainWindow::SetVolume(qint64 new_volume)
{
    this->player->setVolume(new_volume);
}

void MainWindow::NextStation()
{

}

void MainWindow::PreviousStation()
{

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
    QDir dir = QDir::currentPath();
    this->DisplayError(dir.cleanPath(dir.absoluteFilePath(this->stationFiles[station_index])));
    player->setMedia(QUrl::fromLocalFile(dir.cleanPath(dir.absoluteFilePath(this->stationFiles[station_index]))));
}

void MainWindow::PopulateFileList()
{
    QString dir = "../";
    QDirIterator it(dir, QStringList() << "*.mp3", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        this->stationFiles[stationFileCount] = it.next();
        this->stationFileCount ++;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->playing = false;

    this->PopulateFileList();

    this->play();
    this->player = new QMediaPlayer;
}

void MainWindow::Play()
{
    // Cannot play if already playing
    if (this->playing)
        return;

    // No stations available
    if (this->stationFileCount == 0)
        return;
}

void MainWindow::PopulateFileList()
{
    QString dir = ".";
    QDirIterator it(dir, QStringList() << "*.mp3", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        qDebug() << it.next();
        this->stationFiles[stationFileCount] = it.next();
        this->stationFileCount ++;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


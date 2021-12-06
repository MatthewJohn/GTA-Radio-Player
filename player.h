#ifndef PLAYER_H
#define PLAYER_H

#include <cmath>

#include <QObject>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QLabel>

class MainWindow;

class Player : public QObject
{
    Q_OBJECT

public:
    Player();
    ~Player();

    void Setup(MainWindow* main_window);
    QMediaPlayer* GetMediaPlayer();

    bool is_active;


public slots:
    // Slots for media events
    void OnMediaStateChange(QMediaPlayer::State new_state);
    void OnDurationChange(qint64 new_duration);
    void OnPositionChanged(qint64 new_position);

private:
    MainWindow* main_window;
    QMediaPlayer* player;
};

#endif // PLAYER_H


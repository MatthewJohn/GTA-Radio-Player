#ifndef PLAYER_H
#define PLAYER_H

#include <cmath>

#include <QObject>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QLabel>
#include <QCoreApplication>

class MainWindow;

class Player : public QObject
{
    Q_OBJECT

public:
    Player();
    ~Player();

    void Setup(MainWindow* main_window, int player_index);
    QMediaPlayer* GetMediaPlayer();

    void PrepareFlipTo(QUrl url);
    void FlipFrom(bool was_playing);
    void FlipTo(bool was_playing);
    void Play();
    void Pause();

public slots:
    // Slots for media events
    void OnMediaStatusChange(QMediaPlayer::MediaStatus status);
    void OnDurationChange(qint64 new_duration);
    void OnPositionChanged(qint64 new_position);
    void OnStateChanged(QMediaPlayer::State state);

private:
    MainWindow* main_window;
    QMediaPlayer* player;
    int player_index;
    bool is_active;
    bool media_interupts_enabled;
    bool media_loaded;
    bool media_buffered;
    qint64 track_duration;
    void SetPosition();
    void PrintDebug(QString debug);

};

#endif // PLAYER_H


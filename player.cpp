#include "player.h"
#include "mainwindow.h"

Player::Player()
{
    //this->main_window = main_window;
    this->player = new QMediaPlayer;
    this->is_active = false;

}

void Player::Setup(MainWindow* main_window)
{
    this->main_window = main_window;

    QObject::connect(this->GetMediaPlayer(), SIGNAL(stateChanged(QMediaPlayer::State)), this->main_window, SLOT(OnMediaStateChange(QMediaPlayer::State)));

    // Add callbacks to handle durationChanged to set position
    QObject::connect(this->GetMediaPlayer(), SIGNAL(durationChanged(qint64)), this, SLOT(OnDurationChange(qint64)));

    QObject::connect(this->GetMediaPlayer(), SIGNAL(positionChanged(qint64)), this, SLOT(OnPositionChanged(qint64)));
}

QMediaPlayer* Player::GetMediaPlayer()
{
    return this->player;
}

void Player::OnPositionChanged(qint64 new_position)
{

    // Do not act on position change if not active
    if (! this->is_active)
        return;

     std::cout << "PUTTING DATA";
    qint64 duration = this->GetMediaPlayer()->duration();

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
        this->main_window->GetPositionLabel()->setText(label_text);
    } else {
        this->main_window->GetPositionLabel()->setText("0:00:00 / 0:00:00");
    }
}


void Player::OnDurationChange(qint64 new_duration) {
    std::cout << "OnDurationChange called" << std::endl;
    if (this->main_window->set_position_callback_enabled) {
        // Re-disable callback
        this->main_window->set_position_callback_enabled = false;

        // Set position based on time since application startup, using modulus of track length.
        // Ignore tts less than 0, maybe due to time change or race condition
        qint64 tts = (QDateTime::currentMSecsSinceEpoch() - this->main_window->GetStartupTime());
        if (tts > 0)
        {
            qint64 dur = this->GetMediaPlayer()->duration();
            std::cout << "Track duration: " << this->GetMediaPlayer()->duration() << std::endl;
            if (dur > 0) {
                std::cout << "Setting track to position: " << tts % this->GetMediaPlayer()->duration() << std::endl;
                this->GetMediaPlayer()->setPosition(tts % this->GetMediaPlayer()->duration());
            }
        }
    }
}

void Player::OnMediaStateChange(QMediaPlayer::State newState) {
    std::cout << "New State: " << newState << std::endl;
    // If interupts are disabled (when swapping players), if some comes to an end,
    // start the player again.
    if (this->main_window->MediaStateChangeInteruptEnabled && newState == QMediaPlayer::StoppedState) {
        std::cout << "Interupts enabled, restarting current player" << std::endl;
        this->GetMediaPlayer()->play();
    }
}

Player::~Player()
{
}


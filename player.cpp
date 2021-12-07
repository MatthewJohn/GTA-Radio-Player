#include "player.h"
#include "mainwindow.h"

Player::Player()
{
    this->player = new QMediaPlayer;
    this->is_active = false;
    this->media_interupts_enabled = false;
    this->position_set_required = false;
    this->media_buffered = false;
    this->media_loaded = false;
    this->track_duration = 0;
}

void Player::Setup(MainWindow* main_window, int player_index)
{
    this->main_window = main_window;
    this->player_index = player_index;

    QObject::connect(this->GetMediaPlayer(), SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(OnStateChanged(QMediaPlayer::State)));
    QObject::connect(this->GetMediaPlayer(), SIGNAL(durationChanged(qint64)), this, SLOT(OnDurationChange(qint64)));
    QObject::connect(this->GetMediaPlayer(), SIGNAL(positionChanged(qint64)), this, SLOT(OnPositionChanged(qint64)));
    QObject::connect(this->GetMediaPlayer(), SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(OnMediaStatusChange(QMediaPlayer::MediaStatus)));
    this->PrintDebug("Setup connectors");
}

void Player::OnMediaStatusChange(QMediaPlayer::MediaStatus status)
{
    std::cout << "Player " << this-> player_index << ": State: " << status << std::endl;
    if (status == QMediaPlayer::LoadedMedia)
        this->media_loaded = true;
    else if (status == QMediaPlayer::BufferedMedia)
        this->media_buffered = true;
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
    std::cout << "Player " << this-> player_index << ": OnDurationChange called: " << new_duration << std::endl;
    this->track_duration = new_duration;
    if (this->position_set_required) {
        // Re-disable callback
        this->position_set_required = false;
    }
}

void Player::OnStateChanged(QMediaPlayer::State newState) {
    std::cout << "Player " << this-> player_index << ": Media State: " << newState << std::endl;
    // If interupts are disabled (when swapping players), if some comes to an end,
    // start the player again.
    if (this->media_interupts_enabled && newState == QMediaPlayer::StoppedState) {
        std::cout << "Interupts enabled, restarting current player" << std::endl;
        this->GetMediaPlayer()->play();
    }
}

void Player::PrepareFlipTo(QUrl url)
{
    this->position_set_required = true;

    this->media_loaded = false;
    this->media_buffered = false;
    this->track_duration = 0;

    int old_volume = this->GetMediaPlayer()->volume();
    bool was_active = this->is_active;

    // Update file path of next player
    this->PrintDebug("Loading file: " + url.url());
    this->GetMediaPlayer()->setMedia(url);

    // Check for any errors after loading media
    if (this->GetMediaPlayer()->error())
        this->main_window->DisplayError(this->GetMediaPlayer()->errorString());
    else if (this->GetMediaPlayer()->state() != QMediaPlayer::PlayingState)
        this->main_window->DisplayError("Not playing");

    this->PrintDebug("Waiting for media to load.");
    while (this->media_loaded == false)
        // Wait for 50ms
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    this->PrintDebug("Media loaded.");

    // Play track
    this->is_active = false;
    this->GetMediaPlayer()->pause();

    // Wait for media to buffer
    this->PrintDebug("Waiting for media to buffer.");
    while (this->media_buffered == false)
        // Wait for 50ms
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    this->PrintDebug("Media buffered.");

    // Wait for position to be set (and required flag set to false)
    // @TODO: Do not play audio with minimal volume - this will be audible to the user.
    // This is required as the duration will not be populated (nor will the durationChanged
    // slot be called) if: media is paused instead of played, mediaplayer volume is set to 0
    // or mediaplayer is set to muted.
    // Therefore, this is the only way to be able to obtain the duration of the track.
    this->GetMediaPlayer()->setVolume(1);
    this->GetMediaPlayer()->play();
    this->PrintDebug("Waiting for duration to be set.");
    while (this->track_duration == 0)
        // Wait for 50ms
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    this->GetMediaPlayer()->pause();
    this->PrintDebug("Duration set.");

    this->GetMediaPlayer()->setVolume(old_volume);
    this->is_active = was_active;
}

void Player::PrintDebug(QString debug)
{
    std::cout << "Player " << this->player_index << ": " << debug.toStdString() << std::endl;
}

void Player::FlipFrom(bool was_playing)
{
    this->media_interupts_enabled = false;
    if (was_playing)
        this->player->pause();
}

void Player::FlipTo(bool was_playing)
{
    this->SetPosition();

    this->is_active = true;
    if (was_playing)
        this->Play();

    this->media_interupts_enabled = true;
}

void Player::SetPosition()
{

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

void Player::Play()
{
    this->GetMediaPlayer()->play();
}

Player::~Player()
{
}


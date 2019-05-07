#include "playercontrols.h"

#include <QBoxLayout>
#include <QToolButton>
#include <QComboBox>
#include <QAudio>

PlayerControls::PlayerControls(QWidget *parent)
    : QWidget(parent)
{
    _playButton = new QToolButton(this);
    _playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    connect(_playButton, &QAbstractButton::clicked, this, &PlayerControls::playClicked);

    _stopButton = new QToolButton(this);
    _stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    _stopButton->setEnabled(false);

    connect(_stopButton, &QAbstractButton::clicked, this, &PlayerControls::stop);

    _nextButton = new QToolButton(this);
    _nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));

    connect(_nextButton, &QAbstractButton::clicked, this, &PlayerControls::next);

    _previousButton = new QToolButton(this);
    _previousButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));

    connect(_previousButton, &QAbstractButton::clicked, this, &PlayerControls::previous);

    _muteButton = new QToolButton(this);
    _muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));

    connect(_muteButton, &QAbstractButton::clicked, this, &PlayerControls::muteClicked);

    _volumeSlider = new QSlider(Qt::Horizontal, this);
    _volumeSlider->setRange(0, 100);

    connect(_volumeSlider, &QSlider::valueChanged, this, &PlayerControls::onVolumeSliderValueChanged);

    _rateBox = new QComboBox(this);
    _rateBox->addItem("0.5x", QVariant(0.5));
    _rateBox->addItem("1.0x", QVariant(1.0));
    _rateBox->addItem("2.0x", QVariant(2.0));
    _rateBox->setCurrentIndex(1);

    connect(_rateBox, QOverload<int>::of(&QComboBox::activated), this, &PlayerControls::updateRate);

    QBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(_stopButton);
    layout->addWidget(_previousButton);
    layout->addWidget(_playButton);
    layout->addWidget(_nextButton);
    layout->addWidget(_muteButton);
    layout->addWidget(_volumeSlider);
    layout->addWidget(_rateBox);
    setLayout(layout);
}

QMediaPlayer::State PlayerControls::state() const
{
    return _playerState;
}

void PlayerControls::setState(QMediaPlayer::State state)
{
    if (state != _playerState) {
        _playerState = state;

        switch (state) {
        case QMediaPlayer::StoppedState:
            _stopButton->setEnabled(false);
            _playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
        case QMediaPlayer::PlayingState:
            _stopButton->setEnabled(true);
            _playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            break;
        case QMediaPlayer::PausedState:
            _stopButton->setEnabled(true);
            _playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
        }
    }
}

int PlayerControls::volume() const
{
    qreal linearVolume =  QAudio::convertVolume(_volumeSlider->value() / qreal(100),
                                                QAudio::LogarithmicVolumeScale,
                                                QAudio::LinearVolumeScale);

    return qRound(linearVolume * 100);
}

void PlayerControls::setVolume(int volume)
{
    qreal logarithmicVolume = QAudio::convertVolume(volume / qreal(100),
                                                    QAudio::LinearVolumeScale,
                                                    QAudio::LogarithmicVolumeScale);

    _volumeSlider->setValue(qRound(logarithmicVolume * 100));
}

bool PlayerControls::isMuted() const
{
    return _playerMuted;
}

void PlayerControls::setMuted(bool muted)
{
    if (muted != _playerMuted) {
        _playerMuted = muted;

        _muteButton->setIcon(style()->standardIcon(muted
                ? QStyle::SP_MediaVolumeMuted
                : QStyle::SP_MediaVolume));
    }
}

void PlayerControls::playClicked()
{
    switch (_playerState) {
    case QMediaPlayer::StoppedState:
    case QMediaPlayer::PausedState:
        emit play();
        break;
    case QMediaPlayer::PlayingState:
        emit pause();
        break;
    }
}

void PlayerControls::muteClicked()
{
    emit changeMuting(!_playerMuted);
}

qreal PlayerControls::playbackRate() const
{
    return _rateBox->itemData(_rateBox->currentIndex()).toDouble();
}

void PlayerControls::setPlaybackRate(float rate)
{
    for (int i = 0; i < _rateBox->count(); ++i) {
        if (qFuzzyCompare(rate, float(_rateBox->itemData(i).toDouble()))) {
            _rateBox->setCurrentIndex(i);
            return;
        }
    }

    _rateBox->addItem(QString("%1x").arg(rate), QVariant(rate));
    _rateBox->setCurrentIndex(_rateBox->count() - 1);
}

void PlayerControls::updateRate()
{
    emit changeRate(playbackRate());
}

void PlayerControls::onVolumeSliderValueChanged()
{
    emit changeVolume(volume());
}

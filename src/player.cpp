#include "player.h"

#include "playercontrols.h"
#include "playlistmodel.h"
#include "videowidget.h"

#include <QMediaService>
#include <QMediaPlaylist>
#include <QMediaMetaData>
#include <QtWidgets>
#include "audiosearchengine.h"

Player::Player(QWidget *parent)
    : QWidget(parent)
{
    _player = new QMediaPlayer(this);
    _player->setAudioRole(QAudio::VideoRole);

    _audioSearchEngine = new AudioSearchEngine(this);

    connect(_audioSearchEngine, &AudioSearchEngine::error, this, &Player::displayErrorMessage);

    _playlist = new QMediaPlaylist();
    _player->setPlaylist(_playlist);

    connect(_player, &QMediaPlayer::durationChanged, this, &Player::durationChanged);
    connect(_player, &QMediaPlayer::positionChanged, this, &Player::positionChanged);
    connect(_player, QOverload<>::of(&QMediaPlayer::metaDataChanged), this, &Player::metaDataChanged);
    connect(_playlist, &QMediaPlaylist::currentIndexChanged, this, &Player::playlistPositionChanged);
    connect(_player, &QMediaPlayer::mediaStatusChanged, this, &Player::statusChanged);
    connect(_player, &QMediaPlayer::bufferStatusChanged, this, &Player::bufferingProgress);
    connect(_player, &QMediaPlayer::videoAvailableChanged, this, &Player::videoAvailableChanged);
    connect(_player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, [=](){displayErrorMessage(_player->errorString());});

    _videoWidget = new VideoWidget(this);
    _player->setVideoOutput(_videoWidget);

    _playlistModel = new PlaylistModel(this);
    _playlistModel->setPlaylist(_playlist);

    _playlistView = new QListView(this);
    _playlistView->setModel(_playlistModel);
    _playlistView->setCurrentIndex(_playlistModel->index(_playlist->currentIndex(), 0));

    connect(_playlistView, &QAbstractItemView::activated, this, &Player::jump);

    _slider = new QSlider(Qt::Horizontal, this);
    _slider->setRange(0, _player->duration() / 1000);

    _labelDuration = new QLabel(this);
    connect(_slider, &QSlider::sliderMoved, this, &Player::seek);

    QPushButton *openButton = new QPushButton(tr("Open"), this);

    connect(openButton, &QPushButton::clicked, this, &Player::open);

    PlayerControls *controls = new PlayerControls(this);
    controls->setState(_player->state());
    controls->setVolume(_player->volume());
    controls->setMuted(controls->isMuted());

    connect(controls, &PlayerControls::play, _player, &QMediaPlayer::play);
    connect(controls, &PlayerControls::pause, _player, &QMediaPlayer::pause);
    connect(controls, &PlayerControls::stop, _player, &QMediaPlayer::stop);
    connect(controls, &PlayerControls::next, _playlist, &QMediaPlaylist::next);
    connect(controls, &PlayerControls::previous, this, &Player::previousClicked);
    connect(controls, &PlayerControls::changeVolume, _player, &QMediaPlayer::setVolume);
    connect(controls, &PlayerControls::changeMuting, _player, &QMediaPlayer::setMuted);
    connect(controls, &PlayerControls::changeRate, _player, &QMediaPlayer::setPlaybackRate);
    connect(controls, &PlayerControls::stop, _videoWidget, QOverload<>::of(&QVideoWidget::update));

    connect(_player, &QMediaPlayer::stateChanged, controls, &PlayerControls::setState);
    connect(_player, &QMediaPlayer::volumeChanged, controls, &PlayerControls::setVolume);
    connect(_player, &QMediaPlayer::mutedChanged, controls, &PlayerControls::setMuted);

    _fullScreenButton = new QPushButton(tr("FullScreen"), this);
    _fullScreenButton->setCheckable(true);

    _colorButton = new QPushButton(tr("Color Options..."), this);
    _colorButton->setEnabled(false);
    connect(_colorButton, &QPushButton::clicked, this, &Player::showColorDialog);

    QBoxLayout *displayLayout = new QHBoxLayout;
    displayLayout->addWidget(_videoWidget, 2);
    displayLayout->addWidget(_playlistView);

    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    controlLayout->addWidget(openButton);
    controlLayout->addStretch(1);
    controlLayout->addWidget(controls);
    controlLayout->addStretch(1);
    controlLayout->addWidget(_fullScreenButton);
    controlLayout->addWidget(_colorButton);

    QBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(displayLayout);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(_slider);
    hLayout->addWidget(_labelDuration);
    layout->addLayout(hLayout);
    layout->addLayout(controlLayout);
    setLayout(layout);

    if (!isPlayerAvailable()) {
        QMessageBox::warning(this, tr("Service not available"),
                             tr("The QMediaPlayer object does not have a valid service.\n"\
                                "Please check the media service plugins are installed."));

        controls->setEnabled(false);
        _playlistView->setEnabled(false);
        openButton->setEnabled(false);
        _colorButton->setEnabled(false);
        _fullScreenButton->setEnabled(false);
    }

    metaDataChanged();
}

Player::~Player()
{
}

bool Player::isPlayerAvailable() const
{
    return _player->isAvailable();
}

void Player::open()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open Files"));
    QStringList supportedMimeTypes = _player->supportedMimeTypes();
    if (!supportedMimeTypes.isEmpty()) {
        supportedMimeTypes.append("audio/x-m3u"); // MP3 playlists
        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    }
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted)
        addToPlaylist(fileDialog.selectedUrls());
}

static bool isValidUrl(const QUrl &url) // Check for ".m3u" playlists.
{
    if (!url.isLocalFile()) {
        return false;
    }

    const QString localUrl = url.toLocalFile();
    const QFileInfo fileInfo(localUrl);

    return fileInfo.exists();
}

void Player::addToPlaylist(const QList<QUrl> &urls)
{
    for (auto &url: urls) {
        if (isValidUrl(url)) {
            _playlist->addMedia(url);
            _audioSearchEngine->analyze(url.toLocalFile());
        }
    }
}

void Player::setCustomAudioRole(const QString &role)
{
    //_player->setCustomAudioRole(role);
}

void Player::durationChanged(qint64 duration)
{
    _duration = duration / 1000;
    _slider->setMaximum(_duration);
}

void Player::positionChanged(qint64 progress)
{
    if (!_slider->isSliderDown())
        _slider->setValue(progress / 1000);

    updateDurationInfo(progress / 1000);
}

void Player::metaDataChanged()
{
    if (_player->isMetaDataAvailable()) {
        setTrackInfo(QString("%1 - %2")
                .arg(_player->metaData(QMediaMetaData::AlbumArtist).toString())
                .arg(_player->metaData(QMediaMetaData::Title).toString()));

        if (_coverLabel) {
            QUrl url = _player->metaData(QMediaMetaData::CoverArtUrlLarge).value<QUrl>();

            _coverLabel->setPixmap(!url.isEmpty()
                    ? QPixmap(url.toString())
                    : QPixmap());
        }
    }
}

void Player::previousClicked()
{
    // Go to previous track if we are within the first 5 seconds of playback
    // Otherwise, seek to the beginning.
    if (_player->position() <= 5000)
        _playlist->previous();
    else
        _player->setPosition(0);
}

void Player::jump(const QModelIndex &index)
{
    if (index.isValid()) {
        _playlist->setCurrentIndex(index.row());
        _player->play();
    }
}

void Player::playlistPositionChanged(int currentItem)
{
    _playlistView->setCurrentIndex(_playlistModel->index(currentItem, 0));
}

void Player::seek(int seconds)
{
    _player->setPosition(seconds * 1000);
}

void Player::statusChanged(QMediaPlayer::MediaStatus status)
{
    handleCursor(status);

    // handle status message
    switch (status) {
    case QMediaPlayer::UnknownMediaStatus:
    case QMediaPlayer::NoMedia:
    case QMediaPlayer::LoadedMedia:
        setStatusInfo(QString());
        break;
    case QMediaPlayer::LoadingMedia:
        setStatusInfo(tr("Loading..."));
        break;
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
        setStatusInfo(tr("Buffering %1%").arg(_player->bufferStatus()));
        break;
    case QMediaPlayer::StalledMedia:
        setStatusInfo(tr("Stalled %1%").arg(_player->bufferStatus()));
        break;
    case QMediaPlayer::EndOfMedia:
        QApplication::alert(this);
        break;
    case QMediaPlayer::InvalidMedia:
        displayErrorMessage(_player->errorString());
        break;
    }
}

void Player::handleCursor(QMediaPlayer::MediaStatus status)
{
#ifndef QT_NO_CURSOR
    if (status == QMediaPlayer::LoadingMedia ||
        status == QMediaPlayer::BufferingMedia ||
        status == QMediaPlayer::StalledMedia)
        setCursor(QCursor(Qt::BusyCursor));
    else
        unsetCursor();
#endif
}

void Player::bufferingProgress(int progress)
{
    if (_player->mediaStatus() == QMediaPlayer::StalledMedia)
        setStatusInfo(tr("Stalled %1%").arg(progress));
    else
        setStatusInfo(tr("Buffering %1%").arg(progress));
}

void Player::videoAvailableChanged(bool available)
{
    if (!available) {
        disconnect(_fullScreenButton, &QPushButton::clicked, _videoWidget, &QVideoWidget::setFullScreen);
        disconnect(_videoWidget, &QVideoWidget::fullScreenChanged, _fullScreenButton, &QPushButton::setChecked);
        _videoWidget->setFullScreen(false);
    } else {
        connect(_fullScreenButton, &QPushButton::clicked, _videoWidget, &QVideoWidget::setFullScreen);
        connect(_videoWidget, &QVideoWidget::fullScreenChanged, _fullScreenButton, &QPushButton::setChecked);

        if (_fullScreenButton->isChecked())
            _videoWidget->setFullScreen(true);
    }
    _colorButton->setEnabled(available);
}

void Player::setTrackInfo(const QString &info)
{
    _trackInfo = info;

    if (_statusBar) {
        _statusBar->showMessage(_trackInfo);
        _statusLabel->setText(_statusInfo);
    } else {
        if (!_statusInfo.isEmpty())
            setWindowTitle(QString("%1 | %2").arg(_trackInfo).arg(_statusInfo));
        else
            setWindowTitle(_trackInfo);
    }
}

void Player::setStatusInfo(const QString &info)
{
    _statusInfo = info;

    if (_statusBar) {
        _statusBar->showMessage(_trackInfo);
        _statusLabel->setText(_statusInfo);
    } else {
        if (!_statusInfo.isEmpty())
            setWindowTitle(QString("%1 | %2").arg(_trackInfo).arg(_statusInfo));
        else
            setWindowTitle(_trackInfo);
    }
}

void Player::displayErrorMessage(const QString &errorMessage)
{
    setStatusInfo(errorMessage);
}

void Player::updateDurationInfo(qint64 currentInfo)
{
    QString tStr;
    if (currentInfo || _duration) {
        QTime currentTime((currentInfo / 3600) % 60, (currentInfo / 60) % 60,
            currentInfo % 60, (currentInfo * 1000) % 1000);
        QTime totalTime((_duration / 3600) % 60, (_duration / 60) % 60,
            _duration % 60, (_duration * 1000) % 1000);
        QString format = "mm:ss";
        if (_duration > 3600)
            format = "hh:mm:ss";
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    _labelDuration->setText(tStr);
}

void Player::showColorDialog()
{
    if (!_colorDialog) {
        QSlider *brightnessSlider = new QSlider(Qt::Horizontal);
        brightnessSlider->setRange(-100, 100);
        brightnessSlider->setValue(_videoWidget->brightness());
        connect(brightnessSlider, &QSlider::sliderMoved, _videoWidget, &QVideoWidget::setBrightness);
        connect(_videoWidget, &QVideoWidget::brightnessChanged, brightnessSlider, &QSlider::setValue);

        QSlider *contrastSlider = new QSlider(Qt::Horizontal);
        contrastSlider->setRange(-100, 100);
        contrastSlider->setValue(_videoWidget->contrast());
        connect(contrastSlider, &QSlider::sliderMoved, _videoWidget, &QVideoWidget::setContrast);
        connect(_videoWidget, &QVideoWidget::contrastChanged, contrastSlider, &QSlider::setValue);

        QSlider *hueSlider = new QSlider(Qt::Horizontal);
        hueSlider->setRange(-100, 100);
        hueSlider->setValue(_videoWidget->hue());
        connect(hueSlider, &QSlider::sliderMoved, _videoWidget, &QVideoWidget::setHue);
        connect(_videoWidget, &QVideoWidget::hueChanged, hueSlider, &QSlider::setValue);

        QSlider *saturationSlider = new QSlider(Qt::Horizontal);
        saturationSlider->setRange(-100, 100);
        saturationSlider->setValue(_videoWidget->saturation());
        connect(saturationSlider, &QSlider::sliderMoved, _videoWidget, &QVideoWidget::setSaturation);
        connect(_videoWidget, &QVideoWidget::saturationChanged, saturationSlider, &QSlider::setValue);

        QFormLayout *layout = new QFormLayout;
        layout->addRow(tr("Brightness"), brightnessSlider);
        layout->addRow(tr("Contrast"), contrastSlider);
        layout->addRow(tr("Hue"), hueSlider);
        layout->addRow(tr("Saturation"), saturationSlider);

        QPushButton *button = new QPushButton(tr("Close"));
        layout->addRow(button);

        _colorDialog = new QDialog(this);
        _colorDialog->setWindowTitle(tr("Color Options"));
        _colorDialog->setLayout(layout);

        connect(button, &QPushButton::clicked, _colorDialog, &QDialog::close);
    }
    _colorDialog->show();
}

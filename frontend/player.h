#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include "../backend/audiosearchengine.h"

QT_BEGIN_NAMESPACE
class QAbstractItemView;
class QLabel;
class QMediaPlayer;
class QModelIndex;
class QPushButton;
class QSlider;
class QStatusBar;
class QVideoProbe;
class QVideoWidget;
class QAudioProbe;
QT_END_NAMESPACE

class PlaylistModel;

class Player : public QWidget
{
    Q_OBJECT

public:
    explicit Player(QWidget *parent = nullptr);
    ~Player();

    bool isPlayerAvailable() const;

    void addToPlaylist(const QList<QUrl> &urls);
    void setCustomAudioRole(const QString &role);

signals:
    void fullScreenChanged(bool fullScreen);

private slots:
    void open();
    void durationChanged(qint64 duration);
    void positionChanged(qint64 progress);
    void metaDataChanged();

    void previousClicked();

    void seek(int seconds);
    void jump(const QModelIndex &index);
    void playlistPositionChanged(int);

    void statusChanged(QMediaPlayer::MediaStatus status);
    void bufferingProgress(int progress);
    void videoAvailableChanged(bool available);

    void displayErrorMessage(const QString &errorMessage);

    void showColorDialog();

private:
    void setTrackInfo(const QString &info);
    void setStatusInfo(const QString &info);
    void handleCursor(QMediaPlayer::MediaStatus status);
    void updateDurationInfo(qint64 currentInfo);

    QMediaPlayer *_player = nullptr;
    QMediaPlaylist *_playlist = nullptr;
    QVideoWidget *_videoWidget = nullptr;
    QLabel *_coverLabel = nullptr;
    QSlider *_slider = nullptr;
    QLabel *_labelDuration = nullptr;
    QPushButton *_fullScreenButton = nullptr;
    QPushButton *_colorButton = nullptr;
    QDialog *_colorDialog = nullptr;
    QLabel *_statusLabel = nullptr;
    QStatusBar *_statusBar = nullptr;

    PlaylistModel *_playlistModel = nullptr;
    QAbstractItemView *_playlistView = nullptr;
    QString _trackInfo;
    QString _statusInfo;
    qint64 _duration;

    AudioSearchEngine *_audioSearchEngine = nullptr;
};

#endif // PLAYER_H

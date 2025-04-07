#include "audioplayer.h"
#include <QMediaPlayer>
#include <QBuffer>
#include <QAudioOutput>

AudioPlayer::AudioPlayer(QObject *parent) :
    QObject{parent}
{

}

void AudioPlayer::playWavFromMemory(const QByteArray &wavData)
{
    // audioOutput is required
    // https://doc.qt.io/qt-6/audiooverview.html
    QMediaPlayer *player = new QMediaPlayer{this};
    QAudioOutput *audioOutput = new QAudioOutput{this};
    player->setAudioOutput(audioOutput);
    QBuffer *buf = new QBuffer{player};
    buf->setData(wavData);
    player->setSourceDevice(buf, QUrl{"audio/wav"});
    player->play();
    connect(player, &QMediaPlayer::playbackStateChanged, [=](QMediaPlayer::PlaybackState newState) {
        switch (newState) {
        case QMediaPlayer::PlaybackState::StoppedState:
            player->deleteLater();
            audioOutput->deleteLater();
            return;
        default: {
        }
        }
    });
}

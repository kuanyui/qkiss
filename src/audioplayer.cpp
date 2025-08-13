/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2024 - 2025 ono ono (kuanyui) <azazabc123@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

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

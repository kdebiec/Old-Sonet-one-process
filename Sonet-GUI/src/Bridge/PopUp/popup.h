/****************************************************************
 *  This file is part of Sonet.
 *  Sonet is distributed under the following license:
 *
 *  Copyright (C) 2017, Konrad Dębiec
 *
 *  Sonet is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 3
 *  of the License, or (at your option) any later version.
 *
 *  Sonet is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/
#ifndef POPUP_H
#define POPUP_H

//Qt
#include <QQuickWidget>

//libretroshare
#include <retroshare/rspeers.h>

class QSound;

class PopUp : public QQuickWidget
{
    Q_OBJECT
public:
    explicit PopUp(QWidget *parent = 0);
    void setPeerId(QString peerId);

public slots:
    void pressed(int x, int y);
    void destroyWin();
    void hangupReceived(RsPeerId peerId, int flags);

protected:
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    QPoint dragPosition;
    QString rsPeerId;
    QSound *qsound;
};

#endif // POPUP_H

/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2024  qBittorrent project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that are linked to the OpenSSL library), and
 * distribute the linked executables. You must obey the GNU General Public
 * License in all respects for the containing work to be so.  If you modify
 * this file, you may extend this exception to your version of the file, but
 * you are not obligated to do so.  If you do not wish to do so, delete this
 * exception statement from your version.
 */

#pragma once

#include <QList>
#include <QMenu>
#include <QString>

class QAction;
class QKeyEvent;
class QHideEvent;

class FilterableMenu : public QMenu
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FilterableMenu)

public:
    explicit FilterableMenu(const QString &title, QWidget *parent = nullptr);

    void addFilterableAction(QAction *action);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void applyFilter();
    void resetFilter();

    QString m_filter;
    QAction *m_filterLabel = nullptr;
    QList<QAction *> m_filterableActions;
};

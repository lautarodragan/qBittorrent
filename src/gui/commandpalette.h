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

#include <QDialog>
#include <QList>
#include <QString>

class QKeyEvent;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

struct CommandPaletteItem
{
    QString text;
    Qt::CheckState checkState = Qt::Unchecked;
};

class CommandPalette final : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(CommandPalette)

public:
    enum class Mode { SingleSelect, MultiSelect };

    explicit CommandPalette(const QString &title,
                            const QList<CommandPaletteItem> &items,
                            Mode mode,
                            QWidget *parent = nullptr);

    // Valid after exec() returns Accepted (SingleSelect mode only)
    [[nodiscard]] QString selectedText() const;

signals:
    // Emitted immediately on each toggle (MultiSelect mode)
    void itemToggled(const QString &text, Qt::CheckState newState);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void applyFilter(const QString &text);
    void toggleCurrentItem();

    Mode m_mode;
    QLineEdit *m_searchEdit = nullptr;
    QListWidget *m_listWidget = nullptr;
    QString m_selectedText;
};

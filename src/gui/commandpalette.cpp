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

#include "commandpalette.h"

#include <algorithm>

#include <QApplication>
#include <QFrame>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QVBoxLayout>

using namespace Qt::Literals::StringLiterals;

namespace
{
    constexpr int PALETTE_WIDTH = 500;
    constexpr int MAX_VISIBLE_ITEMS = 14;
}

CommandPalette::CommandPalette(const QString &title,
                               const QList<CommandPaletteItem> &items,
                               Mode mode,
                               QWidget *parent)
    : QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint)
    , m_mode(mode)
{
    setFixedWidth(PALETTE_WIDTH);

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    auto *frame = new QFrame(this);
    frame->setObjectName(u"commandPaletteFrame"_s);
    frame->setFrameShape(QFrame::StyledPanel);
    outerLayout->addWidget(frame);

    auto *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    if (!title.isEmpty())
    {
        auto *titleLabel = new QLabel(title, frame);
        titleLabel->setObjectName(u"commandPaletteTitle"_s);
        titleLabel->setContentsMargins(8, 6, 8, 6);
        layout->addWidget(titleLabel);

        auto *sep = new QFrame(frame);
        sep->setFrameShape(QFrame::HLine);
        sep->setFrameShadow(QFrame::Sunken);
        layout->addWidget(sep);
    }

    m_searchEdit = new QLineEdit(frame);
    m_searchEdit->setObjectName(u"commandPaletteSearch"_s);
    m_searchEdit->setPlaceholderText(tr("Type to filter..."));
    m_searchEdit->setFrame(false);
    m_searchEdit->setContentsMargins(4, 4, 4, 4);
    m_searchEdit->setFixedHeight(32);
    layout->addWidget(m_searchEdit);

    auto *sep2 = new QFrame(frame);
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep2);

    m_listWidget = new QListWidget(frame);
    m_listWidget->setObjectName(u"commandPaletteList"_s);
    m_listWidget->setFrameShape(QFrame::NoFrame);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setFocusPolicy(Qt::NoFocus); // keep focus in search box
    layout->addWidget(m_listWidget);

    for (const CommandPaletteItem &item : items)
    {
        auto *lwItem = new QListWidgetItem(item.text, m_listWidget);
        if (m_mode == Mode::MultiSelect)
        {
            lwItem->setFlags(lwItem->flags() | Qt::ItemIsUserCheckable);
            lwItem->setCheckState(item.checkState);
        }
    }

    if (m_listWidget->count() > 0)
        m_listWidget->setCurrentRow(0);

    // Fix list height to avoid jumpy resizing when items are filtered
    const int rowH = std::max(m_listWidget->sizeHintForRow(0), 20);
    const int visible = std::min(m_listWidget->count(), MAX_VISIBLE_ITEMS);
    m_listWidget->setFixedHeight(visible * rowH + 2);

    adjustSize();

    // Center horizontally on parent window, positioned at ~30% from top
    if (QWidget *win = parent ? parent->window() : nullptr)
    {
        const QRect g = win->geometry();
        move(g.left() + (g.width() - width()) / 2,
             g.top() + g.height() * 3 / 10);
    }

    connect(m_searchEdit, &QLineEdit::textChanged, this, &CommandPalette::applyFilter);

    if (m_mode == Mode::MultiSelect)
    {
        connect(m_listWidget, &QListWidget::itemChanged, this, [this](QListWidgetItem *item)
        {
            if (item)
                emit itemToggled(item->text(), item->checkState());
        });
    }

    // Close when clicking outside this window
    qApp->installEventFilter(this);

    m_searchEdit->setFocus();
}

QString CommandPalette::selectedText() const
{
    return m_selectedText;
}

bool CommandPalette::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        const auto *me = static_cast<QMouseEvent *>(event);
        if (!geometry().contains(me->globalPosition().toPoint()))
        {
            reject();
            return false;
        }
    }
    return QDialog::eventFilter(obj, event);
}

void CommandPalette::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
        reject();
        return;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (m_mode == Mode::SingleSelect)
        {
            QListWidgetItem *item = m_listWidget->currentItem();
            if (item && !item->isHidden())
            {
                m_selectedText = item->text();
                accept();
            }
        }
        else
        {
            toggleCurrentItem();
        }
        return;

    case Qt::Key_Space:
        if (m_mode == Mode::MultiSelect)
        {
            toggleCurrentItem();
            return;
        }
        break;

    case Qt::Key_Down:
    {
        const int next = m_listWidget->currentRow() + 1;
        for (int i = next; i < m_listWidget->count(); ++i)
        {
            if (!m_listWidget->item(i)->isHidden())
            {
                m_listWidget->setCurrentRow(i);
                break;
            }
        }
        return;
    }

    case Qt::Key_Up:
    {
        const int prev = m_listWidget->currentRow() - 1;
        for (int i = prev; i >= 0; --i)
        {
            if (!m_listWidget->item(i)->isHidden())
            {
                m_listWidget->setCurrentRow(i);
                break;
            }
        }
        return;
    }

    default:
        break;
    }

    QDialog::keyPressEvent(event);
}

void CommandPalette::applyFilter(const QString &text)
{
    QListWidgetItem *firstVisible = nullptr;
    for (int i = 0; i < m_listWidget->count(); ++i)
    {
        QListWidgetItem *item = m_listWidget->item(i);
        const bool matches = text.isEmpty() || item->text().contains(text, Qt::CaseInsensitive);
        item->setHidden(!matches);
        if (matches && !firstVisible)
            firstVisible = item;
    }
    if (firstVisible)
        m_listWidget->setCurrentItem(firstVisible);
}

void CommandPalette::toggleCurrentItem()
{
    QListWidgetItem *item = m_listWidget->currentItem();
    if (!item || item->isHidden())
        return;
    item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
}

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

#include "filterablemenu.h"

#include <QAction>
#include <QHideEvent>
#include <QKeyEvent>

using namespace Qt::Literals::StringLiterals;

FilterableMenu::FilterableMenu(const QString &title, QWidget *parent)
    : QMenu(title, parent)
{
    m_filterLabel = new QAction(this);
    m_filterLabel->setEnabled(false);
    m_filterLabel->setVisible(false);
    addAction(m_filterLabel);
}

void FilterableMenu::addFilterableAction(QAction *action)
{
    addAction(action);
    m_filterableActions.append(action);
}

void FilterableMenu::keyPressEvent(QKeyEvent *event)
{
    const Qt::KeyboardModifiers mods = event->modifiers();
    const bool hasExtraMods = mods & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);

    if (!hasExtraMods && event->key() == Qt::Key_Backspace)
    {
        if (!m_filter.isEmpty())
        {
            m_filter.chop(1);
            applyFilter();
        }
        return;
    }

    if (event->key() == Qt::Key_Escape)
    {
        if (!m_filter.isEmpty())
        {
            resetFilter();
            return;
        }
    }

    if (!hasExtraMods && !event->text().isEmpty() && event->text().at(0).isPrint())
    {
        m_filter += event->text();
        applyFilter();
        return;
    }

    QMenu::keyPressEvent(event);
}

void FilterableMenu::hideEvent(QHideEvent *event)
{
    resetFilter();
    QMenu::hideEvent(event);
}

void FilterableMenu::applyFilter()
{
    if (m_filter.isEmpty())
    {
        m_filterLabel->setVisible(false);
    }
    else
    {
        m_filterLabel->setText(tr("Filter: %1").arg(m_filter));
        m_filterLabel->setVisible(true);
    }

    QAction *firstMatch = nullptr;
    for (QAction *action : m_filterableActions)
    {
        const bool matches = m_filter.isEmpty()
            || action->text().contains(m_filter, Qt::CaseInsensitive);
        action->setVisible(matches);
        if (matches && !firstMatch)
            firstMatch = action;
    }

    if (firstMatch)
        setActiveAction(firstMatch);
}

void FilterableMenu::resetFilter()
{
    m_filter.clear();
    applyFilter();
}

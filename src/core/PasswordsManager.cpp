/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2015 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
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
*
**************************************************************************/

#include "PasswordsManager.h"
#include "../ui/WebWidget.h"

namespace Otter
{

PasswordsManager* PasswordsManager::m_instance = NULL;

PasswordsManager::PasswordsManager(QObject *parent) : QObject(parent)
{
}

void PasswordsManager::createInstance(QObject *parent)
{
	if (!m_instance)
	{
		m_instance = new PasswordsManager(parent);
	}
}

void PasswordsManager::addPassword(const QUrl &url, const QMap<QString, QString> &values, PasswordsManager::PasswordType type)
{
	Q_UNUSED(url)
	Q_UNUSED(values)
	Q_UNUSED(type)
}

void PasswordsManager::applyPassword(WebWidget *widget)
{
	Q_UNUSED(widget)
}

}

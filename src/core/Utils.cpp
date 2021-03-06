/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2015 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
* Copyright (C) 2014 Jan Bajer aka bajasoft <jbajer@gmail.com>
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

#include "Utils.h"
#include "Application.h"
#include "PlatformIntegration.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QRegularExpression>
#include <QtCore/QTime>
#include <QtCore/QtMath>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

namespace Otter
{

namespace Utils
{

void runApplication(const QString &command, const QUrl &url)
{
	if (command.isEmpty() && !url.isValid())
	{
		return;
	}

	PlatformIntegration *integration = Application::getInstance()->getPlatformIntegration();

	if (integration)
	{
		integration->runApplication(command, url);
	}
	else
	{
		QDesktopServices::openUrl(QUrl(url));
	}
}

QString createIdentifier(const QString &base, const QStringList &exclude, bool toLowerCase)
{
	QString identifier;

	if (!base.isEmpty())
	{
		identifier = base;

		if (toLowerCase)
		{
			identifier = base.toLower();
		}

		identifier.remove(QRegularExpression(QLatin1String("[^A-Za-z0-9\\-_]")));
	}

	if (identifier.isEmpty())
	{
		identifier = QLatin1String("custom");
	}

	if (!exclude.contains(identifier))
	{
		return identifier;
	}

	int number = 2;

	const QRegularExpression expression(QLatin1String("_([0-9]+)$"));
	const QRegularExpressionMatch match = expression.match(identifier);

	if (match.hasMatch())
	{
		identifier.remove(expression);

		number = match.captured(1).toInt();
	}

	while (exclude.contains(identifier + QLatin1Char('_') + QString::number(number)))
	{
		++number;
	}

	return identifier + QLatin1Char('_') + QString::number(number);
}

QString elideText(const QString &text, QWidget *widget, int width)
{
	if (width < 0)
	{
		width = (QApplication::desktop()->screenGeometry(widget).width() / 4);
	}

	return (widget ? widget->fontMetrics() : QApplication::fontMetrics()).elidedText(text, Qt::ElideRight, qMax(100, width));
}

QString formatConfigurationEntry(const QLatin1String &key, const QString &value, bool quote)
{
	QString escapedValue(value);
	escapedValue.replace(QLatin1Char('\n'), QLatin1String("\\n"));

	if (quote)
	{
		return QStringLiteral("%1=\"%2\"\n").arg(key).arg(escapedValue.replace(QLatin1Char('\"'), QLatin1String("\\\"")));
	}

	return QStringLiteral("%1=%2\n").arg(key).arg(escapedValue);
}

QString formatTime(int value)
{
	QTime time(0, 0);
	time = time.addSecs(value);

	if (value > 3600)
	{
		QString string = time.toString(QLatin1String("hh:mm:ss"));

		if (value > SECONDS_IN_DAY)
		{
			string = QCoreApplication::translate("utils", "%n days %1", "", (qFloor(static_cast<qreal>(value) / SECONDS_IN_DAY))).arg(string);
		}

		return string;
	}

	return time.toString(QLatin1String("mm:ss"));
}

QString formatDateTime(const QDateTime &dateTime, const QString &format)
{
	return (format.isEmpty() ? QLocale().toString(dateTime, QLocale::ShortFormat) : QLocale().toString(dateTime, format));
}

QString formatUnit(qint64 value, bool isSpeed, int precision)
{
	if (value < 0)
	{
		return QString('?');
	}

	if (value > 1024)
	{
		if (value > 1048576)
		{
			if (value > 1073741824)
			{
				return QCoreApplication::translate("utils", (isSpeed ? "%1 GB/s" : "%1 GB")).arg((value / 1073741824.0), 0, 'f', precision);
			}

			return QCoreApplication::translate("utils", (isSpeed ? "%1 MB/s" : "%1 MB")).arg((value / 1048576.0), 0, 'f', precision);
		}

		return QCoreApplication::translate("utils", (isSpeed ? "%1 KB/s" : "%1 KB")).arg((value / 1024.0), 0, 'f', precision);
	}

	return QCoreApplication::translate("utils", (isSpeed ? "%1 B/s" : "%1 B")).arg(value);
}

QIcon getIcon(const QString &name, bool fromTheme)
{
	if (fromTheme && QIcon::hasThemeIcon(name))
	{
		return QIcon::fromTheme(name);
	}

	return QIcon(QStringLiteral(":/icons/%1.png").arg(name));
}

QList<ApplicationInformation> getApplicationsForMimeType(const QMimeType &mimeType)
{
	PlatformIntegration *integration = Application::getInstance()->getPlatformIntegration();

	if (integration)
	{
		return integration->getApplicationsForMimeType(mimeType);
	}

	return QList<ApplicationInformation>();
}

bool isUrlEmpty(const QUrl &url)
{
	return (url.isEmpty() || (url.scheme() == QLatin1String("about") && (url.path().isEmpty() || url.path() == QLatin1String("blank") || url.path() == QLatin1String("start"))));
}

}

}

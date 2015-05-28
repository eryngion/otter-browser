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

#include "SourceViewerWidget.h"
#include "../core/SettingsManager.h"

#include <QtGui/QPainter>
#include <QtGui/QTextBlock>

namespace Otter
{

MarginWidget::MarginWidget(SourceViewerWidget *parent) : QWidget(parent),
	m_sourceViewer(parent),
	m_lastClickedLine(-1)
{
	setAmount(m_sourceViewer->blockCount());
	setContextMenuPolicy(Qt::NoContextMenu);

	connect(m_sourceViewer, SIGNAL(blockCountChanged(int)), this, SLOT(setAmount(int)));
	connect(m_sourceViewer, SIGNAL(textChanged()), this, SLOT(setAmount()));
	connect(m_sourceViewer, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateNumbers(QRect,int)));
}

void MarginWidget::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.fillRect(event->rect(), Qt::transparent);

	QTextBlock block = m_sourceViewer->firstVisibleBlock();
	int top = (int) m_sourceViewer->blockBoundingGeometry(block).translated(m_sourceViewer->contentOffset()).top();
	int bottom = (top + (int) m_sourceViewer->blockBoundingRect(block).height());
	const int right = (width() - 5);
	const int left = (width() - 2);
	const int selectionStart = m_sourceViewer->document()->findBlock(m_sourceViewer->textCursor().selectionStart()).blockNumber();
	const int selectionEnd = m_sourceViewer->document()->findBlock(m_sourceViewer->textCursor().selectionEnd()).blockNumber();

	while (block.isValid() && top <= event->rect().bottom())
	{
		if (block.isVisible() && bottom >= event->rect().top())
		{
			painter.setPen(palette().color(QPalette::Window).darker((block.blockNumber() >= selectionStart && block.blockNumber() <= selectionEnd) ? 250 : 150));
			painter.drawText(0, top, right, fontMetrics().height(), Qt::AlignRight, QString::number(block.blockNumber() + 1));
		}

		block = block.next();
		top = bottom;
		bottom = (top + (int) m_sourceViewer->blockBoundingRect(block).height());
	}
}

void MarginWidget::mouseMoveEvent(QMouseEvent *event)
{
	QTextCursor textCursor = m_sourceViewer->cursorForPosition(QPoint(1, event->y()));
	int currentLine = textCursor.blockNumber();

	if (currentLine == m_lastClickedLine)
	{
		return;
	}

	textCursor.movePosition(((currentLine > m_lastClickedLine) ? QTextCursor::Up : QTextCursor::Down), QTextCursor::KeepAnchor, qAbs(m_lastClickedLine - currentLine));

	m_sourceViewer->setTextCursor(textCursor);
}

void MarginWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		QTextCursor textCursor = m_sourceViewer->cursorForPosition(QPoint(1, event->y()));
		textCursor.select(QTextCursor::LineUnderCursor);

		m_lastClickedLine = textCursor.blockNumber();

		m_sourceViewer->setTextCursor(textCursor);
	}
	else
	{
		QWidget::mousePressEvent(event);
	}
}

void MarginWidget::mouseReleaseEvent(QMouseEvent *event)
{
	m_lastClickedLine = -1;

	QWidget::mouseReleaseEvent(event);
}

void MarginWidget::updateNumbers(const QRect &rectangle, int offset)
{
	if (offset)
	{
		 scroll(0, offset);
	}
	else
	{
		update(0, rectangle.y(), width(), rectangle.height());
	}
}

void MarginWidget::setAmount(int amount)
{
	if (amount < 0 && m_sourceViewer)
	{
		amount = m_sourceViewer->blockCount();
	}

	int digits = 1;
	int max = qMax(1, amount);

	while (max >= 10)
	{
		max /= 10;

		++digits;
	}

	setFixedWidth(6 + (fontMetrics().width(QLatin1Char('9')) * digits));

	m_sourceViewer->setViewportMargins(width(), 0, 0, 0);
}

bool MarginWidget::event(QEvent *event)
{
	const bool value = QWidget::event(event);

	if (event->type() == QEvent::FontChange)
	{
		setAmount();
	}

	return value;
}

SourceViewerWidget::SourceViewerWidget(QWidget *parent) : QPlainTextEdit(parent),
	m_marginWidget(NULL),
	m_findFlags(WebWidget::NoFlagsFind),
	m_zoom(100)
{
	optionChanged(QLatin1String("SourceViewer/ShowLineNumbers"), SettingsManager::getValue(QLatin1String("SourceViewer/ShowLineNumbers")));
	optionChanged(QLatin1String("SourceViewer/WrapLines"), SettingsManager::getValue(QLatin1String("SourceViewer/WrapLines")));

	connect(this, SIGNAL(textChanged()), this, SLOT(updateSelection()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateTextCursor()));
	connect(SettingsManager::getInstance(), SIGNAL(valueChanged(QString,QVariant)), this, SLOT(optionChanged(QString,QVariant)));
}

void SourceViewerWidget::resizeEvent(QResizeEvent *event)
{
	QPlainTextEdit::resizeEvent(event);

	if (m_marginWidget)
	{
		m_marginWidget->setGeometry(QRect(contentsRect().left(), contentsRect().top(), m_marginWidget->width(), contentsRect().height()));
	}
}

void SourceViewerWidget::wheelEvent(QWheelEvent *event)
{
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		setZoom(getZoom() + (event->delta() / 16));

		event->accept();

		return;
	}

	QPlainTextEdit::wheelEvent(event);
}

void SourceViewerWidget::optionChanged(const QString &option, const QVariant &value)
{
	if (option == QLatin1String("SourceViewer/ShowLineNumbers"))
	{
		if (value.toBool() && !m_marginWidget)
		{
			m_marginWidget = new MarginWidget(this);
			m_marginWidget->show();
			m_marginWidget->setGeometry(QRect(contentsRect().left(), contentsRect().top(), m_marginWidget->width(), contentsRect().height()));
		}
		else if (!value.toBool() && m_marginWidget)
		{
			m_marginWidget->deleteLater();
			m_marginWidget = NULL;

			setViewportMargins(0, 0, 0, 0);
		}
	}
	else if (option == QLatin1String("SourceViewer/WrapLines"))
	{
		setLineWrapMode(value.toBool() ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
	}
}

void SourceViewerWidget::updateTextCursor()
{
	m_findTextAnchor = textCursor();
}

void SourceViewerWidget::updateSelection()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!m_findText.isEmpty())
	{
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(QColor(255, 150, 50));
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = m_findTextSelection;

		extraSelections.append(selection);

		QTextCursor textCursor = this->textCursor();
		textCursor.setPosition(0);

		if (m_findFlags.testFlag(WebWidget::HighlightAllFind))
		{
			QTextDocument::FindFlags nativeFlags;

			if (m_findFlags.testFlag(WebWidget::CaseSensitiveFind))
			{
				nativeFlags |= QTextDocument::FindCaseSensitively;
			}

			while (!textCursor.isNull())
			{
				textCursor = document()->find(m_findText, textCursor, nativeFlags);

				if (!textCursor.isNull() && textCursor != m_findTextSelection)
				{
					QTextEdit::ExtraSelection selection;
					selection.format.setBackground(QColor(255, 255, 0));
					selection.cursor = textCursor;

					extraSelections.append(selection);
				}
			}
		}
	}

	setExtraSelections(extraSelections);
}

void SourceViewerWidget::setZoom(int zoom)
{
	if (zoom != m_zoom)
	{
		m_zoom = zoom;

		QFont font = parentWidget()->font();
		font.setPointSize(font.pointSize() * (qreal(zoom) / 100));

		setFont(font);

		emit zoomChanged(zoom);
	}
}

int SourceViewerWidget::getZoom() const
{
	return m_zoom;
}

bool SourceViewerWidget::findText(const QString &text, WebWidget::FindFlags flags)
{
	const bool isTheSame = (text == m_findText);

	m_findText = text;
	m_findFlags = flags;

	if (!text.isEmpty())
	{
		QTextDocument::FindFlags nativeFlags;

		if (flags.testFlag(WebWidget::BackwardFind))
		{
			nativeFlags |= QTextDocument::FindBackward;
		}

		if (flags.testFlag(WebWidget::CaseSensitiveFind))
		{
			nativeFlags |= QTextDocument::FindCaseSensitively;
		}

		QTextCursor findTextCursor = m_findTextAnchor;

		if (!isTheSame)
		{
			findTextCursor = textCursor();
		}
		else if (!flags.testFlag(WebWidget::BackwardFind))
		{
			findTextCursor.setPosition(findTextCursor.selectionEnd(), QTextCursor::MoveAnchor);
		}

		m_findTextAnchor = document()->find(text, findTextCursor, nativeFlags);

		if (m_findTextAnchor.isNull())
		{
			m_findTextAnchor = textCursor();
			m_findTextAnchor.setPosition((flags.testFlag(WebWidget::BackwardFind) ? (document()->characterCount() - 1) : 0), QTextCursor::MoveAnchor);
			m_findTextAnchor = document()->find(text, m_findTextAnchor, nativeFlags);
		}

		if (!m_findTextAnchor.isNull())
		{
			const QTextCursor currentTextCursor = textCursor();

			disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateTextCursor()));

			setTextCursor(m_findTextAnchor);
			ensureCursorVisible();
			setTextCursor(currentTextCursor);

			connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateTextCursor()));
		}
	}

	m_findTextSelection = m_findTextAnchor;

	updateSelection();

	return !m_findTextAnchor.isNull();
}

}
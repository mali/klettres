/***************************************************************************
 *   Copyright (C) 2001-2006 by Anne-Marie Mahfouf                              *
 *   annemarie.mahfouf@free.fr                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#include <QLabel>
#include <QTimer>
#include <QToolTip>
#include <QWhatsThis>
#include <QPainter>

#include <kapplication.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <krandom.h>
//Project headers
#include "klettres.h"
#include "klettresview.h"
#include "prefs.h"

KLettresView::KLettresView(KLettres *parent)
        : QWidget(parent)
{
    m_klettres = parent;

    setMinimumSize( QSize( 640, 480 ) );
    setMaximumSize( QSize( 640, 480 ) );

    //lineEdit for user input
    m_letterEdit = new KLineEdit( this );
    m_letterEdit->setGeometry( QRect( 40, 310, 161, 160 ) );
    m_letterEdit->setToolTip(i18n("Type the letter or syllable that you just heard" ) );

    setFont(Prefs::font());
    //load background pics
    m_grownupPicture.load(KStandardDirs::locate("data","klettres/pics/klettres_grownup.png"));
    m_kidPicture.load(locate("data","klettres/pics/klettres_kids.jpeg"));
    m_desertPicture.load(locate("data","klettres/pics/klettres_desert.png"));
    setAutoFillBackground(true);
}

KLettresView::~KLettresView()
{
}

void KLettresView::chooseSound()
{
    //If there are no sounds loaded
    if (m_klettres->soundFactory->sounds ==0)
        return;
    //get a random sound
    m_random=KRandom::random()%(m_klettres->soundFactory->sounds);
    //have not 2 same sounds consecutively
    int temp = 0;
#ifdef __GNUC__
#warning this is dead code!
#endif	
    if (temp<0)
        temp=m_random;
    else
    {
        while (m_random==temp)
            m_random=KRandom::random()%(m_klettres->soundFactory->sounds);
        temp=m_random;
    }

    //The sound is played
    kDebug() << "m_random " << m_random << endl;
    m_klettres->soundFactory->playSound(m_random);
    //store letter or syllable in m_currentLetter
    m_currentLetter = m_klettres->soundFactory->namesList[m_random];
    //Find the length of the syllable
    m_length=m_klettres->soundFactory->namesList[m_random].length();
    kDebug() << "syllable length " << m_length << endl;
    int width;
    if (m_length<3)
        width = 200;
    else
        width = 200+(20*(m_length-2));
    update();
    m_letterEdit->setMinimumSize( QSize( width, 160 ) );
    m_letterEdit->setMaximumSize( QSize( width, 160 ) );
}

void KLettresView::paintEvent( QPaintEvent * )
{
    if (Prefs::level()%2==1) {
        QPainter paint(this);
        paint.setFont(Prefs::font());
        QString start = i18n("Start");

        paint.setFont(Prefs::font());
        if (Prefs::theme() == Prefs::EnumTheme::desert) {
            paint.setPen( QColor(115, 50, 95)); //brown
            //m_letterEdit->setPaletteForegroundColor(QColor(115, 50, 95));
        }
        else if (Prefs::theme() == Prefs::EnumTheme::arctic) {
            paint.setPen( Qt::white );
            //m_letterEdit->setPaletteForegroundColor(QColor(48, 75, 137));
        }
        else {
            paint.setPen( Qt::white );
            //m_letterEdit->setPaletteForegroundColor(Qt::black);
        }
        paint.drawText(50, 230, m_currentLetter);
    }
}


void KLettresView::game()
{
    m_cursorPos =1;
    //reset everything so when you change language or levels
    //it all restarts nicely
    QObject::disconnect(m_letterEdit, SIGNAL(textChanged(const QString&)),this,SLOT(slotProcess(const QString&)) );
    m_letterEdit->clear();
    m_letterEdit->setCursorPosition(0);
    m_letterEdit->setFocus();
    chooseSound();
    QObject::connect(m_letterEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotProcess(const QString&)) );
}

void KLettresView::slotProcess(const QString &inputLetter)
{
    QObject::disconnect(m_letterEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotProcess(const QString&)) );
    kDebug() << "Input: " << inputLetter << endl;
    m_inputLetter=m_letterEdit->text();
    if (m_inputLetter.at(0).isLetter()) //(a1.at(inputLetter.length()).lower().isLetter())
    {
        m_upperLetter = m_inputLetter.toUpper();    //put it in uppercase
        m_letterEdit->selectAll();
        m_letterEdit->cut();
        m_letterEdit->setText(m_upperLetter);
        QTimer *timer = new QTimer( this );
        connect( timer, SIGNAL(timeout()), this, SLOT(slotTimerDone()) );
        timer->start( m_timer*100);
    }
    else if (m_inputLetter.length() < m_cursorPos)
    {
        kDebug() << "In backspace case !!! " << endl;
        m_cursorPos--;
        QObject::connect(m_letterEdit, SIGNAL(textChanged(const QString&)),this,SLOT(slotProcess(const QString&)) );
    }
    else
    {
        kDebug() << "in no char loop" << endl;
        kDebug() << "cursor " << m_cursorPos << endl;
        m_letterEdit->backspace();
        QObject::connect(m_letterEdit, SIGNAL(textChanged(const QString&)),this,SLOT(slotProcess(const QString&)) );
    }
}

void KLettresView::slotTimerDone()
{
    kDebug() << "in timer done" << endl;
    QString match = m_currentLetter.left(m_cursorPos );
    if (match == m_upperLetter)
    {
        if (m_cursorPos!=m_length)  //if text in lineEdit not equal to text on button
        {            //i.e if you still have to allow input
            m_letterEdit->setMaxLength( m_cursorPos +1 );
            m_letterEdit->setCursorPosition( m_cursorPos );
            m_letterEdit->setFocus();
            m_cursorPos ++;
            QObject::connect(m_letterEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotProcess(const QString&)) );
        }
        else
        {
            m_letterEdit->selectAll();
            m_letterEdit->cut();
            m_letterEdit->setCursorPosition(0 );
            m_letterEdit->setFocus();
            m_letterEdit->setMaxLength( 1 );
            game();  //another syllable
        }
    }
    else   //if not, cut it
    {
        m_letterEdit->backspace();  //delete the char to the left  and position curseur accordingly
        m_letterEdit->setFocus();
        //play sound again
        m_klettres->soundFactory->playSound(m_random);

        QObject::connect(m_letterEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotProcess(const QString&)) );
    }
}

void KLettresView::slotPlayAgain()
{
    //TODO wait for the previous sound to be payed before playing again as it won't play if the previous one was not finished
    m_klettres->soundFactory->playSound(m_random);
}

void KLettresView::viewThemeArctic()
{
    Prefs::setTheme(Prefs::EnumTheme::arctic);
    Prefs::writeConfig();
    pal.setBrush(QPalette::Window, m_grownupPicture);
    setPalette(pal);
}

void KLettresView::viewThemeClassroom()
{
    Prefs::setTheme(Prefs::EnumTheme::classroom);
    Prefs::writeConfig();
    pal.setBrush(QPalette::Window, m_kidPicture);
    setPalette(pal);
}

void KLettresView::viewThemeDesert()
{
    Prefs::setTheme(Prefs::EnumTheme::desert);
    Prefs::writeConfig();
    pal.setBrush(QPalette::Window, m_desertPicture);
    setPalette(pal);
}

#include "klettresview.moc"

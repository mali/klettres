/*
 * Copyright (C) 2003 Anne-Marie Mahfouf <annma@kde.org>
 */

//Qt headers
#include <qtimer.h>
#include <qtooltip.h>
//KDE headers
#include <kdebug.h>
#include <klocale.h>
//C++ includes
#include <stdlib.h>
#include <string.h>
#include <time.h>
//Project headers
#include "klettres.h"
#include "klettresview.h"


KLettresView::KLettresView(KLettres *parent)
    : QWidget(parent)
{
    klettres = parent;

    setMinimumSize( QSize( 640, 538 ) );
    setMaximumSize( QSize( 640, 538 ) );

    //Button with letter or syllable
    button1 = new QLabel( this, "button1" );
    button1->setGeometry( QRect( 50, 100, 160, 160 ) );
    cg.setColor( QColorGroup::Foreground, white );
    cg.setColor( QColorGroup::Background, QColor(53,87,158));
    pal.setActive( cg );
    button1->setPalette( pal );
    button1->setText( i18n( "A" ) );
    QToolTip::add( button1, i18n( "You must type the letter you hear and/or see in the field below" ) );
    //lineEdit for user input
    line1 = new QLineEdit( this, "line1" );
    line1->setGeometry( QRect( 40, 310, 161, 160 ) );
    QToolTip::add( line1, i18n( "Type the letter that you just heard" ) );

    //load background pics
    pm_a.load(locate("data","klettres/pics/background1.png"));
    pm_k.load(locate("data","klettres/pics/klettres_back.jpeg"));
    //maybe a warning if background pics are not found
    n = 0;
    temp=-1;

}

KLettresView::~KLettresView()
{
}

///Set the GUI for the grownup look
void KLettresView::slotGrownup()
{
    style="grownup";
    setMinimumSize( QSize( 640, 480 ) );
    setMaximumSize( QSize( 640, 480 ) );
    setBackgroundPixmap(pm_a);
    //button1 background
    cg.setColor( QColorGroup::Foreground, white );
    cg.setColor( QColorGroup::Background, QColor(53,87,158));
    pal.setActive( cg );
    button1->setPalette( pal );
}

///Set GUI for kid look
void KLettresView::slotKid()
{
    style="kid";
    setMinimumSize( QSize( 640, 480 ) );
    setMaximumSize( QSize( 640, 480 ) );
    setBackgroundPixmap(pm_k);
    //change button1 background
    cg.setColor( QColorGroup::Foreground, white );
    cg.setColor( QColorGroup::Background, black);
    pal.setActive( cg );
    button1->setPalette( pal );
}

///Start the prog
void KLettresView::game()
{
 //reset everything so when you change language or levels
 //it all restart nicely
 QObject::disconnect(line1, SIGNAL(textChanged(const
 QString&)),this,SLOT(slotLet2(const QString&)) );
 input = 1;
 line1->clear();
 line1->setCursorPosition(0);
 line1->setFocus();

 if (niveau==1)
	button1->show();

    if (niveau==2)
	button1->hide();

    if (niveau==1||niveau==2)
    {
	button1->setMinimumSize( QSize( 200, 160 ) );
	button1->setMaximumSize( QSize( 200, 160 ) );
	line1->setMinimumSize( QSize( 140, 160 ) );
	line1->setMaximumSize( QSize( 140, 160 ) );
	chooseSound();

    	QObject::connect(line1, SIGNAL(textChanged(const
 QString&)),this,SLOT(treat1(const QString&)) );
   	 QObject::disconnect(line1, SIGNAL(textChanged(const
 QString&)),this,SLOT(slotLet2(const QString&)) );
    }

    if (niveau==3)
	button1->show();

    if (niveau==4)
	button1->hide();

    if (niveau==3 || niveau==4)
    {
        chooseSound();
	if (length==2)
	{
		button1->setMinimumSize( QSize( 200, 160 ) );
		button1->setMaximumSize( QSize( 200, 160 ) );
		line1->setMinimumSize( QSize( 200, 160 ) );
		line1->setMaximumSize( QSize( 200, 160 ) );
	}
       	if (length==3)
	{
		button1->setMinimumSize( QSize(250, 160 ) );
		button1->setMaximumSize( QSize( 250, 160 ) );
		line1->setMinimumSize( QSize( 250, 160 ) );
		line1->setMaximumSize( QSize( 250, 160 ) );
	}
	QObject::connect(line1, SIGNAL(textChanged(const
 QString&)),this,SLOT(slotLet2(const QString&)) );
	QObject::disconnect(line1, SIGNAL(textChanged(const
 QString&)),this,SLOT(treat1(const QString&)) );
	}
	line1->setMaxLength( 1 );
	line1->setCursorPosition(0);
	line1->setFocus();
}

void KLettresView::treat1(const QString& )
{
	QObject::disconnect(line1, SIGNAL(textChanged(const
 QString&)),this,SLOT(treat1(const QString&)) );
	QObject::disconnect(line1, SIGNAL(textChanged(const
 QString&)),this,SLOT(slotLet2(const QString&)) );
	a1=line1->text();   //get text from LineEdit
	if (!a1.at(0).isLetter()) //if it's not a letter which was typed
	{
	QObject::connect(line1, SIGNAL(textChanged(const
 QString&)),this,SLOT(treat1(const QString&)) );
	QObject::connect(line1, SIGNAL(textChanged(const
 QString&)),this,SLOT(slotLet2(const QString&)) );
	}
   	t1 = a1.upper();    //put it in uppercase
	line1->selectAll();
	line1->cut();
	line1->setText(t1);     //display it in uppercase
	if (niveau==2)
		button1->hide();
	QTimer *timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()),
	         this, SLOT(timer1()) );
	timer->start( 2000, TRUE );
}

void KLettresView::timer1()
{
	line1->selectAll();
	if ((button1->text())==t1)
	{
		chooseSound();
	}
	else
	{
		if (niveau==2)
		button1->show(); //show letter after first miss
		klettres->soundFactory->playSound(n);//replay sound
	}

	line1->cut();

	QObject::connect(line1, SIGNAL(textChanged(const
 				QString&)),this,SLOT(treat1(const QString&)) );
	line1->setFocus();
}

//levels 3 and 4
void KLettresView::slotLet2(const QString& )
{
	line1->clearFocus();
	//disconnect
	QObject::disconnect(line1, SIGNAL(textChanged(const
 		QString&)),this,SLOT(slotLet2(const QString&)) );

	line1->setSelection(input-1,1);
	a1=line1->selectedText();   //get the input letter
	t1 = a1.upper(); //input in uppercase
	line1->cut();
	line1->setText(line1->text()+t1);
  	t1=line1->text(); //t1 is the whole lineEdit text now
	sj=st.left(input);
	QTimer *timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()),
	         this, SLOT(timerDone()) );
	timer->start( 1000, TRUE );
}

void KLettresView::timerDone()
{
	if (t1==sj)   //if letter input is correct
	{
	   if (sj!=st)  //if text in lineEdit not equal to text on button
		{            //i.e if you still have to allow input
			line1->setMaxLength( input+1 );
			line1->setCursorPosition( input );
			line1->setFocus();
			input++;
			QObject::connect(line1, SIGNAL(textChanged(const
 					QString&)),this,SLOT(slotLet2(const QString&)) );
       }
		else
		{
			line1->selectAll();
			line1->cut();
			line1->setCursorPosition(0 );
			line1->setFocus();
			line1->setMaxLength( 1 );
      			if (niveau==4)
			button1->hide();
			game();  //another syllable
		}
	}
	else   //if not, cut it
	{
		line1->backspace();  //delete the char to the left  and position curseur accordingly
		line1->setFocus();
		//play sound again
		klettres->soundFactory->playSound(n);

		QObject::connect(line1, SIGNAL(textChanged(const
 QString&)),this,SLOT(slotLet2(const QString&)) );
	}
}

void KLettresView::chooseSound()
{
	input =1;
	srand((unsigned int)time((time_t *)NULL));
        //If there are no sounds loaded
        if (klettres->soundFactory->sounds ==0)
        	return;
	n=rand()%(klettres->soundFactory->sounds);//l;
	//have not 2 same sounds consecutively
	if (temp<0)
		temp=n;
	else
	{
		while (n==temp)
			n=rand()%(klettres->soundFactory->sounds);
		temp=n;
	}

        //The sound is played
        klettres->soundFactory->playSound(n);
        //The letter/syllable is displayed
        button1->setText(klettres->soundFactory->namesList[n]);
        //store letter or syllable in st
        st = klettres->soundFactory->namesList[n];
        //Find the length of the syllable
        length=klettres->soundFactory->namesList[n].length();
}

#include "klettresview.moc"

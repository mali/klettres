/***************************************************************************
 *   Copyright 2001-2008 by Anne-Marie Mahfouf                              *
 *   annma@kde.org                                             *
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

#include "klettres.h"

//Qt includes
#include <QBitmap>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QPainter>
#include <QTextStream>
#include <QDomDocument>
#include <QWidget>

//KDE includes
#include <KAction>
#include <KActionCollection>
#include <KComboBox>
#include <KConfigDialog>
#include <KLocale>
#include <KMenuBar>
#include <KMessageBox>
#include <KNumInput>
#include <KSelectAction>
#include <KStandardAction>
#include <KStandardDirs>
#include <KStatusBar>
#include <KToggleAction>
#include <KToolBar>
#include <KIcon>
#include <KApplication>
#include <KGlobal>

#include <knewstuff3/downloaddialog.h>
//Project includes
#include "ui_fontsdlg.h"
#include "timer.h"
#include "prefs.h"
#include "langutils.h"
#include "kltheme.h"

class fontsdlg : public QWidget, public Ui::fontsdlg
{    
    public:
        fontsdlg( QWidget * parent ) : QWidget(parent)
        {
            setupUi(this);
        }
};

const int ID_KIDB      = 100;
const int ID_GROWNB    = 101;
const int ID_MENUBARB  = 102;

KLettres::KLettres()
        : KXmlGuiWindow( 0)
{
    setObjectName( QLatin1String("KLettres") );
    mNewStuff = 0;
    m_view = new KLettresView(this);
    setMinimumSize( QSize( 800, 600 ) );
    //Tell the KXmlGuiWindow that this is indeed the main widget
    setCentralWidget(m_view);
    //Populate Languages menu with m_languageNames
    m_languageNames = LangUtils::getLanguagesNames(LangUtils::getLanguages());
    //MainWindow GUI: menus, tolbars and statusbar
    setupActions();
    setupStatusbar();
    setupToolbars();
    //Load Settings
    loadSettings();
    //Setup current language sounds
    soundFactory = new SoundFactory(this, "sounds");
    //Start game
    m_view->game();
}

KLettres::~KLettres()
{
}

bool KLettres::loadLayout(QDomDocument &layoutDocument)
{
    QFile layoutFile(KStandardDirs::locate("data", "klettres/"+Prefs::language()+"/sounds.xml"));
    //if xml file is not found, program exits
    if (!layoutFile.exists()) {
        kWarning() << "sounds.xml file not found in $KDEDIR/share/apps/klettres/"+Prefs::language() ;
        QString mString=i18n("The file sounds.xml was not found in\n"
                             "$KDEDIR/share/apps/klettres/\n\n"
                             "Please install this file and start KLettres again.\n\n");
        KMessageBox::information( this, mString,i18n("KLettres - Error") );
        qApp->quit();//exit(1);
    }
    
    if (!layoutFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    //Check if document is well-formed
    if (!layoutDocument.setContent(&layoutFile)) {
        layoutFile.close();
        return false;
    }
    
    layoutFile.close();

    return true;
}

void KLettres::setupActions()
{
    KAction *m_newAction = actionCollection()->addAction("play_new");
    m_newAction->setText(i18n("New Sound"));
    m_newAction->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_N));
    m_newAction->setIcon(KIcon("document-new")); // ### better icon for this?
    connect(m_newAction, SIGNAL(triggered(bool)), m_view, SLOT(game()));
    m_newAction->setToolTip(i18n("Play a new sound"));
    m_newAction->setWhatsThis(i18n("You can play a new sound by clicking this button or using the File menu, New Sound."));

    QAction *m_stuffAction = actionCollection()->addAction("downloadnewstuff");
    m_stuffAction->setText(i18n("Get Alphabet in New Language..."));
    m_stuffAction->setIcon(KIcon("get-hot-new-stuff"));
    connect(m_stuffAction, SIGNAL(triggered(bool)), this, SLOT(slotDownloadNewStuff()));

    KAction *m_playAgainAction = actionCollection()->addAction("play_again");
    m_playAgainAction->setText(i18n("Replay Sound"));
    m_playAgainAction->setShortcut(QKeySequence(Qt::Key_F5));
    m_playAgainAction->setIcon(KIcon("media-playback-start"));
    m_playAgainAction->setToolTip(i18n("Play the same sound again"));
    connect(m_playAgainAction, SIGNAL(triggered(bool)), m_view, SLOT(slotPlayAgain()));
    m_playAgainAction->setWhatsThis(i18n("You can replay the same sound again by clicking this button or using the File menu, Replay Sound."));
    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

    m_menubarAction = KStandardAction::showMenubar(this, SLOT(slotMenubar()), actionCollection());

    m_levelAction = actionCollection()->add<KSelectAction>("levels");
    m_levelAction->setText(i18nc("@label:listbox which difficulty level to use", "L&evel"));
    m_levelAction->setToolTip(i18n("Select the level"));
    m_levelAction->setWhatsThis(i18n("You can select the level: level 1 displays a letter and you hear it; level 2 does not display the letter, you only hear it; level 3 displays a syllable and you hear it; level 4 does not display the syllable, you only hear it."));

    m_languageAction = actionCollection()->add<KSelectAction>("languages");
    m_languageAction->setText(i18nc("@label:listbox", "&Language"));
    m_languageAction->setItems(m_languageNames);

    m_levelsNames.append(i18ncp("@item:inlistbox choose level", "Level %1" , "Level %1", 1));
    m_levelsNames.append(i18ncp("@item:inlistbox choose level", "Level %1" , "Level %1", 2));
    m_levelsNames.append(i18ncp("@item:inlistbox choose level", "Level %1" , "Level %1", 3));
    m_levelsNames.append(i18ncp("@item:inlistbox choose level", "Level %1" , "Level %1", 4));
    m_levelAction->setItems(m_levelsNames);
    m_levelAction->setCurrentItem(Prefs::level()-1);

    m_themeAction = actionCollection()->add<KSelectAction>("looks");
    m_themeAction->setText(i18n("Themes"));
    m_themeAction->setItems(KLThemeFactory::instance()->themeList());
    m_themeAction->setCurrentItem(Prefs::theme());
    m_themeAction->setToolTip(i18n("Select the theme"));
    m_themeAction->setWhatsThis(i18n("Here you can change the theme for KLettres. A theme consists in the background picture and the font color for the letter displayed."));

    m_kidAction = actionCollection()->add<KToggleAction>("mode_kid");
    m_kidAction->setText(i18n("Mode Kid"));
    m_kidAction->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_K));
    m_kidAction->setIcon(KIcon("klettres_kids"));
    connect(m_kidAction, SIGNAL(triggered(bool)), this, SLOT(slotModeKid()));
    m_kidAction->setWhatsThis(i18n("If you are in the Grown-up mode, clicking on this button will set up the Kid mode. The Kid mode has no menubar and the font is bigger in the statusbar."));

    m_grownupAction = actionCollection()->add<KToggleAction>("mode_grownup");
    m_grownupAction->setText(i18n("Mode Grown-up"));
    m_grownupAction->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_G));
    m_grownupAction->setIcon(KIcon("klettres_grownup"));
    connect(m_grownupAction, SIGNAL(triggered(bool)), this, SLOT(slotModeGrownup()));
    m_grownupAction->setWhatsThis(i18n("The Grownup mode is the normal mode where you can see the menubar."));

    connect(m_levelAction, SIGNAL(triggered(int)), this, SLOT(slotChangeLevel(int)));
    connect(m_languageAction, SIGNAL(triggered(int)), this, SLOT(slotChangeLanguage(int)));
    connect(m_themeAction, SIGNAL(triggered(int)), this, SLOT(slotChangeTheme(int)));

    KStandardAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

    setupGUI();
}

void KLettres::setupStatusbar()
{
    KStatusBar *st=statusBar();
    m_langLabel = new QLabel(st);
    m_levLabel = new QLabel(st);
    st->addWidget(m_langLabel);
    st->insertFixedItem("", 1);//add a space
    st->addWidget(m_levLabel);   
    statusBar();
}

void KLettres::setupToolbars()
{
    // Toolbar for special characters
    specialCharToolbar = toolBar("specialCharToolbar");
    addToolBar ( Qt::BottomToolBarArea, specialCharToolbar);
}

void KLettres::optionsPreferences()
{
    if(KConfigDialog::showDialog("settings")) {
        return;
    }
    
    KConfigDialog *dialog = new KConfigDialog(this, "settings", Prefs::self());
    dialog->addPage(new fontsdlg(0), i18n("Font Settings"), "preferences-desktop-font");
    //fontsdlg is the page name, mFont is the widget name, Font Settings is the page display string
    //fonts is the icon
    Timer *m_timer = new Timer();
    dialog->addPage(m_timer, i18n("Timer"), "chronometer");
    connect(dialog, SIGNAL(settingsChanged(const QString &)), this, SLOT(slotUpdateSettings()));
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->setHelp(QString(), "klettres");
    dialog->show();
}

void KLettres::loadSettings()
{
    if (LangUtils::getLanguages().indexOf(Prefs::language()) <0)  {
        Prefs::setLanguage("en");
    }
    QString langString = LangUtils::getLanguagesNames(LangUtils::getLanguages())[LangUtils::getLanguages().indexOf(Prefs::language())];
    m_languageAction->setCurrentItem(LangUtils::getLanguages().indexOf(Prefs::language()));
    langString.remove('&');
    m_langLabel->setText(langString);
    loadLangToolBar();
    // load default level
    m_levLabel->setText(i18nc("@info:status the current level chosen", "(Level %1)", Prefs::level()));

    m_view->setTheme(KLThemeFactory::instance()->buildTheme(Prefs::theme()));

    if (Prefs::mode() == Prefs::EnumMode::grownup) {
        slotModeGrownup();
    } else {
        slotModeKid();
    }
    
    m_menubarAction->setChecked(Prefs::menuBarBool());
    slotMenubar();
}

void KLettres::slotDownloadNewStuff()
{
    QPointer<KNS3::DownloadDialog> dialog(new KNS3::DownloadDialog("klettres.knsrc", this)); 
    if ( dialog->exec() == QDialog::Accepted ) {
        // do nothing
    }

    delete dialog;

    //look for languages dirs installed
    QStringList languages = LangUtils::getLanguages();
    m_languageNames = LangUtils::getLanguagesNames(languages);

    //refresh Languages menu
    m_languageAction->setItems(m_languageNames);
    slotChangeLanguage(languages.indexOf(Prefs::language()));
    m_languageAction->setCurrentItem(languages.indexOf(Prefs::language()));
}

void KLettres::slotMenubar()
{
    menuBar()->setVisible(m_menubarAction->isChecked());
    Prefs::setMenuBarBool(m_menubarAction->isChecked());
    Prefs::self()->writeConfig();
}

void KLettres::slotUpdateSettings()
{
    m_view->m_timer = Prefs::kidTimer();
    m_view->m_timer = Prefs::grownTimer();
    //apply the font
    m_view->setFont(Prefs::font());
}

void KLettres::slotChangeLevel(int newLevel)
{
    Prefs::setLevel(newLevel+1);
    Prefs::self()->writeConfig();
    updateLevMenu(newLevel);
    //TODO is that necessary? Change level effectively by reloading sounds

    //this is duplicated in changeLanguage()
    soundFactory->change(Prefs::language());
    //update game effectively
    m_view->randomInt = 0;
    m_view->game();
}

void KLettres::updateLevMenu(int id)
{
    //m_levelCombo->setCurrentItem(id);
    m_levelAction->setCurrentItem(id);
    m_levLabel->setText(i18nc("@info:status the current level chosen", "(Level %1)", Prefs::level()));
}

void KLettres::slotChangeLanguage(int newIndex)
{
    // Write new language ISO in config
    QString newLanguage = LangUtils::getLanguages()[newIndex];
    Prefs::setLanguage(newLanguage);
    Prefs::self()->writeConfig();
    // Update the StatusBar
    QString langString = LangUtils::getLanguagesNames(LangUtils::getLanguages())[newIndex];
    langString.remove('&');
    m_langLabel->setText(langString);
    loadLangToolBar();
    // Change language effectively
    bool ok = loadLayout(soundFactory->m_layoutsDocument);
    
    if (ok) {
        soundFactory->change(Prefs::language());
    }
    
    m_view->randomInt = 0;
    m_view->game();
}

void KLettres::slotChangeTheme(int index)
{
    Prefs::setTheme(index);
    Prefs::self()->writeConfig();
    m_view->setTheme(KLThemeFactory::instance()->buildTheme(index));
}

void KLettres::slotModeGrownup()
{
    QPalette pal;
    pal.setColor( QPalette::Background, Qt::white);
    statusBar()->setPalette( pal );
    QFont f_lab( "Serif" , 10);  //font for statusBar
    m_levLabel->setFont(f_lab);
    m_langLabel->setFont(f_lab);
    m_menubarAction->setChecked(true);
    m_grownupAction->setChecked(true);
    m_kidAction->setChecked(false);
    m_grownupAction->setToolTip(i18n("Grown-up mode is currently active"));
    m_kidAction->setToolTip(i18n("Switch to Kid mode"));
    menuBar()->show();
    m_view->m_timer = Prefs::grownTimer();
    Prefs::setMode(Prefs::EnumMode::grownup);
    Prefs::self()->writeConfig();
}

void KLettres::slotModeKid()
{
    QPalette pal;
    pal.setColor( QPalette::Background, Qt::white);
    statusBar()->setPalette( pal );
    QFont f_lab( "Serif" , 12);  //font for statusBar
    f_lab.setBold(true);
    m_levLabel->setFont(f_lab);
    m_langLabel->setFont(f_lab);
    m_menubarAction->setChecked(false);
    m_kidAction->setChecked(true);
    m_kidAction->setToolTip(i18n("Kid mode is currently active"));
    m_grownupAction->setToolTip(i18n("Switch to Grown-up mode"));
    m_grownupAction->setChecked(false);
    menuBar()->hide();
    m_view->m_timer = Prefs::kidTimer();
    Prefs::setMode(Prefs::EnumMode::kid);
    Prefs::self()->writeConfig();
}

void KLettres::loadLangToolBar()
{
    QString lang = Prefs::language();

    specialCharToolbar->clear();

    if (LangUtils::hasSpecialChars(lang)) {//Dutch, English, French and Italian have no special characters
        allData.clear();
        QString myString=QString("klettres/%1.txt").arg(lang);
        QFile myFile;
        myFile.setFileName(KStandardDirs::locate("data",myString));
        
        if (!myFile.exists()) {
            QString mString=i18n("File $KDEDIR/share/apps/klettres/%1.txt not found;\n"
                                    "please check your installation.", lang);
            KMessageBox::sorry( this, mString,
                                    i18n("Error") );
            qApp->quit();
        }
        
        //we open the file and store info into the stream...
        QFile openFileStream(myFile.fileName());
        openFileStream.open(QIODevice::ReadOnly);
        QTextStream readFileStr(&openFileStream);
        readFileStr.setCodec("UTF-8");
        //allData contains all the words from the file
        allData = readFileStr.readAll().split('\n');
        openFileStream.close();
        
        for (int i=0; i<(int) allData.count(); ++i) {
            if (!allData[i].isEmpty()) {
                QAction *act = specialCharToolbar->addAction(allData.at(i));
                act->setIcon(charIcon(allData.at(i).at(0)));
                // used to carry the id
                act->setData(i);
                connect(act, SIGNAL(triggered(bool)), this, SLOT(slotPasteChar()));
            }
        }
        
        specialCharToolbar->show();
        update();
    } else {
      specialCharToolbar->hide();
    }
}

void KLettres::slotPasteChar()
{
    QAction *act = qobject_cast<QAction*>(sender());
    if (!act) {
        return;
    }

    bool ok = true;
    int id = act->data().toInt(&ok);
    
    if (!ok || id < 0 || id >= allData.count()) {
        return;
    }
    
    m_view->m_letterEdit->insert(allData.at(id));
}

QIcon KLettres::charIcon(const QChar & c)
{
    ///Create a name and path for the icon
    QString s = KStandardDirs::locateLocal("icon", "char" + QString::number(c.unicode()) + ".png");

    QRect r(4, 4, 120, 120);

    ///A font to draw the character with
    QFont font;
    font.setFamily( "Arial" );
    font.setPixelSize(120);
    font.setWeight(QFont::Normal);

    ///Create the pixmap
    QPixmap pm(128, 128);
    pm.fill(Qt::white);
    QPainter p(&pm);
    p.setFont(font);
    p.setPen(Qt::black);
    p.drawText(r, Qt::AlignCenter, (QString) c);

    ///Create transparency mask
    QBitmap bm(128, 128);
    bm.fill(Qt::color0);
    QPainter b(&bm);
    b.setFont(font);
    b.setPen(Qt::color1);
    b.drawText(r, Qt::AlignCenter, (QString) c);

    ///Mask the pixmap
    pm.setMask(bm);

    ///Save the icon to disk
    pm.save(s, "PNG");

    return QIcon(pm);
}

#include "klettres.moc"

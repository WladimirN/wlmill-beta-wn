#include <QApplication>
#include <QTextCodec>
#include <QLibrary>
#include <QThread>
#include <QDebug>
#include <QTranslator>

#include "wlmill.h"


int main(int argc, char *argv[])
{ 
	QApplication a(argc, argv);

    for(int i=1;i<argc;i++)
    {
    QString str=argv[i];

    if(str=="--noopengl"){ // no OpenGL hardware
       QApplication::setAttribute(Qt::AA_ForceRasterWidgets, false);
       }
    }

    QTranslator translator;
    translator.load(QApplication::applicationDirPath()+"//lang//WLMill_"+QLocale::system().name());

    a.installTranslator(&translator);

    QTextCodec *codec = QTextCodec::codecForName("UTF-8"); //Windows-1251

    QTextCodec::setCodecForLocale(codec);

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // DPI support
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps); //HiDPI pixmaps

   #ifndef QT_DEBUG
    WLLog::getInstance()->setEnableDebug(true);
   #endif

   #ifdef Q_OS_WIN
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
   #endif

    WLMill w;
    w.show();

	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

	return a.exec();	
}


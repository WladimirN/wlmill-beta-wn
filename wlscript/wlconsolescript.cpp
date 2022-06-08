#include "wlconsolescript.h"
#include "ui_wlconsolescript.h"
#include <QDebug>

WLConsoleScript::WLConsoleScript(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WLConsoleScript)
{
    ui->setupUi(this);

    connect(ui->pbClear,&QPushButton::clicked,ui->textEdit,&QTextEdit::clear);

    //ui->textEdit->setReadOnly(true);
   // ui->textEdit->setUndoRedoEnabled(false);
    ui->textEdit->setWordWrapMode(QTextOption::NoWrap); //тут отключаем перенос слов/символов
    ui->textEdit->document()->setMaximumBlockCount(DEF_MAXBLOCKCONSOLE); //вот тут ограничиваемся 10 - ю строками!!!
}

WLConsoleScript::~WLConsoleScript()
{
delete ui;
}

void WLConsoleScript::addText(QString txt)
{
ui->textEdit->append(txt);
}

#ifndef WLCONSOLESCRIPT_H
#define WLCONSOLESCRIPT_H

#include <QWidget>

namespace Ui {
class WLConsoleScript;
}


#define DEF_MAXBLOCKCONSOLE 200
class WLConsoleScript : public QWidget
{
    Q_OBJECT

public:
    explicit WLConsoleScript(QWidget *parent = nullptr);
    ~WLConsoleScript();

private:
    Ui::WLConsoleScript *ui;

public slots:
    void addText(QString txt);
};

#endif // WLCONSOLESCRIPT_H

#ifndef WLGTOOL_H
#define WLGTOOL_H

#include <QList>
#include <QTextCodec>
#include <QTextStream>

typedef QMap<QString,QVariant> WLGTool;

struct WLGTools
{
public:
    explicit WLGTools();

bool setTool(int index,QMap<QString,QVariant>);

WLGTool getTool(int key);
WLGTool getToolAt(int index);

WLGTool takeTool(int ikey);

    void setValue(int ikey,QString key,QVariant value);
QVariant getValue(int ikey,QString key,QVariant defvalue);
QVariant getValueAt(int index,QString key,QVariant defvalue);

int getKeyAt(int index) const;
int count() const;

bool readFromFile(QString filename,QString split=";");
bool writeToFile(QString filename ,QString split=";");

private:
QStringList m_keyList;
QMap <int,QMap<QString,QVariant>> m_tools;
};

#endif // WLGTOOL_H

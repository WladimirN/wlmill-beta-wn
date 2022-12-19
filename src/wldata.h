#ifndef WLGDATAL_H
#define WLGDATAL_H

#include <QList>
#include <QTextCodec>
#include <QTextStream>

typedef QMap<QString,QVariant> WLEData;

struct WLData
{
public:
    explicit WLData();

bool setData(int index,QMap<QString,QVariant>);

WLEData getData(int key);
WLEData getDataAt(int index);

WLEData takeData(int ikey);

    void setValue(int ikey,QString key,QVariant value);
QVariant getValue(int ikey,QString key,QVariant defvalue);
QVariant getValueAt(int index,QString key,QVariant defvalue);

int getKeyAt(int index) const;
int count() const;

bool readFromFile(QString filename,QString split=";");
bool writeToFile(QString filename ,QString split=";");

bool write(QString split=";");
bool read(QString split=";");

void setHeaders(QStringList headers);
void clear();

QString getFileName() {return m_fileName;}
private:
QStringList m_headers;

QString m_fileName;

QMap <int,WLEData> m_data;
};

#endif // WLDATA_H

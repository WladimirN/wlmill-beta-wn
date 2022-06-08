#ifndef WLGPROGRAMWIDGET_H
#define WLGPROGRAMWIDGET_H

#include <QWidget>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>
#include <QStyledItemDelegate>
#include <QAbstractListModel>
#include <QListWidgetItem>

#include "ui_wlgprogramwidget.h"

#include "wlgprogram.h"
#include "wlgcodesh.h"

class WLGProgramListModel: public QAbstractTableModel
{
Q_OBJECT

public:
explicit  WLGProgramListModel (WLGProgram *_GProgram,QObject *parent=nullptr): QAbstractTableModel(parent)
   {
   m_GProgram=_GProgram;

   connect(m_GProgram,QOverload<>::of(&WLGProgram::changedProgram),this,[=](){
   beginResetModel();

   endResetModel();
   });

   }

private:
   WLGProgram *m_GProgram=nullptr;

    // QAbstractItemModel interface
public:
  int rowCount(const QModelIndex &parent) const {
      Q_UNUSED(parent);
      return m_GProgram->getElementCount();
      }

    QVariant data(const QModelIndex &index, int role) const {
      if (!index.isValid()) return QVariant();

      if (index.row() >= m_GProgram->getElementCount()) return QVariant();

       if (role == Qt::DisplayRole) {
            switch (role)
            {
            case Qt::DisplayRole: //qDebug()<<"Display Role"<<index.row();
                                   //return QString::number(index.row())+": "+m_GProgram->getTextElement(index.row()).simplified();
                                   return m_GProgram->getTextElement(index.row()).simplified();
            }
        }

      return QVariant();
      }


    // QAbstractItemModel interface
public:
    int columnCount(const QModelIndex &parent) const {return 1;}

    // QAbstractItemModel interface
public:
    QVariant headerData(int section, Qt::Orientation orientation, int role) const {

        if (role != Qt::DisplayRole)
                  return QVariant();

        if(orientation==Qt::Vertical)  return section;

        return  QVariant();
        }
};


class WLGProgramWidget : public QWidget
{
	Q_OBJECT

public:
    WLGProgramWidget(WLGProgram *_Program,QWidget *parent = nullptr);
    ~WLGProgramWidget();

private:
    void showListProgram(int iCenter,bool center=false);

    bool isListProgram() {return ui.stackedWidget->currentIndex()==0;}

private:
    Ui::WLGProgramWidget ui;
	
    WLGProgramListModel *m_ListModel=nullptr;

    bool m_changedProgram;

    WLGProgram *m_GProgram;

    WLGCodeSH *m_GCodeSH;


    QString m_lastNameProgram;

    int iEditElement=0;

    bool m_trackElementF=true;

    QListWidgetItem *itemSelect=nullptr;

signals:

	void changed(bool);
    void changedEditElement(int index);

    void pressOpenFile();

public slots:

	void setEditDisabled(bool);
    void setEditElement(int iElement);
	void loadTextProgram();

private slots:

    void showContextMenuListProgram(const QPoint &pos);
    void copyTextListProgram();

    void updateTrack();
    void setTrack(bool en) {m_trackElementF=en;}

    void saveTextProgram();

	void onUpdate();
	void onAccept();
	void onBackup();
	void onReload();
    void onOpenFile() {emit pressOpenFile();}

	void onChangedTextProgram();
	void onChangedPositionTextProgram();
    void onChangedPositionListProgram(const QModelIndex & current, const QModelIndex & previous);
};

#endif // WLGPROGRAMWIDGET_H

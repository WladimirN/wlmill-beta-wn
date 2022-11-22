#ifndef WLOPENGL_H
#define WLOPENGL_H

#include <QMatrix4x4>
#include <QVector4D>
#include <QTimer>
#include <QQuaternion>
#include <QDebug>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include "wlframe.h"
#include "wlcalc.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif


class WLOpenGL : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

private:
    QTimer *m_timerMovie;

    QQuaternion m_startQuant;
    QQuaternion   m_endQuant;

    QVector4D  m_startOffset;
    QVector4D    m_endOffset;

    float m_startZoom;
    float m_endZoom;
	
	float t;
	
	void startMovie();

    QColor clearColor;

private slots:
	void updateView();

public:
     WLOpenGL();
    ~WLOpenGL();

    float getZoom() {return m_zoom;}
	void  resetRotView();
	void  resetView();


    QColor getClearColor() {return clearColor;}
    void   setGLClearColor() { glClearColor(clearColor.redF()
                                           ,clearColor.greenF()
                                           ,clearColor.blueF()
                                           ,1);}

public:
    virtual QMatrix4x4 getShowMatrix() {return showMatrix;}

protected:
    void initializeGL();
	void resizeGL(int w, int h);

//public:	
	

	void movView(float Xd,float Yd,float Zd=0);
    void rotView(float Xa,float Ya,float Za=0) {showMatrix=getRotMatrix(Xa,Ya,Za)*showMatrix;}

    void zoomView(QPoint MousePos,int delta);

	void setPointRot(QVector4D pR);

    void setRotView();

	
    virtual void initShaders() {}
    virtual void initGLBuffers(){}

private:

protected:
   QMatrix4x4 showMatrix;
   QMatrix4x4 projection;
   QVector4D  showOffset;

   float      m_zoom=1;
   GLint      m_vport[4];
   QMatrix4x4 m_startShowMatrix;

public slots: 

    void setClearColor(QColor _color) {clearColor=_color;}

virtual  void setViewUp(void)     {setView(WLFrame(0,0,0,0,0,0).toM(),showOffset);}
virtual  void setViewDown(void)   {setView(WLFrame(0,0,0,0,0,180).toM(),showOffset);}
virtual  void setViewLeft(void)   {setView(WLFrame(0,0,0,180,0,90).toM(),showOffset);}
virtual  void setViewRight(void)  {setView(WLFrame(0,0,0,0,0,-90).toM(),showOffset);}
virtual  void setViewFront(void)  {setView(WLFrame(0,0,0,-90,-90,0).toM(),showOffset);}
virtual  void setViewRear(void)   {setView(WLFrame(0,0,0,90,90,0).toM(),showOffset);}

    void setView(QMatrix4x4 Fr,QVector4D offset,float zoom=-1);


virtual void setViewFrame(void) {
                                 showOffset.setX(0);
                                 showOffset.setY(0);
                                 update();
                                 }


};

#endif // WLOPENGL_H

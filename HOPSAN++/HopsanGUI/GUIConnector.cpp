//$Id$

#include "GUIConnector.h"
#include <QDebug>
#include "GUIPort.h"
#include <vector>
#include <QStyleOptionGraphicsItem>
#include <math.h>

//! Constructor.
//! @param x1 is the x-coordinate of the start position.
//! @param y1 is the y-coordinate of the start position.
//! @param x2 is the x-coordinate of the end position.
//! @param y2 is the y-coordinate of the end position.
//! @param passivePen defines the default width and color of the line.
//! @param activePen defines the width and color of the line when it is selected.
//! @param hoverPen defines the width and color of the line when hovered by the mouse cursor.
//! @param *parentView is a pointer to the GraphicsView the connector belongs to.
//! @param parent is the parent of the port.
GUIConnector::GUIConnector(qreal x1, qreal y1, qreal x2, qreal y2, QPen passivePen, QPen activePen, QPen hoverPen, GraphicsView *parentView, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    this->mpParentView = parentView;

    setFlags(QGraphicsItem::ItemIsFocusable);

    this->setPos(x1, y1);
    this->startPos.setX(x1);
    this->startPos.setY(y1);
    this->endPos.setX(x2);
    this->endPos.setY(y2);

    this->mPassivePen = passivePen;
    this->mActivePen = activePen;
    this->mHoverPen = hoverPen;

    this->mEndPortConnected = false;
    this->mFirstFixedLineAdded = false;

        //Add first line
    mpTempLine = new GUIConnectorLine(this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(),
                                      this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(),
                                      mPassivePen, mActivePen, mHoverPen, 0, this);
    mLines.push_back(mpTempLine);

    this->setActive();

    connect(mLines[mLines.size()-1],SIGNAL(lineSelected(bool)),this,SLOT(doSelect(bool)));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
    connect(this,SIGNAL(endPortConnected()),mLines[mLines.size()-1],SLOT(setConnected()));
}


//! Destructor.
GUIConnector::~GUIConnector()
{
    mLines.clear();
    //! @todo more cleanup
}


//! Sets the pointer to the start port of a connector.
//! @param *port is the pointer to the new start port.
//! @see setEndPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void GUIConnector::setStartPort(GUIPort *port)
{
    this->mpStartPort = port;
    connect(this->mpStartPort->getComponent(),SIGNAL(componentMoved()),this,SLOT(updatePos()));
    connect(this->mpStartPort->getComponent(),SIGNAL(componentDeleted()),this,SLOT(deleteMe()));
}


//! Sets the pointer to the end port of a connector, and executes the final tasks before creation of the connetor is complete. Then flags that the end port is connected.
//! @param *port is the pointer to the new end port.
//! @see setStartPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void GUIConnector::setEndPort(GUIPort *port)
{
    this->mEndPortConnected = true;
    this->mpEndPort = port;
    connect(this->mpEndPort->getComponent(),SIGNAL(componentMoved()),this,SLOT(updatePos()));
    connect(this->mpEndPort->getComponent(),SIGNAL(componentDeleted()),this,SLOT(deleteMe()));

        //Make all lines selectable and all lines except first and last movable
    for(std::size_t i=1; i!=mLines.size()-1; ++i)
        mLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
    for(std::size_t i=0; i!=mLines.size(); ++i)
        mLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);

        //Add arrow to the connector if it is of signal type
    if(port->getPortType() == GUIPort::READ)
        this->getLastLine()->addEndArrow();
    else if(port->getPortType() == GUIPort::WRITE)
        this->mLines[0]->addStartArrow();

    emit endPortConnected();
    this->setPassive();
}


//! Returns the pointer to the start port of a connector.
//! @see setStartPort(GUIPort *port)
//! @see setEndPort(GUIPort *port)
//! @see getEndPort()
GUIPort *GUIConnector::getStartPort()
{
    return this->mpStartPort;
}


//! Returns the pointer to the end port of a connector.
//! @see setStartPort(GUIPort *port)
//! @see setEndPort(GUIPort *port)
//! @see getStartPort()
GUIPort *GUIConnector::getEndPort()
{
    return this->mpEndPort;
}


//! Updates an already finished connector with start and end positions from its ports by using the updateConnector function. Used to make connectors follow the components as they move.
//! @see setStartPort(GUIPort *port)
//! @see setEndPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void GUIConnector::updatePos()
{
    QPointF startPort = this->getStartPort()->mapToScene(this->getStartPort()->boundingRect().center());
    QPointF endPort = this->getEndPort()->mapToScene(this->getEndPort()->boundingRect().center());
    this->updateConnector(startPort, endPort);
}


//! Slot that activates or deactivates the connector if one of its lines is selected or deselected.
//! @param lineSelected tells whether the signal was induced by selection or deselection of a line.
//! @see setActive()
//! @see setPassive()
void GUIConnector::doSelect(bool lineSelected)
{
    if(this->mEndPortConnected)     //Non-finished lines shall not be selectable
    {
        if(lineSelected)
        {
            this->setActive();
            qDebug() << "Activating line";
        }
        else
        {
            this->setPassive();
            qDebug() << "Passivating line";
        }
    }
}


//! Activates a connector, activates each line and connects delete function with delete key.
//! @see setPassive()
void GUIConnector::setActive()
{
    connect(this->mpParentView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
    if(this->mEndPortConnected)
    {
        mIsActive = true;
        for (std::size_t i=0; i!=mLines.size(); ++i )
        {
            mLines[i]->setActive();
        }
        qDebug() << "setActive()";
    }
}


//! Deactivates a connector, deactivates each line and disconnects delete function with delete key.
//! @see setActive()
void GUIConnector::setPassive()
{
    disconnect(this->mpParentView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
    if(this->mEndPortConnected)
    {
        mIsActive = false;
        for (std::size_t i=0; i!=mLines.size(); ++i )
        {
            mLines[i]->setPassive();
        }
    }
}

//! Changes connector style back to normal if it is not active. Used when mouse stops hovering a line.
//! @see setHovered()
//! @see setPassive()
void GUIConnector::setUnHovered()
{
    if(this->mEndPortConnected && !this->mIsActive)
    {
        for (std::size_t i=0; i!=mLines.size(); ++i )
        {
            mLines[i]->setPassive();
        }
    }
}


//! Changes connector style to hovered if it is not active. Used when mouse starts hovering a line.
//! @see setUnHovered()
void GUIConnector::setHovered()
{
    if(this->mEndPortConnected && !this->mIsActive)
    {
        for (std::size_t i=0; i!=mLines.size(); ++i )
        {
            mLines[i]->setHovered();
        }
    }
}


//! Updates the first two and last two lines of a connector with respect to start position, end position and the geometry of the lines.
//! @param startPos is the new start position of the connector.
//! @param endPos is the new end pospition of the connector.
//! @see updateLine(int lineNumber)
void GUIConnector::updateConnector(QPointF startPos, QPointF endPos)
{

        //Update first two lines with respect to start position and angle of start port
    if (this->getStartPort()->getPortDirection() == GUIPort::HORIZONTAL)
    {
        getLine(0)->setLine(getLine(0)->mapFromScene(startPos).x(),
                            getLine(0)->mapFromScene(startPos).y(),
                            getLine(0)->mapFromParent(getLine(1)->mapToParent(getLine(1)->line().p2())).x(),
                            getLine(0)->mapFromScene(startPos).y());
        getLine(1)->setLine(getLine(1)->mapFromParent(getLine(0)->mapToParent(getLine(0)->line().p2())).x(),
                            getLine(1)->mapFromParent(getLine(0)->mapToParent(getLine(0)->line().p2())).y(),
                            getLine(1)->line().x2(),
                            getLine(1)->line().y2());
    }
    else if (this->getStartPort()->getPortDirection() == GUIPort::VERTICAL)
    {
        getLine(0)->setLine(getLine(0)->mapFromScene(startPos).x(),
                            getLine(0)->mapFromScene(startPos).y(),
                            getLine(0)->mapFromScene(startPos).x(),
                            getLine(0)->mapFromParent(getLine(1)->mapToParent(getLine(1)->line().p2())).y());
        getLine(1)->setLine(getLine(1)->mapFromParent(getLine(0)->mapToParent(getLine(0)->line().p2())).x(),
                            getLine(1)->mapFromParent(getLine(0)->mapToParent(getLine(0)->line().p2())).y(),
                            getLine(1)->line().x2(),
                            getLine(1)->line().y2());
    }

        //If connector only has two lines
    if (getNumberOfLines()<3 and getStartPort()->getPortDirection() == GUIPort::HORIZONTAL and getLastLine()->getGeometry()!=GUIConnectorLine::DIAGONAL)
    {
        getSecondLastLine()->setLine(getSecondLastLine()->mapFromScene(startPos).x(),
                               getSecondLastLine()->mapFromScene(startPos).y(),
                               getSecondLastLine()->mapFromScene(endPos).x(),
                               getSecondLastLine()->mapFromScene(startPos).y());
        getSecondLastLine()->setGeometry(GUIConnectorLine::HORIZONTAL);
        getLastLine()->setGeometry(GUIConnectorLine::VERTICAL);
    }
    else if (getNumberOfLines()<3 and getStartPort()->getPortDirection() == GUIPort::VERTICAL and getLastLine()->getGeometry()!=GUIConnectorLine::DIAGONAL)
    {
        getSecondLastLine()->setLine(getSecondLastLine()->mapFromScene(startPos).x(),
                               getSecondLastLine()->mapFromScene(startPos).y(),
                               getSecondLastLine()->mapFromScene(startPos).x(),
                               getSecondLastLine()->mapFromScene(endPos).y());
        getSecondLastLine()->setGeometry(GUIConnectorLine::VERTICAL);
        getLastLine()->setGeometry(GUIConnectorLine::HORIZONTAL);
    }

        //If connector has more than two lines
    else if (getSecondLastLine()->getGeometry()== GUIConnectorLine::VERTICAL and getLastLine()->getGeometry()!=GUIConnectorLine::DIAGONAL)
    {
        getSecondLastLine()->setLine(getSecondLastLine()->mapFromParent(getThirdLastLine()->mapToParent(getThirdLastLine()->line().p2())).x(),
                               getSecondLastLine()->mapFromParent(getThirdLastLine()->mapToParent(getThirdLastLine()->line().p2())).y(),
                               getSecondLastLine()->mapFromParent(getThirdLastLine()->mapToParent(getThirdLastLine()->line().p2())).x(),
                               getSecondLastLine()->mapFromScene(endPos).y());
        getLastLine()->setGeometry(GUIConnectorLine::HORIZONTAL);
    }
    else if (getSecondLastLine()->getGeometry()==GUIConnectorLine::HORIZONTAL and getLastLine()->getGeometry()!=GUIConnectorLine::DIAGONAL)
    {
        getSecondLastLine()->setLine(getSecondLastLine()->mapFromParent(getThirdLastLine()->mapToParent(getThirdLastLine()->line().p2())).x(),
                               getSecondLastLine()->mapFromParent(getThirdLastLine()->mapToParent(getThirdLastLine()->line().p2())).y(),
                               getSecondLastLine()->mapFromScene(endPos).x(),
                               getSecondLastLine()->mapFromParent(getThirdLastLine()->mapToParent(getThirdLastLine()->line().p2())).y());
        getLastLine()->setGeometry(GUIConnectorLine::VERTICAL);
    }

        //Update second last line with respect to end position and angle of end port (but only if end port is connected)
    if (getNumberOfLines()>2 and mEndPortConnected and getEndPort()->getPortDirection() == GUIPort::VERTICAL)
    {
        getSecondLastLine()->setLine(getSecondLastLine()->mapFromParent(getThirdLastLine()->mapToParent(getThirdLastLine()->line().p2())).x(),
                               getSecondLastLine()->mapFromParent(getThirdLastLine()->mapToParent(getThirdLastLine()->line().p2())).y(),
                               getSecondLastLine()->mapFromScene(endPos).x(),
                               getSecondLastLine()->mapFromParent(getThirdLastLine()->mapToParent(getThirdLastLine()->line().p2())).y());
        getLastLine()->setGeometry(GUIConnectorLine::VERTICAL);
        getSecondLastLine()->setGeometry(GUIConnectorLine::HORIZONTAL);
    }
    else if (getNumberOfLines()>2 and mEndPortConnected and getEndPort()->getPortDirection() == GUIPort::HORIZONTAL)
    {
        getSecondLastLine()->setLine(getSecondLastLine()->mapFromParent(getThirdLastLine()->mapToParent(getThirdLastLine()->line().p2())).x(),
                               getSecondLastLine()->mapFromParent(getThirdLastLine()->mapToParent(getThirdLastLine()->line().p2())).y(),
                               getSecondLastLine()->mapFromParent(getThirdLastLine()->mapToParent(getThirdLastLine()->line().p2())).x(),
                               getSecondLastLine()->mapFromScene(endPos).y());
        getLastLine()->setGeometry(GUIConnectorLine::HORIZONTAL);
        getSecondLastLine()->setGeometry(GUIConnectorLine::VERTICAL);
    }

        //Update last line with respect to end position
    getLastLine()->setLine(getLastLine()->mapFromParent(getSecondLastLine()->mapToParent(getSecondLastLine()->line().p2())).x(),
                           getLastLine()->mapFromParent(getSecondLastLine()->mapToParent(getSecondLastLine()->line().p2())).y(),
                           getLastLine()->mapFromScene(endPos).x(),
                           getLastLine()->mapFromScene(endPos).y());
}


//! Adds a new line with no specified end position. Used when creating lines manually.
//! @see addFixedLine(int length, int height, GUIConnectorLine::geometryType geometry)
void GUIConnector::addFreeLine()
{
    mpTempLine = new GUIConnectorLine(mLines[mLines.size()-1]->line().p2().x(), mLines[mLines.size()-1]->line().p2().y(),
                                      mLines[mLines.size()-1]->line().p2().x(), mLines[mLines.size()-1]->line().p2().y(),
                                      mPassivePen, mActivePen, mHoverPen, mLines.size(), this);
    mpTempLine->setActive();
    mLines.push_back(mpTempLine);
    mLines[mLines.size()-2]->setPassive();
    connect(mLines[mLines.size()-1],SIGNAL(lineSelected(bool)),this,SLOT(doSelect(bool)));
    connect(mLines[mLines.size()-1],SIGNAL(lineMoved(int)),this, SLOT(updateLine(int)));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
    connect(this,SIGNAL(endPortConnected()),mLines[mLines.size()-1],SLOT(setConnected()));
}


//! Adds a line with specified geometry and length/height at the end of the connector. Used when loading connectors from a model file.
//! @param length is the size of the line in the y-direction. Not used for vertical lines.
//! @param height is the size of the line in the x-direction. Not used for horizontal lines.
//! @param geometry defines whether the line is horizontal, vertical or diagonal.
//! @see addFreeLine()
void GUIConnector::addFixedLine(int length, int height, GUIConnectorLine::geometryType geometry)
{
    //If only two lines exist and if this check has not been done before, we are at the beginning.
    //Therefore we must remove the automatically created lines, and used the connector start
    //position as the line start position.
    if(this->mLines.size() == 2 && !mFirstFixedLineAdded)
    {
        qDebug() << "First line";
        this->scene()->removeItem(mLines.back());
        mLines.pop_back();
        mFirstFixedLineAdded = true;

        if(geometry == GUIConnectorLine::HORIZONTAL)
        {
            qDebug() << "HORIZONTAL from" <<    this->startPos.x() <<        this->startPos.y();
            qDebug() << "HORIZONTAL to" <<      this->startPos.x()+length << this->startPos.y();
            QPointF tempStartPos = mapFromScene(this->startPos);
            QPointF tempEndPos = this->startPos;
            tempEndPos.setX(tempEndPos.x()+length);
            tempEndPos = mapFromScene(tempEndPos);
            this->mLines.front()->setLine(      tempStartPos.x(),       tempStartPos.y(),
                                                tempEndPos.x(),         tempEndPos.y());
            this->mLines.front()->startPos = tempStartPos;
            this->mLines.front()->endPos = tempEndPos;
        }
        else if(geometry == GUIConnectorLine::VERTICAL)
        {
            qDebug() << "VERTICAL from" <<      this->startPos.x() <<       this->startPos.y();
            qDebug() << "VERTICAL to" <<        this->startPos.x() <<       this->startPos.y()+height;
            QPointF tempStartPos = mapFromScene(this->startPos);
            QPointF tempEndPos = this->startPos;
            tempEndPos.setY(tempEndPos.y()+height);
            tempEndPos = mapFromScene(tempEndPos);
            this->mLines.front()->setLine(      tempStartPos.x(),       tempStartPos.y(),
                                                tempEndPos.x(),         tempEndPos.y());
            this->mLines.front()->startPos = tempStartPos;
            this->mLines.front()->endPos = tempEndPos;
        }
        else if(geometry == GUIConnectorLine::DIAGONAL)
        {
                qDebug() << "DIAGONAL from" <<  this->startPos.x() <<           this->startPos.y();
                qDebug() << "DIAGONAL to" <<    this->startPos.x()+length <<    this->startPos.y()+height;
                QPointF tempStartPos = mapFromScene(this->startPos);
                QPointF tempEndPos = this->startPos;
                tempEndPos.setX(tempEndPos.x()+height);
                tempEndPos.setY(tempEndPos.y()+height);
                tempEndPos = mapFromScene(tempEndPos);
                this->mLines.front()->setLine(      tempStartPos.x(),       tempStartPos.y(),
                                                    tempEndPos.x(),         tempEndPos.y());
                this->mLines.front()->startPos = tempStartPos;
                this->mLines.front()->endPos = tempEndPos;
        }
        this->mLines.front()->setGeometry(geometry);
        this->mLines.front()->setActive();
    }
    else
    //We are not at the beginning. Keep the previous lines and add a new one, with the end position of the
    //previous line as start position for the new one.
    {
        if(geometry == GUIConnectorLine::HORIZONTAL)
        {
            qDebug() << "HORIZONTAL from" << mLines[mLines.size()-1]->line().p2().x() << mLines[mLines.size()-1]->line().p2().y();
            qDebug() << "HORIZONTAL to" << mLines[mLines.size()-1]->line().p2().x()+length << mLines[mLines.size()-1]->line().p2().y();
            QPointF tempEndPos = mapToScene(mLines[mLines.size()-1]->line().p2());
            tempEndPos.setX(tempEndPos.x()+length);
            tempEndPos = mapFromScene(tempEndPos);
            mpTempLine = new GUIConnectorLine(mLines[mLines.size()-1]->line().p2().x(), mLines[mLines.size()-1]->line().p2().y(),
                                              tempEndPos.x(), tempEndPos.y(),
                                              mPassivePen, mActivePen, mHoverPen, mLines.size(), this);
        }
        else if(geometry == GUIConnectorLine::VERTICAL)
        {
            qDebug() << "VERTICAL from" << mLines[mLines.size()-1]->line().p2().x() << mLines[mLines.size()-1]->line().p2().y();
            qDebug() << "VERTICAL to" << mLines[mLines.size()-1]->line().p2().x() << mLines[mLines.size()-1]->line().p2().y()+height;
            QPointF tempEndPos = mapToScene(mLines[mLines.size()-1]->line().p2());
            tempEndPos.setY(tempEndPos.y()+height);
            tempEndPos = mapFromScene(tempEndPos);
            mpTempLine = new GUIConnectorLine(mLines[mLines.size()-1]->line().p2().x(), mLines[mLines.size()-1]->line().p2().y(),
                                              tempEndPos.x(), tempEndPos.y(),
                                              mPassivePen, mActivePen, mHoverPen, mLines.size(), this);
        }
        else if(geometry == GUIConnectorLine::DIAGONAL)
        {
            qDebug() << "DIAGONAL from" << mLines[mLines.size()-1]->line().p2().x() << mLines[mLines.size()-1]->line().p2().y();
            qDebug() << "DIAGONAL to" << mLines[mLines.size()-1]->line().p2().x()+length << mLines[mLines.size()-1]->line().p2().y()+height;
            QPointF tempEndPos = mapToScene(mLines[mLines.size()-1]->line().p2());
            tempEndPos.setX(tempEndPos.x()+length);
            tempEndPos.setY(tempEndPos.y()+height);
            tempEndPos = mapFromScene(tempEndPos);
            mpTempLine = new GUIConnectorLine(mLines[mLines.size()-1]->line().p2().x(), mLines[mLines.size()-1]->line().p2().y(),
                                              tempEndPos.x(), tempEndPos.y(),
                                              mPassivePen, mActivePen, mHoverPen, mLines.size(), this);
        }
        mpTempLine->setGeometry(geometry);
        mpTempLine->setActive();
        mLines.push_back(mpTempLine);
        mLines[mLines.size()-2]->setPassive();
        connect(mLines[mLines.size()-1],SIGNAL(lineSelected(bool)),this,SLOT(doSelect(bool)));
        connect(mLines[mLines.size()-1],SIGNAL(lineMoved(int)),this, SLOT(updateLine(int)));
        connect(mLines[mLines.size()-1],SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
        connect(mLines[mLines.size()-1],SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
        connect(this,SIGNAL(endPortConnected()),mLines[mLines.size()-1],SLOT(setConnected()));
    }
}


//! Removes last line from a connector and updates it, or removes the entire connector if only two lines remains. Used when right clicking during line creation.
//! @param cursorPos is the current cursor position. The end of the connector shall point here.
void GUIConnector::removeLine(QPointF cursorPos)
{
    if (getNumberOfLines() > 2)
    {
        qDebug() << "Removing line!";
        this->scene()->removeItem(mLines.back());
        mLines.pop_back();
        this->updateConnector(this->mapToScene(mLines[0]->line().p1()), cursorPos);
    }
    else
    {
        this->scene()->removeItem(this);
        delete(this);
    }
}

//! Returns the number of lines in a connector.
int GUIConnector::getNumberOfLines()
{
    return mLines.size();
}

//! Asks my parent to delete myself
void GUIConnector::deleteMe()
{
    mpParentView->removeConnector(this);
}

//! Updates the lines before and after the specified lines. Used to make lines follow each other when they are moved.
//! @param lineNumber is the number of the line that has moved.
//! @see updatePos()
//! @see updateConnector(QPointF startPos, QPointF endPos)
void GUIConnector::updateLine(int lineNumber)
{
    qDebug() << "Updating line: x = " << getLine(lineNumber)->line().x2();
    if (this->mEndPortConnected && lineNumber != 0 && lineNumber != mLines.size())
    {
        if(getLine(lineNumber)->getGeometry()==GUIConnectorLine::HORIZONTAL)
        {
            getLine(lineNumber-1)->setLine(getLine(lineNumber-1)->line().x1(),
                                           getLine(lineNumber-1)->line().y1(),
                                           getLine(lineNumber-1)->line().x2(),
                                           getLine(lineNumber-1)->mapFromParent(getLine(lineNumber)->mapToParent(getLine(lineNumber)->line().p1())).y());
            getLine(lineNumber+1)->setLine(getLine(lineNumber+1)->line().x1(),
                                           getLine(lineNumber+1)->mapFromParent(getLine(lineNumber)->mapToParent(getLine(lineNumber)->line().p2())).y(),
                                           getLine(lineNumber+1)->line().x2(),
                                           getLine(lineNumber+1)->line().y2());
            getLine(lineNumber)->setLine(getLine(lineNumber)->mapFromParent(getLine(lineNumber-1)->mapToParent(getLine(lineNumber-1)->line().p2())).x(),
                                         getLine(lineNumber)->line().y1(),
                                         getLine(lineNumber)->mapFromParent(getLine(lineNumber+1)->mapToParent(getLine(lineNumber+1)->line().p1())).x(),
                                         getLine(lineNumber)->line().y2());
        }
        else if(getLine(lineNumber)->getGeometry()==GUIConnectorLine::VERTICAL)
        {
            getLine(lineNumber-1)->setLine(getLine(lineNumber-1)->line().x1(),
                                           getLine(lineNumber-1)->line().y1(),
                                           getLine(lineNumber-1)->mapFromParent(getLine(lineNumber)->mapToParent(getLine(lineNumber)->line().p1())).x(),
                                           getLine(lineNumber-1)->line().y2());
            getLine(lineNumber+1)->setLine(getLine(lineNumber+1)->mapFromParent(getLine(lineNumber)->mapToParent(getLine(lineNumber)->line().p2())).x(),
                                           getLine(lineNumber+1)->line().y1(),
                                           getLine(lineNumber+1)->line().x2(),
                                           getLine(lineNumber+1)->line().y2());
            getLine(lineNumber)->setLine(getLine(lineNumber)->line().x1(),
                                         getLine(lineNumber)->mapFromParent(getLine(lineNumber-1)->mapToParent(getLine(lineNumber-1)->line().p2())).y(),
                                         getLine(lineNumber)->line().x2(),
                                         getLine(lineNumber)->mapFromParent(getLine(lineNumber+1)->mapToParent(getLine(lineNumber+1)->line().p1())).y());
        }
    }
    this->updatePos();
}


//! Returns the third last line of the connector.
//! @see getSecondLastLine()
//! @see getLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getThirdLastLine()
{
    return mLines[mLines.size()-3];
}


//! Returns the second last line of the connector.
//! @see getThirdLastLine()
//! @see getLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getSecondLastLine()
{
    return mLines[mLines.size()-2];
}


//! Returns the last line of the connector.
//! @see getThirdLastLine()
//! @see getSecondLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getLastLine()
{
    return mLines[mLines.size()-1];
}


//! Returns the line with specified number.
//! @param line is the number of the wanted line.
//! @see getThirdLastLine()
//! @see getSecondLastLine()
//! @see getLastLine()
GUIConnectorLine *GUIConnector::getLine(int line)
{
    return mLines[line];
}





//! Constructor.
//! @param x1 is the x-coordinate of the start position of the line.
//! @param y1 is the y-coordinate of the start position of the line.
//! @param x2 is the x-coordinate of the end position of the line.
//! @param y2 is the y-coordinate of the end position of the line.
//! @param primaryPen defines the default color and width of the line.
//! @param activePen defines the color and width of the line when it is selected.
//! @param hoverPen defines the color and width of the line when it is hovered by the mouse cursor.
//! @param lineNumber is the number of the line in the connector.
//! @param *parent is a pointer to the parent object.
GUIConnectorLine::GUIConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, QPen primaryPen, QPen activePen, QPen hoverPen, int lineNumber, QGraphicsItem *parent)
        : QGraphicsLineItem(x1,y1,x2,y2,parent)
{
    setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    this->mPrimaryPen = primaryPen;
    this->mActivePen = activePen;
    this->mHoverPen = hoverPen;
    this->mLineNumber = lineNumber;
    this->setAcceptHoverEvents(true);
    this->mParentConnectorEndPortConnected = false;
    this->startPos = QPointF(x1,y1);
    this->endPos = QPointF(x2,y2);
    this->setGeometry(GUIConnectorLine::HORIZONTAL);
    this->mHasStartArrow = false;
    this->mHasEndArrow = false;
    this->mArrowSize = 10.0;
    this->mArrowAngle = 0.6;
}


//! Destructor
GUIConnectorLine::~GUIConnectorLine()
{
}


//! Reimplementation of paint function. Removes the ugly dotted selection box.
void GUIConnectorLine::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    QStyleOptionGraphicsItem *_o = const_cast<QStyleOptionGraphicsItem*>(o);
    _o->state &= ~QStyle::State_Selected;
    QGraphicsLineItem::paint(p,_o,w);
}


//! Changes the style of the line to active.
//! @see setPassive()
//! @see setHovered()
void GUIConnectorLine::setActive()
{
        this->setPen(mActivePen);
}


//! Changes the style of the line to default.
//! @see setActive()
//! @see setHovered()
void GUIConnectorLine::setPassive()
{
        this->setPen(mPrimaryPen);
}


//! Changes the style of the line to hovered.
//! @see setActive()
//! @see setPassive()
void GUIConnectorLine::setHovered()
{
        this->setPen(mHoverPen);
}


//! Defines what shall happen if the line is clicked.
void GUIConnectorLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit lineClicked();
}


//! Devines what shall happen if the mouse cursor enters the line. Change cursor if the line is movable.
//! @see hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
void GUIConnectorLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(this->flags().testFlag((QGraphicsItem::ItemIsMovable)))
    {
        if(this->mParentConnectorEndPortConnected && this->getGeometry()==GUIConnectorLine::VERTICAL)
        {
            this->setCursor(Qt::SizeHorCursor);
        }
        else if(this->mParentConnectorEndPortConnected && this->getGeometry()==GUIConnectorLine::HORIZONTAL)
        {
            this->setCursor(Qt::SizeVerCursor);
        }
    }
    emit lineHoverEnter();
}


//! Defines what shall happen when mouse cursor leaves the line.
//! @see hoverEnterEvent(QGraphicsSceneHoverEvent *event)
void GUIConnectorLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit lineHoverLeave();
}


//! Returns the type of geometry (vertical, horizontal or diagonal) of the line.
//! @see setGeometry(geometryType newgeometry)
GUIConnectorLine::geometryType GUIConnectorLine::getGeometry()
{
    return mGeometry;
}


//! Sets the type of geometry (vertical, horizontal or diagonal) of the line.
//! @see getGeometry()
void GUIConnectorLine::setGeometry(geometryType newgeometry)
{
    mGeometry=newgeometry;
}


//! Returns the number of the line in the connector.
int GUIConnectorLine::getLineNumber()
{
    return mLineNumber;
}


//! Defines what shall happen if the line is selected or moved.
QVariant GUIConnectorLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        qDebug() << "Line selection status = " << this->isSelected();
        emit lineSelected(this->isSelected());
    }
    else if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        qDebug() << "Line has moved\n";
        emit lineMoved(this->mLineNumber);
    }
    return value;
}


//! Tells the line that its parent connector has been connected at both ends
void GUIConnectorLine::setConnected()
{
    mParentConnectorEndPortConnected = true;
}


//! Reimplementation of setLine; stores the start and end positions before changing them
//! @param x1 is the x-coordinate of the start position.
//! @param y1 is the y-coordinate of the start position.
//! @param x2 is the x-coordinate of the end position.
//! @param y2 is the y-coordinate of the end position.
void GUIConnectorLine::setLine(qreal x1, qreal y1, qreal x2, qreal y2)
{
    this->startPos = QPointF(x1,y1);
    this->endPos = QPointF(x2,y2);
    if(this->mHasEndArrow)
    {
        delete(this->mArrowLine1);
        delete(this->mArrowLine2);
        this->addEndArrow();
    }
    else if(this->mHasStartArrow)
    {
        delete(this->mArrowLine1);
        delete(this->mArrowLine2);
        this->addStartArrow();
    }
    QGraphicsLineItem::setLine(x1,y1,x2,y2);
}


//! Adds an arrow at the end of the line.
//! @see addStartArrow()
void GUIConnectorLine::addEndArrow()
{
    qreal angle = atan2((this->endPos.y()-this->startPos.y()), (this->endPos.x()-this->startPos.x()));
    mArrowLine1 = new QGraphicsLineItem(this->endPos.x(),
                                        this->endPos.y(),
                                        this->endPos.x()-mArrowSize*cos(angle+mArrowAngle),
                                        this->endPos.y()-mArrowSize*sin(angle+mArrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->endPos.x(),
                                        this->endPos.y(),
                                        this->endPos.x()-mArrowSize*cos(angle-mArrowAngle),
                                        this->endPos.y()-mArrowSize*sin(angle-mArrowAngle), this);
    this->setPen(this->pen());
    this->mHasEndArrow = true;
}


//! Adds an arrow at the start of the line.
//! @see addEndArrow()
void GUIConnectorLine::addStartArrow()
{
    qreal angle = atan2((this->endPos.y()-this->startPos.y()), (this->endPos.x()-this->startPos.x()));
    mArrowLine1 = new QGraphicsLineItem(this->startPos.x(),
                                        this->startPos.y(),
                                        this->startPos.x()+mArrowSize*cos(angle+mArrowAngle),
                                        this->startPos.y()+mArrowSize*sin(angle+mArrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->startPos.x(),
                                        this->startPos.y(),
                                        this->startPos.x()+mArrowSize*cos(angle-mArrowAngle),
                                        this->startPos.y()+mArrowSize*sin(angle-mArrowAngle), this);
    this->setPen(this->pen());
    this->mHasStartArrow = true;
}


//! Reimplementation of inherited setPen function to include arrow pen too.
void GUIConnectorLine::setPen (const QPen &pen)
{
    QGraphicsLineItem::setPen(pen);
    if(this->mHasStartArrow | this->mHasEndArrow)       //Update arrow lines two, but ignore dashes
    {
        QPen tempPen = this->pen();
        tempPen = QPen(tempPen.color(), tempPen.width(), Qt::SolidLine);
        mArrowLine1->setPen(tempPen);
        mArrowLine2->setPen(tempPen);
        mArrowLine1->line();
    }
}


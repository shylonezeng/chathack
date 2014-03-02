#include "worker.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutexLocker>
#include <QDebug>
#include <QtNetwork>

QMutex Worker::mutex;

Worker::Worker(qintptr socketDescriptor, QObject *parent, QThread *_self, QTcpSocket *_client, QTcpServer *_server) :
    QObject(parent), socketFd(socketDescriptor),
    log("chathack_workerdaemon.log"), self(_self), client(_client),
    server(_server)
{
    myCmds << "slogin" << "clogin" << "sjoin" << "cjoin" << "sleave"
               << "cleave" << "slogout" << "clogout" << "sexit" << "cexit"
               << "sulroom" << "culroom" << "ssmroom" << "csmroom" << "crecvmsg";
    connect(this,SIGNAL(shouldRun()),this,SLOT(run()));
    connect(this,SIGNAL(TryHttpFinish(QNetworkReply*)),this,SLOT(onHttpFinish(QNetworkReply*)));
    uuid = -1;
    //mgr = new QNetworkAccessManager();
    //url_base = "http://192.168.62.193/chathack/?";
}

Worker::~Worker()
{
    //qDebug() << "destroyed";
    mutex.lock();
    log.log("Worker: I just died.\n");
    mutex.unlock();
}

void Worker::startRun()
{
    emit shouldRun();
}

void Worker::HttpFinish(QNetworkReply *rpy)
{
    emit TryHttpFinish(rpy);
}

void Worker::run()
{
    //qDebug() << "void Worker::run()";
    client = new QTcpSocket();
    if (!client->setSocketDescriptor(socketFd))
    {
        mutex.lock();
        log.log("Worker: Thread error, invalid socket descriptor.\n");
        mutex.unlock();
        return;
    }

    connect(self,SIGNAL(finished()),client,SLOT(deleteLater()));
    connect(client,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
    connect(client,SIGNAL(disconnected()),this,SLOT(onDisconnect()));


    //connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(onHttpFinish(QNetworkReply*)));
    //connect(self,SIGNAL(finished()),mgr,SLOT(deleteLater()));


    mutex.lock();
    log.log("Worker: Worker thread spawned successfully.\n");
    mutex.unlock();

    //client->waitForDisconnected();
}


bool Worker::setup() // initial server setup
{
    return false;
}

bool Worker::processRequest(QString cmd) // will spawn a thread to handle client request
{
    //a  "sjoin"  b  "sjoin"
    //slogin|channel|type|slogin
    QStringList args(parse(cmd));
    QString qryString;
    //qDebug() << "size " << args.size();
    if(args.size() == 0)
        return false;
    //qDebug() << "a " << args[0] << " b "<< args[args.size()-1];
    if(args[0] != args[args.size()-1])
        return false;
    int cntrl = myCmds.indexOf(args[0]);
    //mgr->get(QNetworkRequest(QUrl(url_base+qryString)));
    switch (cntrl)
    {
    case 0:
        //clogin
        //slogin|room|username|slogin
        //clogin|room|uuid|status|clogin
        qryString = QString("cmd=slogin&c="+args[1]+"&u1=&u2="+args[2]+"&t=&m=");
        emit netRequest(qryString);
        //mgr->get( QNetworkRequest(QUrl(url_base+qryString)) );
        //write_c(QString(myCmds[cntrl+1]+"|hack|28|0|"+myCmds[cntrl+1]+"\n"));
        break;
    case 2:
        //cjoin
        //sjoin|uuid|channel|type|sjoin
        //cjoin|channel|status|cjoin
        qryString = QString("cmd=sjoin&c="+args[2]+"&u1="+args[1]+"&u2=&t="+args[3]+"&m=");
        //mgr->get( QNetworkRequest(QUrl(url_base+qryString)) );
        write_c(QString(myCmds[cntrl+1]+"|hack|0|"+myCmds[cntrl+1]+"\n"));
        break;
    case 4:
        //cleave
        //cleave|channel|status|cleave
        //sleave|uuid|channel|type|sleave
        qryString = QString("cmd=sleave&c="+args[2]+"&u1="+args[1]+"&u2=&t="+args[3]+"&m=");
        //mgr->get( QNetworkRequest(QUrl(url_base+qryString)) );
        write_c(QString(myCmds[cntrl+1]+"|hack|0|"+myCmds[cntrl+1]+"\n"));
        break;
    case 6:
        //clogout
        //clogout|status|clogout
        qryString = QString("cmd=slogout&c=&u1="+args[1]+"&u2=&t=&m=");
        //mgr->get( QNetworkRequest(QUrl(url_base+qryString)) );
        write_c(QString(myCmds[cntrl+1]+"|0|"+myCmds[cntrl+1]+"\n"));
        break;
    case 8:
        //cexit
        //cexit|status|cexit
        qryString = QString("cmd=sexit&c=&u1="+args[1]+"&u2=&t=&m=");
        //mgr->get( QNetworkRequest(QUrl(url_base+qryString)) );
        write_c(QString(myCmds[cntrl+1]+"|0|"+myCmds[cntrl+1]+"\n"));
        break;
    case 10:
        //culroom
        //culroom|<comma delimited list of users>|culroom
        qryString = QString("cmd=sulroom&c="+args[1]+"&u1=&u2=&t=&m=");
        //mgr->get( QNetworkRequest(QUrl(url_base+qryString)) );
        write_c(QString(myCmds[cntrl+1]+"|john,brian|"+myCmds[cntrl+1]+"\n"));
        break;
    case 12:
        //csmroom
        //csmroom|room|status|csmroom
        //ssmroom|uuid|room|type|message (must escape vbar)|ssmroom
        qryString = QString("cmd=ssmroom&c="+args[2]+"&u1="+args[1]+"&u2=&t="+args[3]+"&m="+args[4]);
        //mgr->get( QNetworkRequest(QUrl(url_base+qryString)) );
        write_c(QString(myCmds[cntrl+1]+"|hack|0|"+myCmds[cntrl+1]+"\n"));
        //crecvmsg
        //crecvmsg|room|user|message|crecvmsg
        qryString = QString("cmd=srecvmsg&r="+args[1]+"&u1="+args[1]+"&u2=&c=&t=&m=");
        //mgr->get( QNetworkRequest(QUrl(url_base+qryString)) );
        write_c(QString(myCmds[cntrl+2]+"|hack|dan|"+args[4]+"|"+myCmds[cntrl+2]+"\n"));
        break;
    default:
        write_c(QString("Hello, client. Idk wtf you want.\n"));
        break;
    }

    return true;
}

void Worker::onReadyRead()
{
    read();
}

void Worker::read()
{
    QString log_str("");
    QByteArray clientByteArray;
    while(client->canReadLine())
    {
        clientByteArray = client->readLine();
        if( !clientByteArray.contains("\0") )
        {
            log_str = "Worker: Read null character from client. Ending read.\n";
            break;
        }
        log_str = "Worker: Read the following from client... " + QString(clientByteArray.constData());
    }

    {
        QMutexLocker locker(&mutex);
        log.log(log_str);
    }

    mutex.lock();
    log.log("Worker: Server finished reading from client.\n");
    mutex.unlock();

    if(clientByteArray.constData()[strlen(clientByteArray.constData())-1] == '\n')
        clientByteArray.chop(1);
    if (processRequest( QString(clientByteArray.constData()) ) )
    {
        mutex.lock();
        log.log("Worker: Processed client request successfully.\n");
        mutex.unlock();
    }
    else
    {
        mutex.lock();
        log.log("Worker: Could not processed client request.\n");
        mutex.unlock();
    }
}


bool Worker::write_c(QString msg)
{
    mutex.lock();
    log.log("Worker: Writing response to client.\n");
    mutex.unlock();
    QString tmp(msg);
    int msgLen = tmp.length();
    tmp[msgLen] = '\n';
    return client->write(tmp.toStdString().c_str()) != -1;
}

QStringList Worker::parse(QString cmd)
{
    QRegExp regex("(\\||\\\n)"); // vertical bar
    return cmd.split(regex);
}


void Worker::onDisconnect()
{
    mutex.lock();
    log.log("Worker: Client disconnection occurred.\n");
    mutex.unlock();
    emit clientDisconnect(self);
}


void Worker::onHttpFinish(QNetworkReply *rply)
{
    QByteArray bts = rply->readAll();
    QString str(bts);
    //qDebug() << str;
    write_c(str);
}
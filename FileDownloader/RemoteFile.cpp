#include <QFile>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include <QLoggingCategory>

#include "RemoteFile.h"

const QLoggingCategory file("RemoteFile");

RemoteFile::RemoteFile(QUrl url, QString path) {
  setAutoDelete(false);
  
  _url = url;
  _status = Status::Unknown;
  _filePath = path;
  
  _networker = nullptr;
  _connection = nullptr;
}

const QUrl& RemoteFile::url() const {
  return _url;
}

const RemoteFile::Status& RemoteFile::status() const {
  return _status;
}

const QString& RemoteFile::filePath() const {
  return _filePath;
}

const QString& RemoteFile::errorString() const {
  return _errorString;
}

void RemoteFile::startDownload() {
  run();
}

int RemoteFile::progress() const {
  return int((float(_bytesRead) / float(_bytesTotal)) * 100);
}

void RemoteFile::run() {
  cleanConnection();
  
  QNetworkRequest request(url());
  _networker = new QNetworkAccessManager();
  
  _connection = _networker->get(request);
  if (_connection == nullptr) {
    qCritical(file) << "Error download file: QNetworkAccessManager return nullptr by request." << url();
    return;
  }
  
  QEventLoop downloadLoop;
  
  QObject::connect(_connection, &QNetworkReply::finished, &downloadLoop, &QEventLoop::quit);
  QObject::connect(_connection, &QNetworkReply::finished, [this](){ 
    if (_connection->error() == QNetworkReply::NoError)
      _status = Status::Loaded; 
    else {
      _status = Status::Error;
      _errorString = _connection->errorString();
    }
  });
  QObject::connect(_connection, &QNetworkReply::readyRead, [this](){
    QFile dst(filePath());
    if (dst.open(QFile::WriteOnly | QFile::Append)) {
      dst.write(_connection->readAll());
      dst.close();
    }
    else {
      _errorString = dst.errorString();
      _connection->abort();
      
      qCritical(file) << "Error download file: Dst file wont be open for write." << errorString();
    }
  });
  
  QObject::connect(_connection, &QNetworkReply::errorOccurred, [this](QNetworkReply::NetworkError) {
    qCritical(file) << "Error download file: Connection error" << _connection->errorString();
    _connection->abort();
  });
  
  QObject::connect(_connection, &QNetworkReply::downloadProgress, [this](quint64 avail, quint64 total) {
    _bytesRead = avail;
    _bytesTotal = total;
  });
  
  _status = Status::Loading;
  downloadLoop.exec();
}

void RemoteFile::cleanConnection() {
  if (_connection != nullptr)
  {
    _connection->abort();
    
    delete _connection;
    delete _networker;
    
    _connection = nullptr;
    _networker = nullptr;
  }
  
  _errorString.clear();
}

#ifdef QLOGGINGCATEGORY_H
QDebug operator<<(QDebug output, RemoteFile::Status status) {
  switch (status) {
    case RemoteFile::Loading:
      output << "Loading";
      break;
    case RemoteFile::Loaded:
      output << "Loaded";
      break;
    case RemoteFile::Error:
      output << "Error";
      break;
    case RemoteFile::Unknown:
      output << "Unknown";
      break;  
  }
  
  return output;
}
#endif

#ifndef REMOTEFILE_H
#define REMOTEFILE_H

#include <QUrl>
#include <QString>
#include <QRunnable>

class RemoteFile final : public QRunnable
{
public:
  enum Status {
    Loading,
    Loaded,
    Error,
    Unknown
  };
  
public:
  RemoteFile(QUrl url, QString filePath);
  
  const QUrl& url() const;
  const Status& status() const;
  const QString& filePath() const;
  const QString& errorString() const;
  
  int progress() const;
  void startDownload();
  
private:
  void run() override;
  void cleanConnection();
  
private:
  class QNetworkReply* _connection;
  class QNetworkAccessManager* _networker;
  
  uint64_t _bytesRead;
  uint64_t _bytesTotal;
  
  QUrl _url;
  Status _status;
  QString _filePath;
  QString _errorString;
};

#ifdef QLOGGINGCATEGORY_H
QDebug operator<<(QDebug output, RemoteFile::Status);
#endif

#endif // REMOTEFILE_H

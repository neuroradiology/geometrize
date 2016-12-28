#include "completionhandlers.h"

#include <QDebug>
#include <QNetworkReply>
#include <QPixmap>
#include <QRegExp>
#include <QString>

#include "network/downloader.h"
#include "network/networkactions.h"
#include "recentitems.h"
#include "sharedapp.h"

namespace geometrize
{

namespace network
{

namespace completionhandlers
{

void onImageDownloadComplete(network::Downloader* self, QNetworkReply::NetworkError error)
{
    qDebug() << "FINISHED DOWNLOADING WITH ERROR" << error; // TODO error checks

    QPixmap image;
    image.loadFromData(self->downloadedData());
    app::SharedApp::get().getRecentFiles().add(self->getUrl().toString());
    app::createImageJob(nullptr, self->getUrl().toString(), image);

    delete self;
}

void onWebpageDownloadComplete(network::Downloader* self, QNetworkReply::NetworkError error)
{
    qDebug() << "FINISHED DOWNLOADING WITH ERROR" << error; // TODO error checks

    const QString document(self->downloadedData());

    QRegExp imageTagRegex("\\<img[^\\>]*src\\s*=\\s*\"([^\"]*)\"[^\\>]*\\>", Qt::CaseInsensitive);
    imageTagRegex.setMinimal(true);

    QStringList imageMatches;
    QStringList urlMatches;

    int offset{0};
    while((offset = imageTagRegex.indexIn(document, offset)) != -1) {
        offset += imageTagRegex.matchedLength();
        imageMatches.append(imageTagRegex.cap(0)); // Should hold complete img tag
        urlMatches.append(imageTagRegex.cap(1)); // Should hold only src property
    }

    qDebug() << imageMatches;
    qDebug() << urlMatches;

    QList<QUrl> imageUrls;
    for(const QString& url : urlMatches) {
        imageUrls.push_back(QUrl(url));
    }

    const QString baseUrl{self->getUrl().adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toString()};
    for(QUrl& url : imageUrls) {
        if(url.isRelative()) {
            url = QUrl(baseUrl + url.toString());
        }
        network::downloadImage(url, onImageDownloadComplete);
    }

    delete self;
}

}

}

}
/**
 * @file dataworker.cpp
 * @brief Implementacja klasy DataWorker odpowiedzialnej za pobieranie danych do wykresu z sieci.
 */
#include "dataworker.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
/**
 * @brief Konstruktor klasy DataWorker.
 * @param sensorId ID sensora.
 * @param from Data początkowa.
 * @param to Data końcowa.
 */
DataWorker::DataWorker(int sensorId, const QDateTime &from, const QDateTime &to)
    : sensorId(sensorId), dateFrom(from), dateTo(to)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &DataWorker::onReply);
}
/**
 * @brief Rozpoczyna operację pobierania danych do wykresu z API.
 */
void DataWorker::start()
{
    int hours = int(dateFrom.secsTo(dateTo)) / 3600;
    QString url = QString("https://api.gios.gov.pl/pjp-api/rest/archivalData/getDataBySensor/%1?size=%2&dateFrom=%3piotr%4pyka00&dateTo=%5piotr%6pyka00")
                      .arg(sensorId).arg(hours).arg(dateFrom.toString("yyyy-MM-dd")).arg(dateFrom.toString("HH"))
                      .arg(dateTo.toString("yyyy-MM-dd")).arg(dateTo.toString("HH"));

    url.replace("piotr", "%20");
    url.replace("pyka", "%3A");
    QUrl qurl(url);
    QNetworkRequest request(qurl);
    manager->get(request);
}

/**
 * @brief Obsługuje odpowiedź z zapytania sieciowego.
 * @param reply Odpowiedź HTTP zawierająca dane.
 */
void DataWorker::onReply(QNetworkReply *reply)
{
    QVector<DataPoint> points;

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray results = doc.object().value("Lista archiwalnych wyników pomiarów").toArray();

        for (const QJsonValue &val : std::as_const(results)) {
            QJsonObject obj = val.toObject();
            QString dateStr = obj["Data"].toString();
            double value = obj["Wartość"].toDouble();
            QDateTime timestamp = QDateTime::fromString(dateStr, "yyyy-MM-dd HH:mm:ss");
            points.append({ timestamp, value });
        }
    }

    reply->deleteLater();
    emit dataReady(points);
}

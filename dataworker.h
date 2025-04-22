/**
 * @file dataworker.h
 * @brief Definicja klasy DataWorker odpowiedzialnej za pobieranie danych do wykresu.
 */

#ifndef DATAWORKER_H
#define DATAWORKER_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

/**
 * @struct DataPoint
 * @brief Reprezentuje pojedynczy pomiar wartości wraz z czasem.
 */
struct DataPoint {
    QDateTime timestamp;
    double value;
};

/**
 * @class DataWorker
 * @brief Klasa odpowiedzialna za asynchroniczne pobieranie danych z sieci.
 */
class DataWorker : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy DataWorker.
     * @param sensorId Identyfikator sensora.
     * @param from Data początkowa zakresu.
     * @param to Data końcowa zakresu.
     */
    explicit DataWorker(int sensorId, const QDateTime &from, const QDateTime &to);

    /**
     * @brief Rozpoczyna pobieranie danych.
     */
    void start();

signals:
    /**
     * @brief Emitowany po zakończeniu pobierania danych.
     * @param dataPoints Wektor pobranych danych.
     */
    void dataReady(QVector<DataPoint> dataPoints);

private slots:
    /**
     * @brief Przetwarza odpowiedź sieciową.
     * @param reply Odpowiedź HTTP.
     */
    void onReply(QNetworkReply *reply);

private:
    int sensorId;
    QDateTime dateFrom, dateTo;
    QNetworkAccessManager *manager;
};

#endif // DATAWORKER_H

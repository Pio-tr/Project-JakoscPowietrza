/**
 * @file jsonstorage.h
 * @brief Definicja klasy JsonStorage do przechowywania danych lokalnych.
 */

#ifndef JSONSTORAGE_H
#define JSONSTORAGE_H

#include <QString>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>

struct DataPoint;

/**
 * @class JsonStorage
 * @brief Klasa pomocnicza do zapisu i odczytu danych z plików JSON.
 */
class JsonStorage
{
public:
    /** @brief Zapisuje listę stacji do pliku JSON. */
    static void saveStation(const QJsonArray &stations);

    /** @brief Zapisuje listę sensorów dla danej stacji. */
    static void saveSensors(int stationId, const QJsonArray &sensors);

    /** @brief Zapisuje pomiary dla danego sensora. */
    static void saveMeasurements(int stationId, int sensorId, const QVector<DataPoint> &points);

    /** @brief Wczytuje listę stacji z lokalnego pliku. */
    static QJsonArray loadStationList();

    /** @brief Wczytuje listę sensorów przypisaną do stacji. */
    static QJsonArray loadSensors(int stationId);

    /** @brief Wczytuje dane pomiarowe z pliku. */
    static QVector<DataPoint> loadMeasurements(int stationId, int sensorId);
};

#endif // JSONSTORAGE_H

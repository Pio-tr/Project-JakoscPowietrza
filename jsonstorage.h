/**
 * @file jsonstorage.h
 */
#ifndef JSONSTORAGE_H
#define JSONSTORAGE_H

#include <QString>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>

struct DataPoint;

/** @brief Zwraca ścieżkę do katalogu z plikami JSON. */
QString getJsonDir();

/** @brief Tworzy pełną ścieżkę do pliku JSON. */
QString getJsonFilePath(const QString &filename);

/** @brief Wczytuje dokument JSON z pliku. */
QJsonDocument loadJsonDoc(const QString &filename);

/** @brief Zapisuje dane do pliku JSON. */
void saveJsonDoc(const QString &filename, const QJsonArray &array);

/** @brief Zapisuje listę stacji do pliku JSON. */
void saveStation(const QJsonArray &stations);

/** @brief Zapisuje listę sensorów dla danej stacji. */
void saveSensors(int stationId, const QJsonArray &sensors);

/** @brief Zapisuje pomiary dla danego sensora. */
void saveMeasurements(int stationId, int sensorId, const QVector<DataPoint> &points);

/** @brief Wczytuje listę stacji z lokalnego pliku. */
QJsonArray loadStationList();

/** @brief Wczytuje listę sensorów przypisaną do stacji. */
QJsonArray loadSensors(int stationId);

/** @brief Wczytuje dane pomiarowe z pliku. */
QVector<DataPoint> loadMeasurements(int stationId, int sensorId);

#endif // JSONSTORAGE_H

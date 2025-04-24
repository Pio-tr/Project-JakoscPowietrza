/**
 * @file jsonstorage.cpp
 */
#include "jsonstorage.h"
#include "dataworker.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
/**
 * @brief Zwraca ścieżkę do katalogu z plikami JSON.
 * @return Ścieżka do katalogu jako QString.
 */
QString getJsonDir() {
    QString dirPath = "bazajson";
    QDir dir;
    if (!dir.exists(dirPath))
        dir.mkpath(dirPath);
    return dirPath;
}
/**
 * @brief Tworzy pełną ścieżkę do pliku JSON.
 * @param filename Nazwa pliku (bez katalogu).
 * @return Pełna ścieżka do pliku.
 */
QString getJsonFilePath(const QString &filename) {
    return getJsonDir() + "/" + filename;
}
/**
 * @brief Wczytuje dokument JSON z pliku.
 * @param filename Ścieżka do pliku.
 * @return QJsonDocument z danymi lub pusty dokument, jeśli plik nie istnieje.
 */
QJsonDocument loadJsonDoc(const QString &filename) {
    QFile file(filename);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) return QJsonDocument();
    QByteArray data = file.readAll();
    file.close();
    return QJsonDocument::fromJson(data);
}
/**
 * @brief Zapisuje dane do pliku JSON.
 * @param filename Ścieżka do pliku.
 * @param array Dane w formacie QJsonArray.
 */
void saveJsonDoc(const QString &filename, const QJsonArray &array) {
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(array).toJson());
        file.close();
    }
}

/**
 * @brief Zapisuje listę stacji do lokalnego pliku.
 * @param stations Lista stacji jako QJsonArray.
 */
void saveStation(const QJsonArray &stations) {
    QString filename = getJsonFilePath("listastacji.json");
    QJsonArray currentFile = loadJsonDoc(filename).array();
    QSet<int> existingIds;

    for (const QJsonValue &val : currentFile)
        existingIds << val.toObject().value("id").toInt();

    for (const QJsonValue &val : stations) {
        QJsonObject original = val.toObject();
        int id = original.value("id").toInt();
        if (!existingIds.contains(id)) {
            QJsonObject minimalObj;
            minimalObj.insert("id", id);
            minimalObj.insert("stationName", original.value("stationName").toString());
            currentFile.append(minimalObj);
        }
    }

    saveJsonDoc(filename, currentFile);
}
/**
 * @brief Zapisuje listę sensorów dla danej stacji.
 * @param stationId ID stacji.
 * @param sensors Lista sensorów jako QJsonArray.
 */
void saveSensors(int stationId, const QJsonArray &sensors) {
    QString filename = getJsonFilePath(QString("%1-listasensorow.json").arg(stationId));
    QJsonArray currentFile = loadJsonDoc(filename).array();
    QSet<int> existingIds;

    for (const QJsonValue &val : currentFile)
        existingIds << val.toObject().value("id").toInt();

    for (const QJsonValue &val : sensors) {
        QJsonObject original = val.toObject();
        int id = original.value("id").toInt();

        if (!existingIds.contains(id)) {
            QJsonObject minimalObj;
            minimalObj.insert("id", id);
            minimalObj.insert("paramName", original.value("param").toObject().value("paramName").toString());
            currentFile.append(minimalObj);
            existingIds.insert(id);
        }
    }

    saveJsonDoc(filename, currentFile);
}
/**
 * @brief Zapisuje dane pomiarowe dla danego sensora.
 * @param stationId ID stacji.
 * @param sensorId ID sensora.
 * @param points Lista punktów pomiarowych.
 */
void saveMeasurements(int stationId, int sensorId, const QVector<DataPoint> &points) {
    QString filename = getJsonFilePath(QString("%1-%2.json").arg(stationId).arg(sensorId));
    QJsonArray merged = loadJsonDoc(filename).array();
    QSet<QString> existingTimestamps;

    for (const QJsonValue &val : merged)
        existingTimestamps.insert(val.toObject()["timestamp"].toString());

    for (const DataPoint &dp : points) {
        QString ts = dp.timestamp.toString(Qt::ISODate);
        if (!existingTimestamps.contains(ts)) {
            QJsonObject obj;
            obj["timestamp"] = ts;
            obj["value"] = dp.value;
            merged.append(obj);
        }
    }

    saveJsonDoc(filename, merged);
}
/**
 * @brief Wczytuje listę stacji z pliku JSON.
 * @return QJsonArray zawierający listę stacji.
 */
QJsonArray loadStationList() {
    QJsonDocument doc = loadJsonDoc(getJsonFilePath("listastacji.json"));
    return doc.array();
}
/**
 * @brief Wczytuje listę sensorów przypisaną do stacji.
 * @param stationId ID stacji.
 * @return QJsonArray zawierający listę sensorów.
 */
QJsonArray loadSensors(int stationId) {
    QString filename = getJsonFilePath(QString("%1-listasensorow.json").arg(stationId));
    QJsonDocument doc = loadJsonDoc(filename);
    return doc.array();
}
/**
 * @brief Wczytuje dane pomiarowe z pliku JSON.
 * @param stationId ID stacji.
 * @param sensorId ID sensora.
 * @return Wektor punktów pomiarowych typu DataPoint.
 */
QVector<DataPoint> loadMeasurements(int stationId, int sensorId) {
    QVector<DataPoint> data;
    QString filename = getJsonFilePath(QString("%1-%2.json").arg(stationId).arg(sensorId));
    QJsonDocument doc = loadJsonDoc(filename);

    for (const QJsonValue &val : doc.array()) {
        QJsonObject obj = val.toObject();
        QDateTime ts = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        double value = obj["value"].toDouble();
        data.append({ ts, value });
    }
    return data;
}

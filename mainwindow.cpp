/**
 * @file mainwindow.cpp
 * @brief Implementacja klasy MainWindow odpowiedzialnej za interfejs użytkownika.
 */

#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QThread>
#include "jsonstorage.h"
#include "dataworker.h"
#include "chartwindow.h"

/**
 * @brief Konstruktor klasy MainWindow.
 *
 * Inicjalizuje komponenty interfejsu użytkownika, ustawia połączenia sygnałów i slotów,
 * ustawia domyślne daty oraz pobiera listę stacji pomiarowych z API.
 *
 * @param parent Wskaźnik na obiekt rodzica.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    comboBox(new QComboBox(this)),
    comboBoxSensors(new QComboBox(this)),
    backButton(new QPushButton("Cofnij", this)),
    nextButton(new QPushButton("Dalej", this)),
    infoLabel(new QLabel(this)),
    dateFrom(new QLabel("Data i godzina OD: ")),
    dateTo(new QLabel("Data i godzina DO: ")),
    dateTimeFrom(new QDateTimeEdit(this)),
    dateTimeTo(new QDateTimeEdit(this)),
    generateChartButton(new QPushButton("Wygeneruj wykres", this)),
    networkManager(new QNetworkAccessManager(this)),
    currentStep(1)
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *vertical = new QVBoxLayout;
    QHBoxLayout *horizontal = new QHBoxLayout;
    horizontal->addWidget(backButton);
    horizontal->addWidget(nextButton);
    dateTimeFrom->setDisplayFormat("yyyy-MM-dd HH");
    dateTimeTo->setDisplayFormat("yyyy-MM-dd HH");
    vertical->addWidget(infoLabel);
    vertical->addWidget(comboBox);
    vertical->addWidget(comboBoxSensors);
    vertical->addLayout(horizontal);
    vertical->addWidget(dateFrom);
    vertical->addWidget(dateTimeFrom);
    vertical->addWidget(dateTo);
    vertical->addWidget(dateTimeTo);
    vertical->addWidget(generateChartButton);
    central->setLayout(vertical);

    connect(backButton, &QPushButton::clicked, this, &MainWindow::onBackClicked);
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::onNextClicked);
    connect(generateChartButton, &QPushButton::clicked, this, &MainWindow::onGenerateClicked);
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onDataReceived);

    dateTimeFrom->setDateTime(QDateTime::currentDateTime().addDays(-1));
    dateTimeTo->setDateTime(QDateTime::currentDateTime());
    dateTimeFrom->setMaximumDateTime(QDateTime::currentDateTime());
    dateTimeTo->setMaximumDateTime(QDateTime::currentDateTime());
    updateUI();
    fetchDataFromUrl("https://api.gios.gov.pl/pjp-api/rest/station/findAll?sort=stationName");
}

/**
 * @brief Destruktor klasy MainWindow.
 */
MainWindow::~MainWindow() {}

/**
 * @brief Obsługuje kliknięcie przycisku "Cofnij".
 *
 * Przechodzi do poprzedniego kroku formularza, jeśli to możliwe.
 */
void MainWindow::onBackClicked()
{
    if (currentStep == 2) currentStep = 1;
    else if (currentStep == 3) currentStep = 2;
    updateUI();
}

/**
 * @brief Obsługuje kliknięcie przycisku "Dalej".
 *
 * Pobiera dane w zależności od aktualnego kroku formularza i przechodzi dalej.
 */
void MainWindow::onNextClicked()
{
    if (currentStep == 1) {
        currentStep = 2;
        int stationId = comboBox->currentData().toInt();
        selectedStationName = comboBox->currentText();
        QString indexUrl = QString("https://api.gios.gov.pl/pjp-api/rest/aqindex/getIndex/%1").arg(stationId);
        avoidWarning = true;
        fetchDataFromUrl(indexUrl);
        QString sensorsUrl = QString("https://api.gios.gov.pl/pjp-api/rest/station/sensors/%1").arg(stationId);
        fetchDataFromUrl(sensorsUrl);
    } else if (currentStep == 2) {
        currentStep = 3;
        paramName = comboBoxSensors->currentText();
        int sensorId = comboBoxSensors->currentData().toInt();
        QString dataUrl = QString("https://api.gios.gov.pl/pjp-api/rest/data/getData/%1").arg(sensorId);
        fetchDataFromUrl(dataUrl);
    }
    updateUI();
}

/**
 * @brief Wysyła zapytanie HTTP do wskazanego adresu URL.
 *
 * @param url Adres URL, z którego mają zostać pobrane dane.
 */
void MainWindow::fetchDataFromUrl(const QString &url)
{
    QUrl qurl(url);
    QNetworkRequest request(qurl);
    networkManager->get(request);
}

/**
 * @brief Obsługuje kliknięcie przycisku "Wygeneruj wykres".
 *
 * Tworzy osobny wątek do pobierania danych i otwiera nowe okno z wykresem.
 */
void MainWindow::onGenerateClicked()
{
    dateTimeFrom->setMaximumDateTime(QDateTime::currentDateTime());
    dateTimeTo->setMaximumDateTime(QDateTime::currentDateTime());
    QDateTime from = dateTimeFrom->dateTime();
    QDateTime to = dateTimeTo->dateTime();


    int sensorId = comboBoxSensors->currentData().toInt();
    int stationId = comboBox->currentData().toInt();
    QThread *thread = new QThread;
    DataWorker *worker = new DataWorker(sensorId, from, to);
    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &DataWorker::start);
    connect(worker, &DataWorker::dataReady, this, [=](QVector<DataPoint> data) {

        QVector<QPointF> points;
        QVector<QDateTime> timestamps;

        if (data.isEmpty()) {
            QVector<DataPoint> offline = loadMeasurements(stationId, sensorId);

            QVector<DataPoint> filtered;
            for (const DataPoint &dp : std::as_const(offline)) {
                if (dp.timestamp >= from && dp.timestamp <= to) {
                    filtered.append(dp);
                }
            }

            if (!filtered.isEmpty()) {
                QMessageBox::information(this, "Brak połączenia", "Nie udało się pobrać danych z sieci.\nZaładowano dane lokalne.");
                std::sort(filtered.begin(), filtered.end(), [](const DataPoint &a, const DataPoint &b) {
                    return a.timestamp < b.timestamp;
                });
                data = filtered;
            } else {
                QMessageBox::information(this, "Brak połączenia", "Nie udało się pobrać danych z sieci.\nBrak danych lokalnych w podanym zakresie.");
                return;
            }
        }

        double sum = 0, min = data[0].value, max = data[0].value;
        QDateTime minTime = data[0].timestamp, maxTime = data[0].timestamp;

        for (int i = 0; i < data.size(); ++i) {
            double val = data[i].value;
            points.append(QPointF(i, val));
            timestamps.append(data[i].timestamp);
            sum += val;
            if (val < min) {
                min = val;
                minTime = data[i].timestamp;
            }
            if (val > max) {
                max = val;
                maxTime = data[i].timestamp;
            }
        }

        double avg = sum / data.size();
        QString trend = (points.last().y() > points.first().y()) ? "rośnie" : (points.last().y() < points.first().y()) ? "maleje" : "brak";


        ChartWindow *window = new ChartWindow(points, timestamps, min, minTime, max, maxTime, avg, trend, paramName, selectedStationName);

        window->setAttribute(Qt::WA_DeleteOnClose);
        window->show();
        saveMeasurements(stationId, sensorId, data);
        worker->deleteLater();
        thread->deleteLater();
    });

    connect(worker, &DataWorker::dataReady, thread, &QThread::quit);
    thread->start();
}
/**
 * @brief Obsługuje zakończenie zapytania sieciowego.
 *
 * Przetwarza odpowiedź w formacie JSON i aktualizuje interfejs lub pobiera dane lokalne.
 *
 * @param reply Wskaźnik do odpowiedzi z serwera.
 */
void MainWindow::onDataReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        if (currentStep == 1) {
            QJsonArray stations = loadStationList();
            if (stations.isEmpty()) {
                QMessageBox::warning(this, "Brak połączenia", "Nie udało się pobrać danych z sieci.\nBrak zapisanych stacji.");
                nextButton->setVisible(false);
            }
            else {
                comboBox->clear();
                QMessageBox::warning(this, "Brak połączenia", "Nie udało się pobrać danych z sieci.\nZaładowano dane lokalne.");
                for (const QJsonValue &val : std::as_const(stations)) {
                    QJsonObject obj = val.toObject();
                    comboBox->addItem(obj["stationName"].toString(), obj["id"].toInt());
                }
            }
        } else if (!avoidWarning && currentStep == 2) {
            int stationId = comboBox->currentData().toInt();
            QJsonArray sensors = loadSensors(stationId);
            if (sensors.isEmpty()) {
                QMessageBox::warning(this, "Brak połączenia", "Nie udało się pobrać danych z sieci.\nBrak zapisanych danych dla wybranej stacji.");
                currentStep = 1;
                updateUI();
            }
            else {
                QMessageBox::warning(this, "Brak połączenia", "Nie udało się pobrać danych z sieci.\nZaładowano dane lokalne.");
                comboBoxSensors->clear();
                for (const QJsonValue &val : std::as_const(sensors)) {
                    QJsonObject obj = val.toObject();
                    QString name = obj["paramName"].toString();
                    comboBoxSensors->addItem(name, obj["id"].toInt());
                }
            }
        }
        avoidWarning=false;
        reply->deleteLater();
        return;
    }


    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
    if (jsonDoc.isArray()) {
        QJsonArray jsonArray = jsonDoc.array();

        if (currentStep == 1) {
            comboBox->clear();
            for (const QJsonValue &value : std::as_const(jsonArray)) {
                QJsonObject obj = value.toObject();
                QString name = obj["stationName"].toString();
                int stationId = obj["id"].toInt();
                if (!name.isEmpty()) {
                    comboBox->addItem(name, stationId);
                }
            }
            saveStation(jsonArray);
        }
        else if (currentStep == 2) {
            comboBoxSensors->clear();
            for (const QJsonValue &value : std::as_const(jsonArray)) {
                QJsonObject sensorObj = value.toObject();
                QJsonObject paramObj = sensorObj["param"].toObject();
                QString paramName = paramObj["paramName"].toString();
                int sensorId = sensorObj["id"].toInt();
                comboBoxSensors->addItem(paramName, sensorId);
            }
            int stationId = comboBox->currentData().toInt();
            saveSensors(stationId, jsonArray);
        }
    }
    else if (jsonDoc.isObject()) {
        QJsonObject obj = jsonDoc.object();
        if (obj.contains("stIndexLevel")) {
            QJsonObject indexLevel = obj["stIndexLevel"].toObject();
            airQuality = "\nObecny indeks jakości powietrza: " +indexLevel["indexLevelName"].toString();
        }
        else if (obj.contains("values")) {
            QJsonArray values = obj["values"].toArray();

            for (const QJsonValue &v : std::as_const(values)) {
                QJsonObject valObj = v.toObject();
                QJsonValue value = valObj["value"];
                if (!value.isNull()) {
                    paramValue = "\nWartość najnowszego pomiaru: " + QString::number(value.toDouble());
                    break;
                }
            }
        }
    }
    updateUI();
    reply->deleteLater();
}
/**
 * @brief Aktualizuje stan i widoczność komponentów UI w zależności od aktualnego kroku.
 */
void MainWindow::updateUI()
{
    comboBox->setVisible(false);
    comboBoxSensors->setVisible(false);
    backButton->setVisible(false);
    nextButton->setVisible(false);
    dateTimeFrom->setVisible(false);
    dateTimeTo->setVisible(false);
    generateChartButton->setVisible(false);
    dateFrom->setVisible(false);
    dateTo->setVisible(false);

    infoLabel->setText("Wybrana stacja pomiarowa: " + selectedStationName + airQuality + "\nWybrany parametr: " + paramName + paramValue);

    switch (currentStep) {
    case 1:
        comboBox->setVisible(true);
        nextButton->setVisible(true);
        infoLabel->setText("Wybierz stację pomiarową: ");
        break;
    case 2:
        backButton->setVisible(true);
        comboBoxSensors->setVisible(true);
        nextButton->setVisible(true);
        infoLabel->setText("Wybrana stacja pomiarowa: " + selectedStationName + airQuality + "\nWybierz parametr: ");
        break;
    case 3:
        backButton->setVisible(true);
        dateTimeFrom->setVisible(true);
        dateTimeTo->setVisible(true);
        generateChartButton->setVisible(true);
        dateFrom->setVisible(true);
        dateTo->setVisible(true);
        break;

    }
    QSize size = infoLabel->sizeHint();
    infoLabel->setMaximumSize(size);
    adjustSize();
}

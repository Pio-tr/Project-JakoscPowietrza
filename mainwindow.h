/**
 * @file mainwindow.h
 * @brief Deklaracja klasy MainWindow - głównego okna aplikacji.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QDateTimeEdit>

/**
 * @class MainWindow
 * @brief Główne okno aplikacji służącej do pobierania i wizualizacji danych o jakości powietrza.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy MainWindow.
     * @param parent Wskaźnik do obiektu nadrzędnego.
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy MainWindow.
     */
    ~MainWindow();

private slots:
    /** @brief Obsługuje kliknięcie przycisku "Cofnij". */
    void onBackClicked();

    /** @brief Obsługuje kliknięcie przycisku "Dalej". */
    void onNextClicked();

    /** @brief Obsługuje kliknięcie przycisku "Wygeneruj wykres". */
    void onGenerateClicked();

    /**
     * @brief Obsługuje odpowiedź z zapytania sieciowego.
     * @param reply Odpowiedź z serwera.
     */
    void onDataReceived(QNetworkReply *reply);

    /**
     * @brief Wysyła zapytanie HTTP do wskazanego URL.
     * @param url Adres URL API.
     */
    void fetchDataFromUrl(const QString &url);

    /** @brief Aktualizuje interfejs użytkownika na podstawie bieżącego kroku. */
    void updateUI();

private:
    QComboBox *comboBox;
    QComboBox *comboBoxSensors;
    QPushButton *backButton;
    QPushButton *nextButton;
    QLabel *infoLabel;
    QLabel *dateFrom;
    QLabel *dateTo;
    QDateTimeEdit *dateTimeFrom;
    QDateTimeEdit *dateTimeTo;
    QPushButton *generateChartButton;

    QNetworkAccessManager *networkManager;
    int currentStep;
    QString selectedStationName;
    QString paramName;
    QString airQuality;
    QString paramValue;
    bool avoidWarning = false;
};

#endif // MAINWINDOW_H

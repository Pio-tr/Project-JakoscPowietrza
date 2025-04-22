/**
 * @file chartwindow.h
 * @brief Definicja klasy ChartWindow wyświetlającej wykres i statystyki.
 */

#ifndef CHARTWINDOW_H
#define CHARTWINDOW_H

#include <QDialog>
#include <QtCharts>

/**
 * @class ChartWindow
 * @brief Okno dialogowe z wykresem danych pomiarowych oraz statystykami.
 */
class ChartWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy ChartWindow.
     * @param dataPoints Punkty do wykreślenia na wykresie.
     * @param timestamps Znaczniki czasu dla każdego punktu.
     * @param minVal Minimalna wartość pomiaru.
     * @param minTime Czas minimalnej wartości.
     * @param maxVal Maksymalna wartość pomiaru.
     * @param maxTime Czas maksymalnej wartości.
     * @param avg Średnia wartość pomiarów.
     * @param trend Określenie trendu zmian.
     * @param paramName Nazwa parametru.
     * @param selectedStationName Nazwa stacji pomiarowej.
     * @param parent Rodzic okna.
     */
    explicit ChartWindow(const QVector<QPointF> &dataPoints, const QVector<QDateTime> &timestamps,
                         double minVal, QDateTime minTime, double maxVal, QDateTime maxTime,
                         double avg, QString trend, QString paramName, QString selectedStationName, QWidget *parent = nullptr);

private:
    QChartView *chartView;
    QLabel *statsLabel;
};

#endif // CHARTWINDOW_H

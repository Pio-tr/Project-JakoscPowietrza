/**
 * @file chartwindow.cpp
 * @brief Implementacja klasy ChartWindow do wizualizacji danych pomiarowych na wykresie.
 */
#include "chartwindow.h"
/**
 * @brief Konstruktor klasy ChartWindow.
 *
 * Tworzy wykres danych pomiarowych i wyświetla statystyki w osobnym oknie.
 *
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
ChartWindow::ChartWindow(const QVector<QPointF> &dataPoints, const QVector<QDateTime> &timestamps,
                         double minVal, QDateTime minTime, double maxVal, QDateTime maxTime,
                         double avg, QString trend, QString paramName, QString selectedStationName,QWidget *parent)
    : QDialog(parent)
{
    QLineSeries *series = new QLineSeries();
    for (int i = 0; i < dataPoints.size(); ++i) {
        qint64 timestamp = timestamps[i].toMSecsSinceEpoch();
        series->append(timestamp, dataPoints[i].y());
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QString("Wykres danych pomiarowych %1 dla stacji %2").arg(paramName).arg(selectedStationName));
    chart->legend()->hide();

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setFormat("yyyy-MM-dd HH:00");
    axisX->setTitleText("Data pomiaru");
    axisX->setTickCount(4);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText(QString("%1").arg(paramName));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    statsLabel = new QLabel(
        QString("Min: %1 (%2)\nMax: %3 (%4)\nŚrednia: %5\nTrend: %6")
            .arg(minVal).arg(minTime.toString("yyyy-MM-dd HH:mm"))
            .arg(maxVal).arg(maxTime.toString("yyyy-MM-dd HH:mm"))
            .arg(avg).arg(trend)
        );

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(chartView);
    layout->addWidget(statsLabel);
    setLayout(layout);
    resize(700, 450);
}

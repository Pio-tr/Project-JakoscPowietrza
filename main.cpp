/**
 * @file main.cpp
 * @brief Główna funkcja uruchamiająca aplikację GUI.
 */

#include "mainwindow.h"
#include <QApplication>

/**
 * @brief Funkcja główna aplikacji.
 * @param argc Liczba argumentów.
 * @param argv Tablica argumentów.
 * @return Kod zakończenia programu.
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

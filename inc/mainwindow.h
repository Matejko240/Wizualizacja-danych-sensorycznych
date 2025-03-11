#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QtCharts>
#include <QtCore>
#include <QtGui>
#include <QChartView>
#include <QMessageBox>
#include "SerialPort.hh"
#include <QLineSeries>
#include <QTranslator>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE
/**
 * @brief Klasa MainWindow zarządza głównym oknem aplikacji.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy MainWindow.
     *
     * Inicjalizuje interfejs użytkownika oraz przygotowuje serie danych dla wykresu.
     *
     * @param parent Wskaźnik na rodzica widgetu.
     */
    MainWindow(QWidget *parent = nullptr);
    /**
     * @brief Destruktor klasy MainWindow.
     *
     * Czyści pamięć po interfejsie użytkownika.
     */
    ~MainWindow();
    /**
     * @brief Struktura przechowująca dane o prędkości.
     */
    struct PredkoscRamka {
        char identyfikator;/**< Identyfikator ramki. */
        float RPM1;/**< Prędkość RPM pierwszego koła. */
        float RPM2;/**< Prędkość RPM drugiego koła. */
    };
    /**
     * @brief Struktura przechowująca dane o położeniu.
     */
    struct PolozenieRamka {
        char identyfikator;/**< Identyfikator ramki. */
        float X;/**< Współrzędna X. */
        float Y;/**< Współrzędna Y. */
        float Z;/**< Współrzędna Z. */
    };
    /**
     * @brief Struktura przechowująca dane o przeszkodach.
     */
    struct PrzeszkodaRamka {
        char identyfikator;/**< Identyfikator ramki. */
        float odleglosc;/**< Odległość do przeszkody. */
    };
    /**
     * @brief Odczytuje ramkę danych.
     *
     * @param ramka Tekst ramki.
     * @param predkoscRamka Struktura przechowująca dane o prędkości.
     * @param polozenieRamka Struktura przechowująca dane o położeniu.
     * @param przeszkodaRamka Struktura przechowująca dane o przeszkodach.
     * @return true jeśli ramka została poprawnie odczytana, false w przeciwnym razie.
     */
        bool czytaj_ramke(const std::string& ramka, PredkoscRamka& predkoscRamka,PolozenieRamka& polozenieRamka,PrzeszkodaRamka& przeszkodaRamka);
private slots:
    void on_przod_pressed();
    void on_prawo_pressed();
    void on_lewo_pressed();
    void on_tyl_pressed();
    void on_polacz_clicked();
    void readData(QByteArray data);
    void loadPorts();
    void sendSterowanieRamka(int P1, int PWM1, int P2, int PWM2);
    void sendKordynatyRamka(int X, int Y);

    void on_predkoscslider_valueChanged(int value);

    void on_stop_clicked();

    void on_polski_clicked();

    void on_angielski_clicked();

    void on_przod_released();

    void on_prawo_released();

    void on_tyl_released();

    void on_lewo_released();
    void updateDistance(int distance);
    void updateSpeedIndicators(int leftWheelSpeed, int rightWheelSpeed);
    void on_wyslij_clicked();
    void updateConnectionState();
    void updateRobotImage();
    void updateArrowImage();
    void updateStopImage();
private:
    QString filePath="C:/studia/WDS/Interfejsrobota/wds/resources/";
    Ui::MainWindow *ui; /**< Wskaźnik na interfejs użytkownika. */
    SerialPort _port;  /**< Obiekt portu szeregowego. */
    int sliderValue; /**< Wartość suwaka prędkości. */
    QChart *chart; /**< Wykres prędkości. */
    QLineSeries *series; /**< Seria danych dla wykresu. */
    QValueAxis *axisX; /**< Oś X wykresu. */
    QValueAxis *axisY; /**< Oś Y wykresu. */
    int podstCzas; /**< Podstawowy czas wykresu. */
    void createChartPredkosc();
    void chart_add_Predkosc(float predkosc);
    QTranslator translator; /**< Obiekt tłumacza. */
    void retranslateUi(); // Funkcja do ponownego ustawienia tekstów widżetów
     QLabel* przeszkoda;
    enum ConnectionState {
        Disconnected,
        Connecting,
        Connected
    };
    ConnectionState currentState = Disconnected;
};
#endif // MAINWINDOW_H

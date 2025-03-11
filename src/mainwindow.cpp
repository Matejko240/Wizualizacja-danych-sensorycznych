
#include "../inc/mainwindow.h"
#include "ui_mainwindow.h"
#include <QListWidget>
#include <sstream>
#include <QPixmap>
/**
 * @brief Konstruktor klasy MainWindow.
 *
 * Inicjalizuje interfejs użytkownika oraz przygotowuje serie danych dla wykresu.
 *
 * @param parent Wskaźnik na rodzica widgetu.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , chart(new QChart())
    , series(new QLineSeries())
    , axisX(new QValueAxis())
    , axisY(new QValueAxis())
    , podstCzas(0)
{
    // Inicjalizacja interfejsu użytkownika
    ui->setupUi(this);
    setWindowTitle(tr("Interfejs robota"));
    // Connect signals and slots
    loadPorts();
    connect(&_port, &SerialPort::dataReceived, this, &MainWindow::readData);
    createChartPredkosc();
    QIntValidator *validator = new QIntValidator(-1000, 10000, this); // Przykład: zakres od 0 do 10000
    ui->wyslijx->setValidator(validator);
    ui->wysliy->setValidator(validator);
    przeszkoda = ui->przeszkoda;
    updateConnectionState();
    //ui->robot->setPixmap(QPixmap("C:/studia/WDS/Interfejsrobota/wds/resources/platforma.png"));
    updateRobotImage();
    updateArrowImage();
    updateStopImage();
}

/**
 * @brief Destruktor klasy MainWindow.
 *
 * Czyści pamięć po interfejsie użytkownika.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Dodaje prędkość do wykresu.
 *
 * @param predkosc Wartość prędkości do dodania.
 */
void MainWindow::chart_add_Predkosc(float predkosc)
{
    chart->axes(Qt::Horizontal).first()->setRange(podstCzas-20,podstCzas + 20);
    series->append(podstCzas, predkosc);
    if (predkosc > axisY->max()) {
        axisY->setMax(predkosc + 0.5); // Dodanie przestrzeni ponad maksymalną wartość prędkości
    }
}
/**
 * @brief Tworzy i konfiguruje wykres prędkości.
 */
void MainWindow::createChartPredkosc()
{
    chart->legend()->hide();
    // Konfiguracja wykresu
    chart->addSeries(series);
    chart->setTitle(tr("Wykres Predkosc(cm/s) w czasie"));
    chart->createDefaultAxes();
    chart->axes(Qt::Vertical).first()->setRange(0,50);
    chart->axes(Qt::Horizontal).first()->setRange(0,11);
    chart->setVisible(true);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setVisible(true);
   // setCentralWidget(chartView);
    ui->wykresPredkosc->addWidget(chartView);

}
/**
 * @brief Obsługuje kliknięcie przycisku "Połącz".
 */
void MainWindow::on_polacz_clicked()
{
    auto isConnected = _port.connect(ui->comboBoxDevices->currentText());
    if (!isConnected)
    {
        QMessageBox::critical(this,tr("Błąd"),tr("Nie podlaczono portu"));
        currentState = Disconnected;
    } else {
        QString message = QString(tr("Podłączono do portu %1")).arg(ui->comboBoxDevices->currentText());
        QMessageBox::information(this, tr("Sukces!"), message);
        currentState = Connected;
    }
     updateConnectionState();
}
/**
 * @brief Ładuje dostępne porty szeregowe do listy rozwijanej.
 */
void MainWindow::loadPorts()
{
    foreach (auto &port, QSerialPortInfo::availablePorts())
    {
        ui->comboBoxDevices->addItem(port.portName());
    }
}

/**
 * @brief Odczytuje dane z portu szeregowego.
 *
 * @param data Odczytane dane.
 */
 void MainWindow::readData(QByteArray data)
{
    std::string tempString = data.toStdString();
    PredkoscRamka predkoscRamka;
    PolozenieRamka polozenieRamka;
    PrzeszkodaRamka przeszkodaRamka;
    if(czytaj_ramke(tempString, predkoscRamka,polozenieRamka, przeszkodaRamka))
    {
   // ui->Messages->addItem(QString::fromStdString(tempString));
        // Przetwarzanie danych o położeniu
       // ui->Messages->addItem(QString("Polozenie: X=%1 Y=%2 Z=%3")
                                //  .arg(polozenieRamka.X)
                                //  .arg(polozenieRamka.Y)
                               //   .arg(polozenieRamka.Z));
        ui->X->setText(QString::number(polozenieRamka.X));
        ui->Y->setText(QString::number(polozenieRamka.Y));

       // ui->Messages->addItem(QString("Predkosc: RPM1=%1 RPM2=%2")
                               //   .arg( predkoscRamka.RPM1)
                                //  .arg(predkoscRamka.RPM2));
       // ui->Messages->addItem(QString("Przeszkoda:%1 cm")
                                //  .arg( przeszkodaRamka.odleglosc));
        updateDistance(przeszkodaRamka.odleglosc);
        float predkosc = abs(((predkoscRamka.RPM2+predkoscRamka.RPM1)/2*3.131592*0.074)/60*100); // cm/s
        chart_add_Predkosc(predkosc); // Dodanie danych do wykresu
        podstCzas++;
        if(przeszkodaRamka.odleglosc<10){
            ui->przeszkoda->setEnabled(true);
        } else {
             ui->przeszkoda->setEnabled(false);
        }
        ui->V->setText(QString::number(predkosc));
        ui->RPML->setText(QString::number(predkoscRamka.RPM1));
        ui->RPMP->setText(QString::number(predkoscRamka.RPM2));
        updateSpeedIndicators(abs(predkoscRamka.RPM1),abs(predkoscRamka.RPM2));
    }
}
/**
 * @brief Odczytuje ramkę danych.
 *
 * @param ramka Tekst ramki.
 * @param predkoscRamka Struktura przechowująca dane prędkości.
 * @param polozenieRamka Struktura przechowująca dane położenia.
 * @param przeszkodaRamka Struktura przechowująca dane o przeszkodach.
 * @return true jeśli ramka została poprawnie odczytana, false w przeciwnym razie.
 */
bool MainWindow::czytaj_ramke(const std::string& ramka, PredkoscRamka& predkoscRamka,PolozenieRamka& polozenieRamka,PrzeszkodaRamka& przeszkodaRamka) {

    std::istringstream iss(ramka);

    // Read and validate terminator '['
    if (!(iss.get()=='[')) {
     //std::cerr << "Error reading is not '['\n";
       // ui->Messages->addItem(QString(tr("Blad")));
   return false;
   }
    // Skip whitespace
    iss.ignore();

    // Read and validate identifier 'K'
    if (!(iss.get(polozenieRamka.identyfikator)) || !(polozenieRamka.identyfikator == 'K')) {
        //std::cerr << "Error reading or identifier is not 'K'\n";
        //ui->Messages->addItem(QString(tr("Blad")));
        return false;
    }

    // Skip whitespace
    iss.ignore();

    // Read int value
    if (!(iss >> polozenieRamka.X)) {
        //std::cerr << "Error reading float value\n";
        return false;
    }

    // Skip whitespace
    iss.ignore();

    // Read float value
    if (!(iss >> polozenieRamka.Y)) {
        //std::cerr << "Error reading float value\n";
        return false;
    }

    // Skip whitespace
    iss.ignore();

    // Read int value
    if (!(iss >> polozenieRamka.Z)) {
        //std::cerr << "Error reading float value\n";
        return false;
    }


    // Skip whitespace
    iss.ignore();


    // Read and validate identifier 'V'
    if (!(iss.get(predkoscRamka.identyfikator)) || !(predkoscRamka.identyfikator == 'V')) {
        //std::cerr << "Error reading or identifier is not 'V'\n";
        return false;
    }

    // Skip whitespace
    iss.ignore();


    // Read float value
    if (!(iss >> predkoscRamka.RPM1)) {
        //std::cerr << "Error reading float value\n";
        return false;
    }

    // Skip whitespace
    iss.ignore();

    // Read float value
    if (!(iss >> predkoscRamka.RPM2)) {
        //std::cerr << "Error reading float value\n";
        return false;
    }

    // Skip whitespace
    iss.ignore();

    // Read and validate identifier 'P'
    if (!(iss.get(predkoscRamka.identyfikator)) || !(predkoscRamka.identyfikator == 'P')) {
        //std::cerr << "Error reading or identifier is not 'V'\n";
        return false;
    }

    // Skip whitespace
    iss.ignore();


    // Read float value
    if (!(iss >>  przeszkodaRamka.odleglosc)) {
        //std::cerr << "Error reading float value\n";
        return false;
    }

    // Skip whitespace
    iss.ignore();

    if (!(iss.get()==']')) {
        //std::cerr << "Error reading is not '['\n";
        //ui->Messages->addItem(QString(tr("Blad")));
        return false;
    }
    //Skip whitespace
    //iss.ignore();

    return true;
}


/**
 * @brief Obsługuje naciśnięcie przycisku "Przód".
 */
void MainWindow::on_przod_pressed()
{
    sendSterowanieRamka(1, sliderValue, 1, sliderValue);
}
/**
 * @brief Obsługuje kliknięcie przycisku "Prawo".
 */
void MainWindow::on_prawo_pressed()
{
    sendSterowanieRamka(1, sliderValue*0.5, -1, sliderValue*0.5);
}
/*
    size_t checksum_pos = frame.rfind(' ');
    if (checksum_pos != std::string::npos && checksum_pos < frame.size() - 2) {
        std::string data_part = frame.substr(0, checksum_pos);
        std::string received_checksum_str = frame.substr(checksum_pos + 1);
        char received_checksum = (char)strtol(received_checksum_str.c_str(), nullptr, 16);

        char calculated_checksum = 0;
        for (auto c : data_part) {
            calculated_checksum ^= c;
        }
*/
/**
 * @brief Obsługuje kliknięcie przycisku "Lewo".
 */
void MainWindow::on_lewo_pressed()
{
    sendSterowanieRamka(-1, sliderValue*0.5, 1, sliderValue*0.5);
}
/**
 * @brief Obsługuje kliknięcie przycisku "Tył".
 */
void MainWindow::on_tyl_pressed()
{
    sendSterowanieRamka(-1, sliderValue, -1, sliderValue);
}
/**
 * @brief Wysyła ramkę sterowania do portu szeregowego.
 *
 * @param P1 Parametr sterowania w którą stronę ma się kręcić lewe koło.
 * @param PWM1 Parametr PWM 1.
 * @param P2 Parametr sterowania w którą stronę ma się kręcić prawe koło.
 * @param PWM2 Parametr PWM 2.
 */
void MainWindow::sendSterowanieRamka(int P1, int PWM1, int P2, int PWM2)
{
    QString ramka = QString("S %1 %2 %3 %4\n").arg(P1).arg(PWM1).arg(P2).arg(PWM2);
    //QMessageBox::information(this, "Sukces!", ramka);
    _port.sendData(ramka.toUtf8());
}
/**
 * @brief Wysyła ramkę koordynat do portu szeregowego.
 *
 * @param X Współrzędna X.
 * @param Y Współrzędna Y.
 */
void MainWindow::sendKordynatyRamka(int X, int Y)
{
    QString ramka = QString("C %1 %2\n").arg(X).arg(Y);
    // QMessageBox::information(this, "Sukces!", ramka);
    _port.sendData(ramka.toUtf8());
}

/**
 * @brief Obsługuje zmianę wartości suwaka prędkości.
 *
 * @param value Nowa wartość suwaka.
 */
void MainWindow::on_predkoscslider_valueChanged(int value)
{
    sliderValue = value;
}

/**
 * @brief Obsługuje kliknięcie przycisku "Stop".
 */
void MainWindow::on_stop_clicked()
{
     sendSterowanieRamka(0, 0, 0, 0);
}

/**
 * @brief Obsługuje kliknięcie przycisku zmiany języka na polski.
 */
void MainWindow::on_polski_clicked()
{
    qApp->removeTranslator(&translator); // Usuń obecny tłumacz
    retranslateUi(); // Ponownie ustaw teksty widżetów
    QMessageBox::information(this, tr("Informacja"), tr("To jest język Polski"), QMessageBox::Yes);
}
/**
 * @brief Aktualizuje teksty widżetów w interfejsie użytkownika.
 */
void MainWindow::retranslateUi()
{
    ui->retranslateUi(this); // Użyj automatycznie wygenerowanej funkcji retranslateUi
    chart->setTitle(tr("Wykres Predkosc(cm/s) w czasie"));
    // Dodaj inne elementy, które muszą być zaktualizowane, jeśli to konieczne
}

/**
 * @brief Obsługuje kliknięcie przycisku zmiany języka na angielski.
 */
void MainWindow::on_angielski_clicked()
{
    if (translator.load(":/i18n/Interfejsrobota_en_150.ts")) { // Załaduj plik tłumaczeń dla języka angielskiego
        qApp->installTranslator(&translator); // Zainstaluj tłumacza w aplikacji
        retranslateUi(); // Ponownie ustaw teksty widżetów
        QMessageBox::information(this, tr("Notice"), tr("This language is English"), QMessageBox::Yes);
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not load translation file"), QMessageBox::Ok);
    }

}


void MainWindow::on_przod_released()
{
       sendSterowanieRamka(0, 0, 0, 0);
}


void MainWindow::on_prawo_released()
{
         sendSterowanieRamka(0, 0, 0, 0);
}


void MainWindow::on_tyl_released()
{
         sendSterowanieRamka(0, 0, 0, 0);
}


void MainWindow::on_lewo_released()
{
         sendSterowanieRamka(0, 0, 0, 0);
}





void MainWindow::on_wyslij_clicked()
{
    QString tekstx = ui->wyslijx->text();
    int x = tekstx.toInt(); // Konwersja do int
    QString teksty = ui->wysliy->text();
    int y = teksty.toInt(); // Konwersja do int
    sendKordynatyRamka(x,y);
}
void MainWindow::updateDistance(int distance)
{
    przeszkoda->setText(QString(tr("Przeszkoda za %1 cm")).arg(distance));
}
void MainWindow::updateSpeedIndicators(int leftWheelSpeed, int rightWheelSpeed)
{
    ui->leweKoloProgressBar->setValue(leftWheelSpeed);
    ui->praweKoloProgressBar->setValue(rightWheelSpeed);
}
void MainWindow::updateConnectionState()
{

    switch (currentState)
    {
    case Disconnected:
        // Ustaw diodę na czerwoną (brak połączenia)
        ui->kropa->setPixmap(QPixmap(filePath+"kropazla.png"));
        break;

    case Connecting:
        // Ustaw diodę na migającą zieloną (łączenie)
        // Tutaj możesz użyć animacji lub zmiany tła QLabel'a
        ui->kropa->setStyleSheet("background-color: green; border: 2px solid black");
        break;

    case Connected:
        // Ustaw diodę na zieloną bez migania (połączono)
      ui->kropa->setPixmap(QPixmap(filePath+"kropa.svg"));
              // ui->kropa->setPixmap(QPixmap(filePath));
        break;
    }
}
void MainWindow::updateRobotImage()
{
    // Ścieżka do obrazka PNG
    QString robot = filePath+"platforma.png";

    // Załaduj obrazek
    QPixmap pixmap(robot);


    // Przeskaluj obrazek do rozmiaru QLabel
    QPixmap scaledPixmap = pixmap.scaled(1000, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Ustaw przeskalowany obrazek w QLabel
    ui->robot->setPixmap(scaledPixmap);
}
void MainWindow::updateArrowImage(){
    QPixmap pixmap(filePath+"prawo");
    QPixmap scaledPixmap = pixmap.scaled(100, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QIcon ButtonIcon(scaledPixmap);
    ui->prawo->setIcon(ButtonIcon);
    ui->prawo->setIconSize(scaledPixmap.rect().size());

    QTransform transform;
    transform.rotate(90);

    // Apply the rotation to the original pixmap
    QPixmap rotatedPixmap = scaledPixmap.transformed(transform, Qt::SmoothTransformation);
    // Set the icon for the button
    QIcon buttonIcon(rotatedPixmap);
    ui->tyl->setIcon(buttonIcon);
    ui->tyl->setIconSize(rotatedPixmap.rect().size());
    transform.rotate(90);
    rotatedPixmap = scaledPixmap.transformed(transform, Qt::SmoothTransformation);
    QIcon lewo(rotatedPixmap);
    ui->lewo->setIcon(lewo);
    ui->lewo->setIconSize(rotatedPixmap.rect().size());
   transform.rotate(90);
    rotatedPixmap = scaledPixmap.transformed(transform, Qt::SmoothTransformation);
    QIcon przod(rotatedPixmap);
    ui->przod->setIcon(przod);
    ui->przod->setIconSize(rotatedPixmap.rect().size());
}
void MainWindow::updateStopImage(){
    QPixmap pixmap(filePath+"stop");
    QPixmap scaledPixmap = pixmap.scaled(100,90, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QIcon ButtonIcon(scaledPixmap);
    ui->stop->setIcon(ButtonIcon);
    ui->stop->setIconSize(scaledPixmap.rect().size());
}

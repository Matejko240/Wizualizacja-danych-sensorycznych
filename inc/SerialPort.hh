#ifndef SERIALPORT_HH
#define SERIALPORT_HH

#include <QObject>
#include <QSerialPort>
/**
 * @brief Klasa SerialPort zarządza połączeniem szeregowym.
 */
class SerialPort : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy SerialPort.
     *
     * @param parent Wskaźnik na rodzica obiektu.
     */
    explicit SerialPort(QObject *parent = nullptr);
    /**
     * @brief Łączy się z portem szeregowym.
     *
     * @param portName Nazwa portu szeregowego.
     * @return true jeśli połączenie się powiodło, false w przeciwnym razie.
     */
    bool connect(QString portName);
    /**
     * @brief Destruktor klasy SerialPort.
     */
    ~SerialPort();

public slots:
    /**
     * @brief Slot obsługujący odbieranie danych z portu szeregowego.
     *
     * @param data Odebrane dane.
     */
    void onDataReceived(const QByteArray &data) {
        // Dodaj otrzymane dane do bufora
        receivedDataBuffer = data;

        // Sprawdź, czy bufor zawiera pełną wiadomość (np. zakończoną znakiem nowej linii)
        if (receivedDataBuffer.contains('\n')) {
            // Podziel bufor na linie
            QStringList lines = QString::fromUtf8(receivedDataBuffer).split('\n');

            // Usuń puste części
            lines.removeAll("");

            // Przetwórz każdą linię (można też przetwarzać tylko ostatnią, jeśli wiadomość jest jednoliniowa)
            foreach (const QString &line, lines) {
                emit dataReceived(line.trimmed().toUtf8()); // Wyślij przetworzoną linię do interfejsu użytkownika jako QByteArray
            }

            // Wyczyść bufor po przetworzeniu wiadomości
            receivedDataBuffer.clear();
        }
    }
    /**
     * @brief Wysyła dane przez port szeregowy.
     *
     * @param data Dane do wysłania.
     */
    void sendData(const QByteArray &data);

signals:
    /**
     * @brief Sygnał emitowany po odebraniu danych z portu szeregowego.
     *
     * @param b Odebrane dane.
     */
    void dataReceived(QByteArray b);


private slots:
    /**
     * @brief Slot wywoływany, gdy port szeregowy jest gotowy do odczytu danych.
     */
    void dataReady();

private:
    QSerialPort *_serialPort; /**< Wskaźnik na obiekt portu szeregowego. */
    QByteArray receivedDataBuffer; /**< Bufor na odebrane dane. */
};

#endif // SERIALPORT_HH

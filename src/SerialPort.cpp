#include "inc/SerialPort.hh"

/**
 * @brief Konstruktor klasy SerialPort.
 *
 * Inicjalizuje obiekt portu szeregowego.
 *
 * @param parent Wskaźnik na rodzica obiektu.
 */
SerialPort::SerialPort(QObject *parent)
    : QObject{parent}
    , _serialPort(nullptr)
{}
/**
 * @brief Destruktor klasy SerialPort.
 *
 * Zamyka i usuwa obiekt portu szeregowego, jeśli istnieje.
 */
SerialPort::~SerialPort()
{
    if(_serialPort != nullptr)
    {
        _serialPort->close();
        delete _serialPort;
    }
}
/**
 * @brief Łączy z portem szeregowym.
 *
 * Konfiguruje i otwiera port szeregowy o podanej nazwie.
 *
 * @param portName Nazwa portu szeregowego.
 * @return true jeśli połączenie zostało nawiązane, false w przeciwnym razie.
 */
bool SerialPort::connect(QString portName)
{
    if (_serialPort != nullptr)
    {
        _serialPort->close();
        delete _serialPort;
    }
    _serialPort = new QSerialPort(this);
    _serialPort->setPortName(portName);
    _serialPort->setBaudRate(QSerialPort::Baud115200);
    _serialPort->setDataBits(QSerialPort::Data8);
    _serialPort->setParity(QSerialPort::NoParity);
    _serialPort->setStopBits(QSerialPort::OneStop);

    if (_serialPort->open(QIODevice::ReadWrite))
    {
        QObject::connect(_serialPort, &QSerialPort::readyRead, this, &SerialPort::dataReady);
    }
    return _serialPort->isOpen();
}
/**
 * @brief Slot wywoływany, gdy są dostępne dane do odczytu.
 *
 * Emituje sygnał dataReceived z odczytanymi danymi.
 */
void SerialPort::dataReady()
{
    if (_serialPort->isOpen())
    {
        emit dataReceived(_serialPort->readLine());
    }
}
/**
 * @brief Wysyła dane przez port szeregowy.
 *
 * @param data Dane do wysłania.
 */
void SerialPort::sendData(const QByteArray &data)
{
    if (_serialPort && _serialPort->isOpen())
    {
        _serialPort->write(data);
    }
}

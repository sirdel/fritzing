#include "platformarduino.h"

PlatformArduino::PlatformArduino() : Platform(QString("Arduino"))
{
    setReferenceUrl(QUrl(QString("http://arduino.cc/en/Reference/")));
    setDownloadUrl(QUrl("http://arduino.cc/en/Main/Software"));
    setMinVersion("1.5.2");
    setCanProgram(true);
    setExtensions(QStringList() << ".ino" << ".pde");

    QMap<QString, QString> boards;
    // https://github.com/arduino/Arduino/blob/ide-1.5.x/hardware/arduino/avr/boards.txt
    boards.insert("Arduino UNO", "arduino:avr:uno");
    boards.insert("Arduino Yún", "arduino:avr:yun");
    boards.insert("Arduino Mega/2560", "arduino:avr:mega");
    boards.insert("Arduino Duemilanove/Diecemila", "arduino:avr:diecimila");
    boards.insert("Arduino Nano", "arduino:avr:nano");
    boards.insert("Arduino Mega ADK", "arduino:avr:megaADK");
    boards.insert("Arduino Leonardo", "arduino:avr:leonardo");
    boards.insert("Arduino Micro", "arduino:avr:micro");
    boards.insert("Arduino Esplora", "arduino:avr:Esplora");
    boards.insert("Arduino Mini", "arduino:avr:mini");
    boards.insert("Arduino Ethernet", "arduino:avr:ethernet");
    boards.insert("Arduino Fio", "arduino:avr:fio");
    boards.insert("Arduino BT", "arduino:avr:bt");
    boards.insert("Lilypad Arduino USB", "arduino:avr:LilyPadUSB");
    boards.insert("LilyPad Arduino ", "arduino:avr:lilypad");
    boards.insert("Arduino Pro/Pro Mini", "arduino:avr:pro");
    boards.insert("Arduino NG or older", "arduino:avr:atmegang");
    boards.insert("Arduino Robot Control", "arduino:avr:robotControl");
    boards.insert("Arduino Robot Motor", "arduino:avr:robotMotor");
    // https://github.com/arduino/Arduino/blob/ide-1.5.x/hardware/arduino/sam/boards.txt
    boards.insert("Arduino Due (Programming Port)", "arduino:sam:arduino_due_x_dbg");
    boards.insert("Arduino Due (Native USB Port)", "arduino:sam:arduino_due_x");
    setBoards(boards);
}

void PlatformArduino::upload(QString port, QString board, QString fileLocation)
{

}
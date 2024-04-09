#ifndef QSIMPLE_INI_H
#define QSIMPLE_INI_H
#include <QFile>
#include <QTextStream>
#include <QVariantMap>
namespace cs {
class QSimpleIni : public QVariantMap {
  QString MULTILINE_END = QStringLiteral("%T");
  QString MULTILINE_START = QStringLiteral("T%");

private:
  bool isSection(const QString &line) const {
    if (!line.isEmpty()) {
      if (line.at(0) != '[') {
        return false;
      }
      if (line.at(line.size() - 1) != ']') {
        return false;
      }
      if (line.midRef(1, line.size() - 2).contains('[') ||
          line.midRef(1, line.size() - 2).contains(']')) {
        return false;
      }

      return true;
    } else {
      return false;
    }
  }
  const QString prepareSection(const QString &line) {
    QString name = line;
    name.remove(name.size() - 1, 1).remove(0, 1);
    return name;
  }
  void processSectionValues(QTextStream &stream, const QString &section_name) {
    QString line;
    QVariantMap section = this->take(section_name).toMap();
    int pos = stream.pos();
    while (stream.readLineInto(&line)) {
      if (line.isEmpty() || line.startsWith(';')) {
        continue;
      }
      if (isSection(line)) {
        stream.seek(pos);
        this->insert(section_name, section);
        return;
      }
      QPair<QString, QString> variable;
      variable.first = line.split(QStringLiteral("=")).first().trimmed();
      variable.second = line.split(QStringLiteral("=")).last().trimmed();
      if (variable.second.startsWith(MULTILINE_START)) {
        int var_pos = stream.pos();
        variable.second.remove(MULTILINE_START);
        while (stream.readLineInto(&line)) {
          if (isSection(line)) {
            stream.seek(var_pos);
            return;
          }
          if (line == MULTILINE_END) {
            break;
          }
          variable.second += line + QStringLiteral("\n");
        }
      }
      section.insert(variable.first, variable.second);
      pos = stream.pos();
    }
    this->insert(section_name, section);
  }
  void processData(const QByteArray &data) {
    if (data.isEmpty()) {
      return;
    }
    QTextStream stream(data);
    QString line;
    while (stream.readLineInto(&line)) {
      if (line == MULTILINE_END) {
        continue;
      }
      if (line.isEmpty() || line.startsWith(';')) {
        continue;
      }
      if (isSection(line)) {
        line = prepareSection(line);
        processSectionValues(stream, line);
      }
    }
    stream.flush();
  }

public:
  explicit QSimpleIni(const QByteArray &data) { processData(data); }
  explicit QSimpleIni(const QString &path) {
    QFile file(path);
    if (file.open(QIODevice::ExistingOnly | QIODevice::ReadOnly)) {
      QByteArray data = file.readAll();
      processData(data);
      file.close();
    }
  }
  ~QSimpleIni() {}
};
#endif
}

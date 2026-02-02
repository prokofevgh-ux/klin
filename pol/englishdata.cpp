#include "englishdata.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

EnglishData::EnglishData() {
    for (int i = 0; i < LESSON_COUNT; i++) {
        lessons.append(QList<EnglishWord>());
    }
    load();
}

QString EnglishData::levelName(int levelIndex) {
    switch (levelIndex) {
        case 0: return "A1";
        case 1: return "A2";
        case 2: return "B1";
        case 3: return "B2";
        case 4: return "C1";
        default: return "A1";
    }
}

QString EnglishData::lessonId(int index) const {
    if (index < 0 || index >= LESSON_COUNT) return QString();
    int lev = index / LESSONS_PER_LEVEL;
    int num = (index % LESSONS_PER_LEVEL) + 1;
    return levelName(lev) + "." + QString::number(num);
}

int EnglishData::levelIndexFromLesson(int lessonIndex) const {
    if (lessonIndex < 0 || lessonIndex >= LESSON_COUNT) return 0;
    return lessonIndex / LESSONS_PER_LEVEL;
}

int EnglishData::lessonNumInLevel(int lessonIndex) const {
    if (lessonIndex < 0 || lessonIndex >= LESSON_COUNT) return 1;
    return (lessonIndex % LESSONS_PER_LEVEL) + 1;
}

QList<EnglishWord> EnglishData::getWords(int lessonIndex) const {
    if (lessonIndex < 0 || lessonIndex >= lessons.size()) return QList<EnglishWord>();
    return lessons.at(lessonIndex);
}

void EnglishData::setWords(int lessonIndex, const QList<EnglishWord>& words) {
    if (lessonIndex < 0 || lessonIndex >= lessons.size()) return;
    lessons[lessonIndex] = words;
    save();
}

void EnglishData::addWord(int lessonIndex, const QString& word, const QString& translation) {
    if (lessonIndex < 0 || lessonIndex >= lessons.size()) return;
    EnglishWord w;
    w.word = word.trimmed();
    w.translation = translation.trimmed();
    if (!w.word.isEmpty()) {
        lessons[lessonIndex].append(w);
        save();
    }
}

void EnglishData::removeWord(int lessonIndex, int wordIndex) {
    if (lessonIndex < 0 || lessonIndex >= lessons.size()) return;
    QList<EnglishWord>& list = lessons[lessonIndex];
    if (wordIndex >= 0 && wordIndex < list.size()) {
        list.removeAt(wordIndex);
        save();
    }
}

void EnglishData::load() {
    QString path = dataPath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    file.close();
    for (int i = 0; i < LESSON_COUNT && i < lessons.size(); i++) {
        QString key = "A1.1"; // fallback
        int lev = i / LESSONS_PER_LEVEL;
        int num = (i % LESSONS_PER_LEVEL) + 1;
        key = levelName(lev) + "." + QString::number(num);
        QJsonArray arr = root[key].toArray();
        QList<EnglishWord> list;
        for (const QJsonValue& v : arr) {
            QJsonObject o = v.toObject();
            EnglishWord w;
            w.word = o["word"].toString();
            w.translation = o["translation"].toString();
            list.append(w);
        }
        lessons[i] = list;
    }
}

void EnglishData::save() {
    QString path = dataPath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QJsonObject root;
    for (int i = 0; i < lessons.size(); i++) {
        QString key = "A1.1";
        int lev = i / LESSONS_PER_LEVEL;
        int num = (i % LESSONS_PER_LEVEL) + 1;
        key = levelName(lev) + "." + QString::number(num);
        QJsonArray arr;
        for (const EnglishWord& w : lessons.at(i)) {
            QJsonObject o;
            o["word"] = w.word;
            o["translation"] = w.translation;
            arr.append(o);
        }
        root[key] = arr;
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return;
    file.write(QJsonDocument(root).toJson());
    file.close();
}

QString EnglishData::dataPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/english_vocabulary.json";
}

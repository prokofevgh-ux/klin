#ifndef ENGLISHDATA_H
#define ENGLISHDATA_H

#include <QString>
#include <QList>
#include <QPair>
#include <QJsonObject>

struct EnglishWord {
    QString word;
    QString translation;
};

class EnglishData {
public:
    EnglishData();

    static const int LEVEL_COUNT = 5;   // A1, A2, B1, B2, C1
    static const int LESSONS_PER_LEVEL = 50;
    static const int LESSON_COUNT = LEVEL_COUNT * LESSONS_PER_LEVEL;  // 250

    static QString levelName(int levelIndex);   // "A1", "A2", "B1", "B2", "C1"
    QString lessonId(int index) const;         // "A1.1", "A1.2", ... "C1.50"
    int levelIndexFromLesson(int lessonIndex) const;
    int lessonNumInLevel(int lessonIndex) const;  // 1..50

    QList<EnglishWord> getWords(int lessonIndex) const;
    void setWords(int lessonIndex, const QList<EnglishWord>& words);
    void addWord(int lessonIndex, const QString& word, const QString& translation);
    void removeWord(int lessonIndex, int wordIndex);

    void load();
    void save();

private:
    QList<QList<EnglishWord>> lessons;
    QString dataPath() const;
};

#endif // ENGLISHDATA_H

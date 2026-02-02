#ifndef GAMESTATS_H
#define GAMESTATS_H

#include <QString>
#include <QDate>
#include <QJsonObject>

class GameStats {
public:
    GameStats();

    int getXP() const { return xp; }
    int getLevel() const { return level; }
    int getStreak() const { return streak; }
    int getXPForNextLevel() const;  // XP нужно до следующего уровня

    void addTaskCompleted();       // Выполнена задача сегодня — +XP, обновить streak
    void load();
    void save();

private:
    int xp;
    int level;
    int streak;
    QDate lastCompletedDate;

    void recalcLevel();             // пересчитать level по xp
    QString dataPath() const;
};

#endif // GAMESTATS_H

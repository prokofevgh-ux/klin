#include "gamestats.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>

GameStats::GameStats() : xp(0), level(1), streak(0) {
    load();
}

int GameStats::getXPForNextLevel() const {
    return level * 10;
}

void GameStats::addTaskCompleted() {
    QDate today = QDate::currentDate();
    if (lastCompletedDate == today) {
        return; // уже считали сегодня
    }
    if (lastCompletedDate.isValid() && lastCompletedDate.daysTo(today) == 1) {
        streak++;
    } else if (!lastCompletedDate.isValid() || lastCompletedDate.daysTo(today) > 1) {
        streak = 1;
    }
    lastCompletedDate = today;
    xp += 10; // +10 XP за задачу
    recalcLevel();
    save();
}

void GameStats::recalcLevel() {
    int need = 0;
    int lvl = 1;
    while (xp >= need + lvl * 10) {
        need += lvl * 10;
        lvl++;
    }
    level = lvl;
}

void GameStats::load() {
    QString path = dataPath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    QJsonObject obj = QJsonDocument::fromJson(file.readAll()).object();
    file.close();
    xp = obj["xp"].toInt(0);
    level = obj["level"].toInt(1);
    streak = obj["streak"].toInt(0);
    lastCompletedDate = QDate::fromString(obj["lastCompletedDate"].toString(), Qt::ISODate);
    recalcLevel();
}

void GameStats::save() {
    QString path = dataPath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return;
    QJsonObject obj;
    obj["xp"] = xp;
    obj["level"] = level;
    obj["streak"] = streak;
    obj["lastCompletedDate"] = lastCompletedDate.toString(Qt::ISODate);
    file.write(QJsonDocument(obj).toJson());
    file.close();
}

QString GameStats::dataPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/gamestats.json";
}

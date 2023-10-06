#ifndef COS_H
#define COS_H

#include <QDebug>
#include <QMap>
#include <QString>
#include <QList>
#include <QPair>
#include <QFileInfo>
#include <iostream>
#include <QString>
#include <QVector>
#include <algorithm>
#include <unordered_map>

QMap<QString, QList<int> > get_dict(const QString& a, const QString& b);
double cos_ab(const QString& a, const QString& b);

double guess(const QHash<QChar, unsigned char> &str1, const QHash<QChar, unsigned char> &str2, double s1, double s2);
int levenshtein_distance(const QString &s1, const QString &s2);

#endif // COS_H

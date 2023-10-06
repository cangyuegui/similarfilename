#include "cos.h"
#include <math.h>
#include <algorithm>
#include <QMap>
#include <QVector>
#include <QString>
#include <QSet>



QMap<QString, QList<int> > get_dict(const QString &a, const QString &b)
{
    QMap<QString, QList<int> > dict;
    QList<int> empty_list;
    empty_list.append(0);
    empty_list.append(0);

    for (const QString& v : a+b)
    {
        if (!dict.contains(v))
        {
            dict[v] = empty_list;
        }
    }

    for (const QString& v : a)
    {
        if (dict.contains(v))
        {
            dict[v][0] +=1;
        }
    }

    for (const QString& v : b)
    {
        if (dict.contains(v))
        {
            dict[v][1] += 1;
        }
    }
    return dict;
}

double cos_ab(const QString &a, const QString &b)
{
    int ab = 0;
    int a_distance2 = 0;
    int b_distance2 = 0;
    QMap<QString, QList<int> > dict = get_dict(a, b);
    QMapIterator<QString, QList<int> > i(dict);
    while (i.hasNext())
    {
        i.next();
        const QList<int>& v = i.value();
        ab += v[0]*v[1];

        a_distance2 += v[0]*v[0];
        b_distance2 += v[1]*v[1];
    }
    double s_a = std::sqrt(a_distance2);
    double s_b = std::sqrt(b_distance2);

    return ab/(s_a*s_b);
}

int levenshtein_distance(const QString& s1, const QString& s2)
{
    int m = s1.length();
    int n = s2.length();

    QVector<QVector<int>> dp(m + 1, QVector<int>(n + 1));

    for (int i = 0; i <= m; i++) {
        dp[i][0] = i;
    }
    for (int j = 0; j <= n; j++) {
        dp[0][j] = j;
    }

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = std::min(dp[i][j - 1], std::min(dp[i - 1][j], dp[i - 1][j - 1])) + 1;
            }
        }
    }

    return dp[m][n];
}



double guess(const QHash<QChar, unsigned char> &str1, const QHash<QChar, unsigned char> &str2, double s1, double s2)
{
    if (s1 / s2 < 0.3)
    {
        return 0;
    }

    int count = 0;
    for (auto c = str1.begin(); c != str1.end(); ++c)
    {
        auto titr = str2.find(c.key());
        if (titr != str2.end())
        {
            count += std::min(c.value(), titr.value());
        }
    }

    return static_cast<double>(count) / s2;
}

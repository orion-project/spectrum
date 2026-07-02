#include "StringUtils.h"

#include <QSet>

namespace StringUtils
{

// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Dice%27s_coefficient
float diceSimilarity(const QString& s1, const QString& s2)
{
    if (s1 == s2)
        return 1;

    if (s1.length() < 2 || s2.length() < 2)
        return 0;

    QSet<QStringView> bigrams1;
    for (int i = 0; i < s1.length() - 1; i++)
        bigrams1.insert(QStringView(s1).mid(i, 2));

    QSet<QStringView> bigrams2;
    for (int i = 0; i < s2.length() - 1; i++)
        bigrams2.insert(QStringView(s2).mid(i, 2));

    int intersection = 0;
    for (auto it = bigrams2.cbegin(); it != bigrams2.cend(); it++)
        if (bigrams1.contains(*it))
            intersection++;

    // Dice coefficient formula
    return (2 * intersection) / (float)(bigrams1.size() + bigrams2.size());
}

static float diceSimilarity(const QSet<QStringView>& bigrams1, const QString& s2)
{
    if (s2.length() < 2)
        return 0;

    QSet<QStringView> bigrams2;
    for (int i = 0; i < s2.length() - 1; i++)
        bigrams2.insert(QStringView(s2).mid(i, 2));

    int intersection = 0;
    for (auto it = bigrams2.cbegin(); it != bigrams2.cend(); it++)
        if (bigrams1.contains(*it))
            intersection++;

    // Dice coefficient formula
    return (2 * intersection) / (float)(bigrams1.size() + bigrams2.size());
}

// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance
int levenshteinDistance(const QString& s1, const QString& s2)
{
    const int len1 = s1.length();
    const int len2 = s2.length();
    
    if (len1 > len2)
        return levenshteinDistance(s2, s1);
        
    std::vector<int> dist(len1 + 1);
    
    for (int i = 0; i <= len1; i++)
        dist[i] = i;
        
    for (int j = 1; j <= len2; j++)
    {
        int prev_diagonal = dist[0];
        int prev_diagonal_save;
        
        dist[0]++;
        
        for (int i = 1; i <= len1; i++)
        {
            prev_diagonal_save = dist[i];
            
            if (s1.at(i-1) == s2.at(j-1))
                dist[i] = prev_diagonal;
            else
                dist[i] = std::min(std::min(dist[i-1], dist[i]), prev_diagonal) + 1;
                
            prev_diagonal = prev_diagonal_save;
        }
    }
    
    return dist[len1];
}

static QString replaceDigits(const QString &str)
{
    QString s(str);
    
    //s.replace(QRegularExpression("\\d+"), "#");
    
    for (int i = 0; i < s.size(); i++)
        if (s[i].isDigit())
            s[i] = '#';

    return s;
}

int selectSimilarFileName(const QString& fileName, const QStringList& fileNames)
{
    if (fileName.length() < 2)
        return -1;

    QString fn = replaceDigits(fileName);
    QSet<QStringView> bigrams1;
    for (int i = 0; i < fn.length() - 1; i++)
        bigrams1.insert(QStringView(fn).mid(i, 2));

    int maxIndex = -1;
    double maxSimilarity = 0;
    for (int i = 0; i < fileNames.size(); i++)
    {
        auto similarity = diceSimilarity(bigrams1, replaceDigits(fileNames.at(i)));
        if (similarity >= maxSimilarity)
        {
            maxSimilarity = similarity;
            maxIndex = i;
        }
    }
    return maxSimilarity > 0.85 ? maxIndex : -1;
}

}
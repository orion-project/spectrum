#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <QString>

namespace StringUtils
{

// Sørensen–Dice Coefficient (Bigram Matching)
// Returns a similarity score between 0.0 (completely different) and 1.0 (identical)
float diceSimilarity(const QString& s1, const QString& s2);

// The Levenshtein Distance 
// counts the number of edits needed to transform one string into another.
int levenshteinDistance(const QString& s1, const QString& s2);

int selectSimilarFileName(const QString& fileName, const QStringList& fileNames);
}

#endif // STRING_UTILS_H

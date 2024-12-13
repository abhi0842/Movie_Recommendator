#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <vector>

using namespace std;

#ifndef MATRIX_H
#define MATRIX_H
using Matrix = std::vector<std::vector<int>>;
#endif

// Load ratings from CSV file
Matrix loadRatings(const std::string &filePath) {
    Matrix ratings;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return {};
    }

    std::string line;
    while (std::getline(file, line)) {
        std::vector<int> row;
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, ',')) {
            try {
                row.push_back(std::stoi(cell));
            } catch (const std::exception &e) {
                std::cerr << "Error parsing integer from: " << cell << "\n";
                return {};
            }
        }

        if (!ratings.empty() && row.size() != ratings[0].size()) {
            std::cerr << "Error: Inconsistent number of columns in CSV file." << std::endl;
            return {};
        }

        ratings.push_back(row);
    }

    if (ratings.empty()) {
        std::cerr << "Error: Ratings file is empty or invalid." << std::endl;
    }

    return ratings;
}

// Calculate cosine similarity between two users
double calculateSimilarity(const std::vector<int> &user1, const std::vector<int> &user2) {
    if (user1.empty() || user2.empty() || user1.size() != user2.size()) {
        std::cerr << "Error: Invalid user data for similarity calculation." << std::endl;
        return 0.0;
    }

    double dotProduct = 0, magnitude1 = 0, magnitude2 = 0;

    for (size_t i = 0; i < user1.size(); ++i) {
        dotProduct += user1[i] * user2[i];
        magnitude1 += user1[i] * user1[i];
        magnitude2 += user2[i] * user2[i];
    }

    if (magnitude1 == 0 || magnitude2 == 0) return 0.0;
    return dotProduct / (std::sqrt(magnitude1) * std::sqrt(magnitude2));
}

// Predict ratings for a specific user
std::vector<double> predictRatings(const Matrix &ratings, int userId) {
    if (ratings.empty() || userId < 0 || userId >= (int)ratings.size()) {
        std::cerr << "Error: Invalid user ID for prediction." << std::endl;
        return {};
    }

    int numMovies = ratings[0].size();
    int numUsers = ratings.size();

    std::vector<double> predictedRatings(numMovies, 0.0);
    std::vector<double> similarityScores(numUsers, 0.0);

    for (int i = 0; i < numUsers; ++i) {
        if (i != userId) {
            similarityScores[i] = calculateSimilarity(ratings[userId], ratings[i]);
        }
    }

    for (int movie = 0; movie < numMovies; ++movie) {
        double weightedSum = 0, similaritySum = 0;

        if (ratings[userId][movie] == 0) {
            for (int user = 0; user < numUsers; ++user) {
                if (user != userId && ratings[user][movie] > 0) {
                    weightedSum += similarityScores[user] * ratings[user][movie];
                    similaritySum += std::abs(similarityScores[user]);
                }
            }
            predictedRatings[movie] = (similaritySum > 0) ? (weightedSum / similaritySum) : 0.0;
        }
    }
    return predictedRatings;
}

// Recommend movies for a specific user
std::vector<int> recommendMovies(const Matrix &ratings, int userId, int topN) {
    std::vector<double> predictedRatings = predictRatings(ratings, userId);

    if (predictedRatings.empty()) {
        std::cerr << "Error: No predictions available." << std::endl;
        return {};
    }

    std::vector<int> recommendedMovies;
    std::vector<std::pair<int, double>> movieRatings;

    for (size_t i = 0; i < predictedRatings.size(); ++i) {
        if (ratings[userId][i] == 0) {
            movieRatings.emplace_back(i, predictedRatings[i]);
        }
    }

    // Sort movies by predicted rating in descending order
    std::sort(movieRatings.begin(), movieRatings.end(),
              [](const std::pair<int, double> &a, const std::pair<int, double> &b) {
                  return a.second > b.second;
              });

    // Select top N movies
    for (int i = 0; i < std::min(topN, (int)movieRatings.size()); ++i) {
        recommendedMovies.push_back(movieRatings[i].first);
    }

    return recommendedMovies;
}

// Main function
int main() {
    std::string filePath = "ratings.csv";
    Matrix ratings = loadRatings(filePath);

    if (ratings.empty()) {
        std::cerr << "Error: Unable to load ratings from file." << std::endl;
        return 1;
    }

    int userId = 0; // User for whom recommendations are generated
    int topN = 3;   // Number of top recommendations to display

    if (userId < 0 || userId >= (int)ratings.size()) {
        std::cerr << "Error: Invalid user ID." << std::endl;
        return 1;
    }

    std::cout << "Top " << topN << " recommended movies for User " << userId << ":\n";
    std::vector<int> recommendations = recommendMovies(ratings, userId, topN);

    for (int movie : recommendations) {
        std::cout << "Movie " << movie + 1 << "\n";
    }
}

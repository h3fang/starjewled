#include "solver.h"

#include <algorithm>

Solver::Solver(int age_threshold_ms) :
    solution_age_threshold(age_threshold_ms),
    board(N+2*PADDING, vector<int>(N+2*PADDING, -1)) {
}

int &Solver::cell(int i, int j) {
    return board[i + PADDING][j + PADDING];
}

void Solver::calc_solutions() {
    vector<Solution> current;
    const auto now = high_resolution_clock::now();
    for (int m = N-1+PADDING; m >= PADDING; --m ) {
        for (int n = PADDING; n < N+PADDING; ++n) {
            // swap with the right cell
            if (int e = check_right(board, m, n).size(); e > 0) {
                current.push_back({m - PADDING, n - PADDING, m - PADDING, n + 1 - PADDING, e, now});
            }
            // swap with the top cell
            if (int e = check_top(board, m, n).size(); e > 0) {
                current.push_back({m - PADDING, n - PADDING, m - 1 - PADDING, n - PADDING, e, now});
            }
        }
    }

    for (auto& v : current) {
        if (auto it = std::find(solutions.begin(), solutions.end(), v); it != solutions.end()) {
            v.timestamp = (*it).timestamp;
        }
    }
    solutions = current;
}

Solution Solver::get_best_solution() {
    if (solutions.empty()) {
        return {-1};
    }
    std::sort(solutions.begin(), solutions.end(), std::greater<Solution>());
    const auto now = high_resolution_clock::now();
    for (const auto& s : solutions) {
        if (now - s.timestamp >= solution_age_threshold) {
            return s;
        }
    }
    return solutions[0];
}

set<pair<int, int>> Solver::check_right(vector<vector<int>> &b, int m, int n) const {
    set<pair<int, int>> canceled;
    const int c = b[m][n], cr = b[m][n+1];
    if (c == cr || cr == -1) {
        return canceled;
    }

    canceled.emplace(m, n);

    if (c == b[m-1][n+1] && c == b[m-2][n+1]) {
        canceled.emplace(m-1, n+1);
        canceled.emplace(m-2, n+1);
    }
    if (c == b[m+1][n+1] && c == b[m+2][n+1]) {
        canceled.emplace(m+1, n+1);
        canceled.emplace(m+2, n+1);
    }
    if (c == b[m-1][n+1] && c == b[m+1][n+1]) {
        canceled.emplace(m-1, n+1);
        canceled.emplace(m+1, n+1);
    }
    if (c == b[m][n+2] && c == b[m][n+3]) {
        canceled.emplace(m, n+2);
        canceled.emplace(m, n+3);
    }

    if (canceled.size() == 1) {
        canceled.clear();
    }

    canceled.emplace(m, n+1);
    auto previous_size = canceled.size();

    if (cr == b[m-1][n] && cr == b[m-2][n]) {
        canceled.emplace(m-1, n);
        canceled.emplace(m-2, n);
    }
    if (cr == b[m+1][n] && cr == b[m+2][n]) {
        canceled.emplace(m+1, n);
        canceled.emplace(m+2, n);
    }
    if (cr == b[m-1][n] && cr == b[m+1][n]) {
        canceled.emplace(m-1, n);
        canceled.emplace(m+1, n);
    }
    if (cr == b[m][n-1] && cr == b[m][n-2]) {
        canceled.emplace(m, n-1);
        canceled.emplace(m, n-2);
    }

    if (canceled.size() == previous_size) {
        canceled.erase({m, n+1});
    }
    return canceled;
}

set<pair<int, int>> Solver::check_top(vector<vector<int>> &b, int m, int n) const {
    set<pair<int, int>> canceled;
    const int c = b[m][n], ct = b[m-1][n];
    if (c == ct || ct == -1) {
        return canceled;
    }

    canceled.emplace(m, n);

    if (c == b[m-1][n-1] && c == b[m-1][n-2]) {
        canceled.emplace(m, n-1);
        canceled.emplace(m, n-2);
    }
    if (c == b[m-1][n+1] && c == b[m-1][n+2]) {
        canceled.emplace(m-1, n+1);
        canceled.emplace(m-1, n+2);
    }
    if (c == b[m-1][n+1] && c == b[m-1][n-1]) {
        canceled.emplace(m-1, n+1);
        canceled.emplace(m-1, n-1);
    }
    if (c == b[m-2][n] && c == b[m-3][n]) {
        canceled.emplace(m-2, n);
        canceled.emplace(m-3, n);
    }

    if (canceled.size() == 1) {
        canceled.clear();
    }
    canceled.emplace(m-1, n);
    auto previous_size = canceled.size();

    if (ct == b[m][n-1] && ct == b[m][n-2]) {
        canceled.emplace(m, n-1);
        canceled.emplace(m, n-2);
    }
    if (ct == b[m][n+1] && ct == b[m][n+2]) {
        canceled.emplace(m, n+1);
        canceled.emplace(m, n+2);
    }
    if (ct == b[m][n-1] && ct == b[m][n+1]) {
        canceled.emplace(m, n-1);
        canceled.emplace(m, n+1);
    }
    if (ct == b[m+1][n] && ct == b[m+2][n]) {
        canceled.emplace(m+1, n);
        canceled.emplace(m+2, n);
    }

    if (canceled.size() == previous_size) {
        canceled.erase({m-1, n});
    }
    return canceled;
}

int Solver::simulate(const Solution& m) {
    auto b = board;
    std::swap(b[m.i1][m.j1], b[m.i2][m.j2]);
    vector<pair<int, int>> canceled;
    for (int m = N-1+PADDING; m >= PADDING; --m ) {
        for (int n = PADDING; n < N+PADDING; ++n) {
            for (const auto & c : check_right(b, m, n)) {
                canceled.push_back(c);
            }
            for (const auto & c : check_top(b, m, n)) {
                canceled.push_back(c);
            }
        }
    }

    return 0;
}

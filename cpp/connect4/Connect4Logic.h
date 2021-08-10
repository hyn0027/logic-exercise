//Connect4Logic.h
//created by hyn0027

#pragma once

#include "../sdk/BaseLogic.h"

#include <vector>
#include <iostream>
#include <algorithm>
#include <cstring>

class Connect4Logic: public BaseLogic {
private:
    int turn = 0;
    std::vector<int> history;
    int chess[10] = {};
    int graph[10][10];
    std::vector<std::string> replay_json;

    void prepare() override {
        std::string msg = "{\n\t\"type\": 0,\n\t\"height\": 8,\n\t\"width\": 8\n}\n";
        singleSend(0, msg);
        singleSend(1, msg);
        history.push_back(-1);
        memset(graph, -1, sizeof(graph));
        return;
    }

    std::pair<int, std::string> sendMsgToPlayer(int &timeLimit, int &lengthLimit) override {
        // Time and length limits remain default and do not need updating.
        std::string sendContent = "{\n\t\"type\": 1,\n\t\"lastPos\": ";
        sendContent += std::to_string(history[getState() - 1]);
        sendContent += "\n}\n";
        return std::make_pair(turn, sendContent);
    }

    void writeReplayAndGameOver(int winner) {
        //write replay
        //todo

        std::stringstream ss;
        for (const auto &record: replay_json)
            ss << record << std::endl;
        writeTextToReplay(ss.str());

        std::vector<int> score;
        if (winner == -1) 
            score[0] = score[1] = 1;
        else {
            score[winner] = 2;
            score[1 - winner] = 0;
        }
        sendGameOverMessage(score);
    }

    void handleResponse(const std::string &response, ErrorType &errorType, int &errorPlayer) override {
        if (errorType == NONE) {
            int pos1 = response.find("\"type\": ");
            int pos2 = response.find(",");
            int pos3 = response.find("\"pos\": ");
            int pos4 = response.find("\n}");
            int col;
            bool failed_by_response = false;
            if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos || pos4 == std::string::npos)
                failed_by_response = true;
            else if (pos2 - pos1 != 9) 
                failed_by_response = true;
            else if (response[pos2 - 1] != '2')
                failed_by_response = true;
            else if (pos4 - pos3 != 8)
                failed_by_response = true;
            else if (response[pos4 - 1] < '0' || response[pos4 - 1] > '7')
                failed_by_response = true;
            else {
                col = response[pos4 - 1] - '0';
                if (chess[col] >= 8)   
                    failed_by_response = true;
            }
            if (failed_by_response){
                errorType = RE;
                errorPlayer = turn;
            }
            else {
                replay_json.push_back(response);
                graph[col][chess[col]] = turn;
                chess[col]++;
                int winner = -1;
                for (int i = 0; i < 8; i++)
                    for (int j = 0; j < 5; j++)
                        if (graph[i][j] == graph[i][j + 1] && graph[i][j] == graph[i][j + 2] && graph[i][j] == graph[i][j + 3] && graph[i][j] != -1) {
                            winner = graph[i][j];
                            break;
                        }
                if (winner == -1) {
                    for (int j = 0; j < 8; j++)
                        for (int i = 0; i < 5; i++)
                            if (graph[i][j] == graph[i][j + 1] && graph[i][j] == graph[i][j + 2] && graph[i][j] == graph[i][j + 3] && graph[i][j] != -1) {
                                winner = graph[i][j];
                                break;
                            }
                }
                if (winner == -1) {
                    for (int i = 0; i < 5; i++)
                        for (int j = 0; j < 5; j++)
                            if (graph[i][j] == graph[i + 1][j + 1] && graph[i][j] == graph[i + 2][j + 2] && graph[i][j] == graph[i + 3][j + 3] && graph[i][j] != -1) {
                                winner = graph[i][j];
                                break;
                            }
                }
                if (winner == -1) {
                    for (int i = 0; i < 5; i++)
                        for (int j = 3; j < 8; j++)
                            if (graph[i][j] == graph[i + 1][j - 1] && graph[i][j] == graph[i + 2][j - 2] && graph[i][j] == graph[i + 3][j - 3] && graph[i][j] != -1) {
                                winner = graph[i][j];
                                break;
                            }
                }
                if (winner != -1)
                    writeReplayAndGameOver(winner);
            }
        }

        if (errorType != NONE) {
            replay_json.push_back("Player " + std::to_string(errorPlayer) + " error: " + std::to_string(errorType) + " " + response);
            writeReplayAndGameOver(1 - turn);
            return;
        }

        turn = 1 - turn;
        //full
        bool notfull = false;
        for (int i = 0; i < 8; i++)
            if (chess[i] < 8) {
                notfull = true;
                break;
            }
        if (!notfull) {
            replay_json.push_back("chessboard full");
            writeReplayAndGameOver(-1);
        }
    }
};

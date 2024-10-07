#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <locale>
#include <codecvt>
#include "libs/bridge.h"
#include <queue>
using namespace std;

string utf8_encode(const wstring& wstr);

string** map_data;  // 맵 정보. 예) map_data[0][1] - [0, 1]의 지형/지물
map<string, string*> allies;  // 아군 정보. 예) allies.at("A") - 플레이어 본인의 정보
map<string, string*> enemies;  // 적군 정보. 예) enemies.at("X") - 적 포탑의 정보
string* codes;  // 주어진 암호문. 예) codes[0] - 첫 번째 암호문

int map_height, map_width, num_of_allies, num_of_enemies, num_of_codes;

struct Node
{
    int y;
    int x;
    vector<pair<int, int>> path;
};

// 0 : 우 
// 1: 좌
// 2: 상
// 3: 하
int dy[4] = { 0, 0, 1, -1 };
int dx[4] = { 1, -1, 0, 0 };

// 메모리 할당 해제
void free_memory() {
    for (int i = 0; i < map_height; ++i) {
        delete[] map_data[i];
    }
    delete[] map_data;
    for (auto& ally : allies) {
        delete[] ally.second;
    }
    for (auto& enemy : enemies) {
        delete[] enemy.second;
    }
    delete[] codes;
}

// 입력 데이터를 파싱하여 변수에 저장
void parse_data(string game_data) {
    // 입력 데이터를 문자열 스트림에 담기
    istringstream iss(game_data);
    string line;

    // 첫 번째 행 데이터 읽기
    getline(iss, line);
    istringstream header(line);
    header >> map_height >> map_width >> num_of_allies >> num_of_enemies >> num_of_codes;

    // 맵 정보를 읽어오기
    map_data = new string * [map_height];
    for (int i = 0; i < map_height; ++i) {
        map_data[i] = new string[map_width];
    }

    for (int i = 0; i < map_height; ++i) {
        getline(iss, line);
        istringstream row(line);
        for (int j = 0; j < map_width; ++j) {
            row >> map_data[i][j];
        }
    }

    // 기존의 아군 정보를 초기화하고 다시 읽어오기
    allies.clear();
    for (int i = 0; i < num_of_allies; ++i) {
        getline(iss, line);
        istringstream ally_stream(line);
        string ally_name;
        ally_stream >> ally_name;

        string* ally_data = new string[4];
        for (int j = 0; ally_stream >> ally_data[j]; ++j);

        allies[ally_name] = ally_data;
    }

    // 기존의 적군 정보를 초기화하고 다시 읽어오기
    enemies.clear();
    for (int i = 0; i < num_of_enemies; ++i) {
        getline(iss, line);
        istringstream enemy_stream(line);
        string enemy_name;
        enemy_stream >> enemy_name;

        string* enemy_data = new string[2];  // 예시로 2개의 데이터를 저장
        for (int j = 0; enemy_stream >> enemy_data[j]; ++j);

        enemies[enemy_name] = enemy_data;
    }

    // 암호문 정보를 읽어오기
    codes = new string[num_of_codes];
    for (int i = 0; i < num_of_codes; ++i) {
        getline(iss, codes[i]);
    }
}

// 내 탱크의 위치 찾기
int* find_my_position() {
    int* my_position = new int[2];
    my_position[0] = -1;
    my_position[1] = -1;
    for (int i = 0; i < map_height; ++i) {
        for (int j = 0; j < map_width; ++j) {
            if (map_data[i][j] == "A") {
                my_position[0] = i;
                my_position[1] = j;
                return my_position;
            }
        }
    }
    return my_position;
}

void find_garage(vector<pair<int, int>>& g) {

    for (int i = 0; i < map_height; ++i) {
        for (int j = 0; j < map_width; ++j) {
            if (map_data[i][j] == "F") {
                g.push_back({ i, j });
                // 일단 1개만 하자
                break;

            }
        }
    }
}
//int map_height, map_width, num_of_allies, num_of_enemies, num_of_codes;

vector<pair<int, int>> bfs(int* my_pos, string goal)
{
    queue<Node> q;
    q.push({ my_pos[0], my_pos[1], {} });

    vector<vector<int>> visited(map_height, vector<int>(map_width, 0));
    visited[my_pos[0]][my_pos[1]] = 1;

    while (!q.empty())
    {
        Node now = q.front(); q.pop();

        if (map_data[now.y][now.x] == goal)
        {
            //now.path.pop_back();
            return now.path;
        }

        for (int i = 0; i < 4; i++)
        {
            int ny = now.y + dy[i];
            int nx = now.x + dx[i];

            if (ny >= map_height || ny < 0 || nx >= map_width || nx < 0) continue;
            if (map_data[ny][nx] == "R" || map_data[ny][nx] == "W") continue;
            if (map_data[ny][nx] == "T" /*|| map_data[ny][nx] == "E1"*/) continue;
            if (visited[ny][nx] == 1) continue;

            vector<pair<int, int>> temp = now.path;
            visited[ny][nx] = 1;
            temp.push_back({ ny, nx });
            q.push({ ny, nx, temp });
        }
    }

}

void next_dir(string& result, vector<pair<int, int>>& path, int* my_position, int cnt)
{
    if (my_position[0] + 1 == path[cnt].first && my_position[1] == path[cnt].second)
    {
        result = "D A";
    }

    else if (my_position[0] - 1 == path[cnt].first && my_position[1] == path[cnt].second)
    {
        result = "U A";
    }

    else if (my_position[0] == path[cnt].first && my_position[1] + 1 == path[cnt].second)
    {
        result = "R A";
    }

    else if (my_position[0] == path[cnt].first && my_position[1] - 1 == path[cnt].second)
    {
        result = "L A";
    }

    else
    {
        cout << "ERROR" << '\n';
    }

    cout << "경로" << path[cnt].first << ' ' << path[cnt].second << '\n';
}

string sol_code(string& code)
{
    int cri = 'Z' - 'A';
    int pivot = 9;

    string result = "G ";

    for (int i = 0; i < num_of_codes; i++)
    {
        int curr_c = code[i] - 'A';
        curr_c = (curr_c + pivot) % (cri + 1);

        result += curr_c + 'A';
    }
    cout << "해독: " << result << '\n';
    return result;
}

int main() {
    wstring nickname = L"KBJ";
    string game_data = init(nickname);

    bool start_game = false;
    vector<pair<int, int>> path;
    vector<pair<int, int>> garage;
    int cnt = 0;
    int n_bomb = 0;

    cout << "START! " << cnt << '\n';

    // while 반복문: 배틀싸피 메인 프로그램과 클라이언트(이 코드)가 데이터를 계속해서 주고받는 부분
    while (!game_data.empty()) {
        // 자기 차례가 되어 받은 게임정보를 파싱
        cout << "----입력데이터----\n" << game_data << "\n----------------\n";
        parse_data(game_data);

        // 파싱한 데이터를 화면에 출력하여 확인
        cout << "\n[맵 정보] (" << map_height << " x " << map_width << ")\n";
        for (int i = 0; i < map_height; ++i) {
            for (int j = 0; j < map_width; ++j) {
                cout << map_data[i][j] << " ";
            }
            cout << endl;
        }

        cout << "\n[아군 정보] (아군 수: " << num_of_allies << ")\n";
        for (const auto& ally : allies) {
            string* value = ally.second;
            if (ally.first == "A") {
                cout << "A (내 탱크) - 체력: " << value[0] << ", 방향: " << value[1]
                    << ", 보유한 일반 포탄: " << value[2] << "개, 보유한 대전차 포탄: " << value[3] << "개\n";

                n_bomb = stoi(value[3]);
            }
            else if (ally.first == "H") {
                cout << "H (아군 포탑) - 체력: " << value[0] << "\n";
            }
            else {
                cout << ally.first << " (아군 탱크) - 체력: " << value[0] << "\n";
            }
        }

        cout << "\n[적군 정보] (적군 수: " << num_of_enemies << ")\n";
        for (const auto& enemy : enemies) {
            string* value = enemy.second;
            if (enemy.first == "X") {
                cout << "X (적군 포탑) - 체력: " << value[0] << "\n";
            }
            else {
                cout << enemy.first << " (적군 탱크) - 체력: " << value[0] << "\n";
            }
        }

        cout << "\n[암호문 정보] (암호문 수: " << num_of_codes << ")\n";
        for (int i = 0; i < num_of_codes; ++i) {
            cout << codes[i] << endl;
        }

        string output = "S";  // 알고리즘 결괏값이 없을 경우를 대비하여 초기값을 S로 설정

        int* my_position = find_my_position();

        if (!start_game)
        {
            if (n_bomb < 3)
            {
                path = bfs(my_position, "F");
            }

            else
            {
                path = bfs(my_position, "X");
            }

            start_game = true;
        }

        if (cnt < path.size() - 1)
        {
            next_dir(output, path, my_position, cnt);

            for (int k = 0; k < 4; k++)
            {
                int ny = path[cnt - 1].first + dy[k];
                int nx = path[cnt - 1].second + dx[k];

                if (ny >= map_height || ny < 0 || nx >= map_width || nx < 0) continue;

                if (map_data[path[cnt].first][path[cnt].second] == "E1" ||
                    map_data[path[cnt].first][path[cnt].second] == "E2" ||
                    map_data[path[cnt].first][path[cnt].second] == "E3")
                {
                    if (k == 0) // 0 : 우 
                    {
                        output = "R F";
                    }
                    else if (k == 1) // 1: 좌
                    {
                        output = "L F";
                    }

                    else if (k == 2) // 2: 상
                    {
                        output = "U F";
                    }

                    else // 3: 하
                        output = "D F";
                    cnt--;
                    break;
                }
            }

            if (map_data[path[cnt].first][path[cnt].second] == "A1" ||
                map_data[path[cnt].first][path[cnt].second] == "A2" ||
                map_data[path[cnt].first][path[cnt].second] == "A3"
                )
            {
                cnt--;
                output = "S";
            }

            cnt++;
        }

        // 일반 포탄: (dir) F M
        // 대전차 포탄: (dir) F S

        // 목적지 도착 (창고 or 포탑)
        else
        {
            // 창고 도착

            if (n_bomb < 3 && cnt == path.size() - 1) // 3번 충전
            {
                if (n_bomb == 3)
                {
                    path.pop_back();

                    my_position[0] = (*path.rbegin()).first;
                    my_position[1] = (*path.rbegin()).second;

                    start_game = false;
                }

                output = sol_code(codes[0]);
                game_data = submit(output);

                continue;
            }
            

            string last_result = "";

            my_position[0] = (path[cnt - 1]).first;
            my_position[1] = (path[cnt - 1]).second;

            cout << my_position[0] << ' ' << my_position[1] << '\n';

            next_dir(last_result, path, my_position, cnt);
            cout << last_result << '\n';

            last_result.pop_back();
            output = last_result;
            output += "F M";
        }

        delete[] my_position;
        game_data = submit(output);
    }

    // 반복문을 빠져나왔을 때 배틀싸피 메인 프로그램과의 연결을 완전히 해제하기 위해 close 함수 호출
    free_memory();
    close();
    return 0;
}
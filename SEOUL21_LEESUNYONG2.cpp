//
// Created by LSY on 2024-10-07.
//
#include <iostream>
#include <deque>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <locale>
#include <codecvt>
#include "bridge.h"

using namespace std;

string utf8_encode(const wstring &wstr);

string **map_data;               // 맵 정보. 예) map_data[0][1] - [0, 1]의 지형/지물
map<string, string *> allies;  // 아군 정보. 예) allies.at("A") - 플레이어 본인의 정보
map<string, string *> enemies; // 적군 정보. 예) enemies.at("X") - 적 포탑의 정보
string *codes;                   // 주어진 암호문. 예) codes[0] - 첫 번째 암호문

int map_height, map_width, num_of_allies, num_of_enemies, num_of_codes;

// 메모리 할당 해제
void free_memory()
{
    for (int i = 0; i < map_height; ++i)
    {
        delete[] map_data[i];
    }
    delete[] map_data;
    for (auto &ally : allies)
    {
        delete[] ally.second;
    }
    for (auto &enemy : enemies)
    {
        delete[] enemy.second;
    }
    delete[] codes;
}

// 입력 데이터를 파싱하여 변수에 저장
void parse_data(string game_data)
{
    // 입력 데이터를 문자열 스트림에 담기
    istringstream iss(game_data);
    string line;

    // 첫 번째 행 데이터 읽기
    getline(iss, line);
    istringstream header(line);
    header >> map_height >> map_width >> num_of_allies >> num_of_enemies >> num_of_codes;

    // 맵 정보를 읽어오기
    map_data = new string *[map_height];
    for (int i = 0; i < map_height; ++i)
    {
        map_data[i] = new string[map_width];
    }

    for (int i = 0; i < map_height; ++i)
    {
        getline(iss, line);
        istringstream row(line);
        for (int j = 0; j < map_width; ++j)
        {
            row >> map_data[i][j];
        }
    }

    // 기존의 아군 정보를 초기화하고 다시 읽어오기
    allies.clear();
    for (int i = 0; i < num_of_allies; ++i)
    {
        getline(iss, line);
        istringstream ally_stream(line);
        string ally_name;
        ally_stream >> ally_name;

        string *ally_data = new string[4];
        for (int j = 0; ally_stream >> ally_data[j]; ++j)
            ;

        allies[ally_name] = ally_data;
    }

    // 기존의 적군 정보를 초기화하고 다시 읽어오기
    enemies.clear();
    for (int i = 0; i < num_of_enemies; ++i)
    {
        getline(iss, line);
        istringstream enemy_stream(line);
        string enemy_name;
        enemy_stream >> enemy_name;

        string *enemy_data = new string[2]; // 예시로 2개의 데이터를 저장
        for (int j = 0; enemy_stream >> enemy_data[j]; ++j)
            ;

        enemies[enemy_name] = enemy_data;
    }

    // 암호문 정보를 읽어오기
    codes = new string[num_of_codes];
    for (int i = 0; i < num_of_codes; ++i)
    {
        getline(iss, codes[i]);
    }
}

string decrypt()
{
    string result = "G ";
    for (int index = 0; index < codes->size(); index++)
    {
        const char original = codes[0][index];
        result += static_cast<char>(((original - 'A') + (('Z' - 'A' + 1) - ('S' - 'B'))) % ('Z' - 'A' + 1) + 'A');
    }
    return result;
}

int str_to_int(string str)
{
    int answer = 0;
    for (auto ch : str)
    {
        if ((ch < '0') || (ch > '9'))
            break;
        answer = ((answer * 10) + (ch - '0'));
    }
    return answer;
}

// 플러드필 함수
#define N 50
#define PAD 1
struct node
{
    int x, y;
    int dist;
};
int ff_map[N][N];

// R L U D
int dx[4] = {0, 0, -1, 1};
int dy[4] = {1, -1, 0, 0};

int min_he = 5;
int min_ap = 3;
int ap = 0;
int he = 0;

node my_position, target_position;
// mode 0 search for target
// mode 1 serach route for target
// mode 99 debug mode. for F.
void floodfill(node start, int mode)
{
    // init
    bool visited[N][N];
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            ff_map[i][j] = -1;
            visited[i][j] = false;
        }
    }

    deque<node> q;
    q.push_back(start);
    visited[start.x][start.y] = true;
    ff_map[start.x + PAD][start.y + PAD] = 0;

    while (!q.empty())
    {
        node cur = q.front();
        q.pop_front();
        for (int i = 0; i < 4; i++)
        {
            node next = {cur.x + dx[i], cur.y + dy[i], cur.dist + 1};

            if ((next.x >= map_width) || (next.x < 0))
                continue;
            if ((next.y >= map_width) || (next.y < 0))
                continue;
            if (visited[next.x][next.y])
                continue;
            char datum = map_data[next.x][next.y][0];

            // Debug mode. set target to F.
            if ((mode == 99) && (datum == 'F'))
            {
                target_position = next;
                return;
            }

            // 타겟 선정 단계이고, 포탄이 부족하다면 F를 타켓으로 하자.
            if ((mode == 0) && (datum == 'F') && ((ap < (min_ap * num_of_enemies)) || (he < min_he)))
            {
                target_position = next;
                cout << "target set! type is: " << datum << endl;
                return;
            }

            // 타겟 선정 단계이고, 포탄이 충분하다면, 가까운 X, E를 타켓으로 하자.
            if ((mode == 0) && (datum == 'X'))
            {
                target_position = next;
                cout << "target set! type is: " << datum << endl;
                return;
            }

            if ((mode == 0) && (datum == 'E'))
            {
                // 어차피 질것같으면, 넘어가고 다른 타켓을 잡자.
                if (((next.dist % 2) == 0) && (next.dist < 6))
                {
                    continue;
                }
                else
                {
                    cout << "next dist is: " << next.dist << endl;
                    target_position = next;
                    cout << "target set! type is: " << datum << endl;
                    return;
                }
            }

            // 아예 갈 수 없는 곳
            if (
                datum == 'R' ||
                datum == 'W' ||
                datum == 'F' ||
                datum == 'H' ||
                datum == 'A')
                continue;

            // 포탄이 남아 있어야 경로 개척이 가능하다.
            if (datum == 'E')
                if (ap > min_ap)
                    next.dist += 4;
                else
                    continue;

            // 포탄이 남아 있어야 경로 개척이 가능하다.
            if (datum == 'T')
                if (he > min_he)
                    next.dist += 2;
                else
                    continue;

            q.push_back(next);
            ff_map[next.x + PAD][next.y + PAD] = next.dist;
            visited[next.x][next.y] = true;
        }
    }

    for (int i = 0; i <= map_height; i++)
    {
        for (int j = 0; j <= map_height; j++)
        {
            cout << ff_map[i][j] << ' ';
        }
        cout << endl;
    }
}
// dx index to direction.
string idx_to_dir(int idx)
{
    // R L U D
    if (idx == 0)
    {
        return "R";
    }
    else if (idx == 1)
    {
        return "L";
    }
    else if (idx == 2)
    {
        return "U";
    }
    else if (idx == 3)
    {
        return "D";
    }
    else if (idx == -1)
    {
        return "S";
    }
}

// check enemy
string check_target(node search_point, bool tree)
{
    if ((search_point.x < 0) || (search_point.x >= map_height))
        return "";
    if ((search_point.y < 0) || (search_point.y >= map_width))
        return "";

    int datum = map_data[search_point.x][search_point.y][0];

    if ((datum == 'T') && tree && (he > min_he))
    {
        return "M";
    }
    if ((datum == 'F') && !tree)
    {
        return "F";
    }
    if ((datum == 'X') && !tree && (he > 0))
    {
        return "M";
    }
    if (datum == 'E' && !tree && (ap > 0))
    {
        return "S";
    }
    else
    {
        return "";
    }
}

// 플러드필 맵 기준으로 다음 목적지 설정
string search_n_destroy(node cur)
{
    int min_dist = -1;
    int record = -1;
    for (int i = 0; i < 4; i++)
    {
        node next = {cur.x + dx[i], cur.y + dy[i]};
        int next_ff_value = ff_map[next.x + PAD][next.y + PAD];

        string shell = check_target(next, false);
        if (shell == "M" || shell == "S")
        {
            return idx_to_dir(i) + " F " + shell;
        }
        if (shell == "F")
        {
            return decrypt();
        }

        // printf("next position is %d:%d\n", cur.x + dx[i], cur.y + dy[i]);
        // printf("value of ff_map_to_gun is: %d\n\n", next_ff_value);

        if ((record == -1) && (next_ff_value != -1))
        {
            min_dist = next_ff_value;
            record = i;
        }
        if ((next_ff_value < min_dist) && (next_ff_value != -1))
        {
            min_dist = next_ff_value;
            record = i;
        }
    }

    node next = {cur.x + dx[record], cur.y + dy[record]};
    string shell = check_target(next, true);
    if (shell != "")
    {
        return idx_to_dir(record) + " F " + shell;
    }
    return idx_to_dir(record) + " A";
}

int main()
{
    wstring nickname = L"서울21_이순용2";
    string game_data = init(nickname);

    // while 반복문: 배틀싸피 메인 프로그램과 클라이언트(이 코드)가 데이터를 계속해서 주고받는 부분
    while (!game_data.empty())
    {
        // 자기 차례가 되어 받은 게임정보를 파싱
        cout << "----입력데이터----\n"
             << game_data << "\n----------------\n";
        parse_data(game_data);

        // 파싱한 데이터를 화면에 출력하여 확인
        cout << "\n[맵 정보] (" << map_height << " x " << map_width << ")\n";
        for (int i = 0; i < map_height; ++i)
        {
            for (int j = 0; j < map_width; ++j)
            {
                cout << map_data[i][j] << " ";
                if (map_data[i][j] == "A")
                {
                    my_position = {i, j};
                }
            }
            cout << endl;
        }

        cout << "\n[아군 정보] (아군 수: " << num_of_allies << ")\n";
        for (const auto &ally : allies)
        {
            string *value = ally.second;
            if (ally.first == "A")
            {
                cout << "A (내 탱크) - 체력: " << value[0] << ", 방향: " << value[1]
                     << ", 보유한 일반 포탄: " << value[2] << "개, 보유한 대전차 포탄: " << value[3] << "개\n";
                ap = str_to_int(value[3]);
                he = str_to_int(value[2]);
            }
            else if (ally.first == "H")
            {
                cout << "H (아군 포탑) - 체력: " << value[0] << "\n";
            }
            else
            {
                cout << ally.first << " (아군 탱크) - 체력: " << value[0] << "\n";
            }
        }

        cout << "\n[적군 정보] (적군 수: " << num_of_enemies << ")\n";
        min_ap = 0;
        min_he = 0;
        for (const auto &enemy : enemies)
        {
            string *value = enemy.second;
            if (enemy.first == "X")
            {
                min_he += str_to_int(value[0]);
                cout << "X (적군 포탑) - 체력: " << value[0] << "\n";
            }
            else
            {
                min_ap += str_to_int(value[0]);
                cout << enemy.first << " (적군 탱크) - 체력: " << value[0] << "\n";
            }
        }

        cout << "\n[암호문 정보] (암호문 수: " << num_of_codes << ")\n";
        for (int i = 0; i < num_of_codes; ++i)
        {
            cout << codes[i] << endl;
        }

        // 탱크의 동작을 결정하기 위한 알고리즘을 구현하고 원하는 커맨드를 output 변수에 담기
        // 코드 구현 예시 : '아래쪽으로 전진'하되, 아래쪽이 지나갈 수 있는 길이 아니라면 '오른쪽로 전진'하라

        string output = "S"; // 알고리즘 결괏값이 없을 경우를 대비하여 초기값을 S로 설정

        floodfill({my_position.x, my_position.y, 0}, 0);
        floodfill({target_position.x, target_position.y, 0}, 1);
        output = search_n_destroy(my_position);
        cout << "output is " << output << endl;

        // while 문의 끝에는 다음 코드가 필수로 존재하여야 함
        // output에 담긴 값은 submit 함수를 통해 배틀싸피 메인 프로그램에 전달
        game_data = submit(output);
    }

    // 반복문을 빠져나왔을 때 배틀싸피 메인 프로그램과의 연결을 완전히 해제하기 위해 close 함수 호출
    free_memory();
    close();
    return 0;
}



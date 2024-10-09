//
// Created by IT DICE on 2024-10-04.
//
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <string>
#include <locale>
#include <codecvt>
#include "bridge.h"

using namespace std;

template <typename T>
using matrix = vector<vector<T>>;

constexpr int MAX_INT = 0x7fffffff;

string GROUND = "G";
string WATER = "W";
string ROCK = "R";
string TREE = "T";
string FACILITY = "F";

string MY_TANK = "A";
string MY_TOP = "H";
string ENEMY_TOP = "X";

string UP = "U";
string DOWN = "D";
string LEFT = "L";
string RIGHT = "R";
string STAY = "S";

struct Pos {
	int y;
	int x;
};

struct Task {
	Pos loc;
	int cost;
	bool operator<(const Task &data) const {
		return cost > data.cost;
	}
};

struct Target {
	string type;
	string dir;
};

vector<Pos> MOVE = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

string** map_data;  // 맵 정보. 예) map_data[0][1] - [0, 1]의 지형/지물
map<string, string*> allies;  // 아군 정보. 예) allies.at("A") - 플레이어 본인의 정보
map<string, string*> enemies;  // 적군 정보. 예) enemies.at("X") - 적 포탑의 정보
string* codes;  // 주어진 암호문. 예) codes[0] - 첫 번째 암호문

vector<Pos> visited_path; // Global variable to store the visited path

int map_height, map_width, num_of_allies, num_of_enemies, num_of_codes;

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
void parse_data(const string& game_data) {
	// 입력 데이터를 문자열 스트림에 담기
	istringstream iss(game_data);
	string line;

	// 첫 번째 행 데이터 읽기
	getline(iss, line);
    istringstream header(line);
    header >> map_height >> map_width >> num_of_allies >> num_of_enemies >> num_of_codes;

	// 맵 정보를 읽어오기
	map_data = new string*[map_height];
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

		auto* ally_data = new string[4];
		for (int j = 0; ally_stream >> ally_data[j]; ++j) {}

		allies[ally_name] = ally_data;
	}

	// 기존의 적군 정보를 초기화하고 다시 읽어오기
	enemies.clear();
	for (int i = 0; i < num_of_enemies; ++i) {
		getline(iss, line);
		istringstream enemy_stream(line);
		string enemy_name;
		enemy_stream >> enemy_name;

		auto* enemy_data = new string[2];  // 예시로 2개의 데이터를 저장
		for (int j = 0; enemy_stream >> enemy_data[j]; ++j) {}

		enemies[enemy_name] = enemy_data;
	}

	// 암호문 정보를 읽어오기
	codes = new string[num_of_codes];
	for (int i = 0; i < num_of_codes; ++i) {
		getline(iss, codes[i]);
	}
}

// 내 탱크의 위치 찾기
Pos findPosition(const string& target) {
	Pos myTankPos = {-1, -1};

	for (int row = 0; row < map_height; row++) {
		for (int col = 0; col < map_width; col++) {
			if (map_data[row][col] == target) {
				myTankPos.y = row;
				myTankPos.x = col;
				return myTankPos;
			}
		}
	}

	return myTankPos;
}

// 가장 가까운 target 위치 찾기
Pos findClosestPosition(const string& target) {
	Pos myTankPos = findPosition(MY_TANK);
	Pos closestPos = {-1, -1};
	int minDistance = MAX_INT;

	for (int row = 0; row < map_height; row++) {
		for (int col = 0; col < map_width; col++) {
			if (map_data[row][col] == target) {
				int distance = abs(myTankPos.y - row) + abs(myTankPos.x - col);
				if (distance < minDistance) {
					minDistance = distance;
					closestPos.y = row;
					closestPos.x = col;
				}
			}
		}
	}

	return closestPos;
}

Pos depthWayout(const matrix<int>& moveCost, matrix<int>& isVisited, const Pos start) {
	Pos result = {-1, -1};
	priority_queue<Task> task;

	for (const auto& dir : MOVE) {
		Pos next = {start.y + dir.y, start.x + dir.x};
		if (0 <= next.y && next.y < map_height && 0 <= next.x && next.x < map_width) {
			if (moveCost[next.y][next.x] != MAX_INT && moveCost[next.y][next.x] != 0) {
				task.push({next, moveCost[next.y][next.x]});
			}
			else if (moveCost[next.y][next.x] == 0) {
				result = start;
				return result;
			}
		}
	}

	while (!task.empty()) {
		Task now = task.top();
		task.pop();

		if (moveCost[start.y][start.x] <= now.cost)
			continue;

		if (isVisited[now.loc.y][now.loc.x] != 1) {
			isVisited[now.loc.y][now.loc.x] = 1;
			visited_path.push_back(now.loc); // Save the visited position
			result = depthWayout(moveCost, isVisited, now.loc);
			isVisited[now.loc.y][now.loc.x] = 0;
		}
	}

	return result;
}

// 다음 이동할 위치 찾기
string nextMovement(const Pos target) {
	matrix<int> moveCost(map_height, vector<int>(map_width, MAX_INT));
	Pos myTankPos = findPosition(MY_TANK);
	Pos lastTankPos = {0, 0};
	moveCost[myTankPos.y][myTankPos.x] = 0;

	int NMP = 0;
	if (strtol(allies["A"][0].c_str(), nullptr, 10) > 20)
		NMP = strtol(allies["A"][2].c_str(), nullptr, 10);

	queue<Task> task;
	task.push({myTankPos, 0});
	bool flag = false;

	// 전체 맵의 이동 비용 계산하기
	while (!task.empty() && !flag) {
		Task now = task.front();
		task.pop();

		if (now.cost > moveCost[now.loc.y][now.loc.x])
			continue;
		moveCost[now.loc.y][now.loc.x] = now.cost;

		for (const auto &dir: MOVE) {
			Pos next = {now.loc.y + dir.y, now.loc.x + dir.x};
			if (0 <= next.y && next.y < map_height && 0 <= next.x && next.x < map_width) {
				if (map_data[next.y][next.x] == GROUND)
					task.push({next, now.cost + 1});
				else if (map_data[next.y][next.x] == TREE && NMP > 0) {
					task.push({next, now.cost + 2});
					NMP -= 1;
				}
				else if (next.y == target.y && next.x == target.x) {
					lastTankPos = now.loc;
					flag = true;
					break;
				}
			}
		}
	}

	vector<vector<int>> isVisited(map_height, vector<int>(map_width, 0));
	isVisited[target.y][target.x] = 1;
	isVisited[lastTankPos.y][lastTankPos.x] = 1;

	visited_path.clear();
	Pos nextPos = depthWayout(moveCost, isVisited, lastTankPos);
	string result = STAY;

	if (nextPos.y - myTankPos.y == -1 && nextPos.x - myTankPos.x == 0)
		result = UP;
	else if (nextPos.y - myTankPos.y == 1 && nextPos.x - myTankPos.x == 0)
		result = DOWN;
	else if (nextPos.y - myTankPos.y == 0 && nextPos.x - myTankPos.x == -1)
		result = LEFT;
	else if (nextPos.y - myTankPos.y == 0 && nextPos.x - myTankPos.x == 1)
		result = RIGHT;

	return result;
}

// 적이나 나무가 시야 안에 들어오는지 확인
vector<Target> checkEnemy() {
	Pos myTankPos = findPosition(MY_TANK);
	vector<Target> result;

	for (const auto &dir: MOVE) {
		Pos next = {myTankPos.y + dir.y, myTankPos.x + dir.x};
		if (0 <= next.y && next.y < map_height && 0 <= next.x && next.x < map_width) {
			if (map_data[next.y][next.x] == GROUND || map_data[next.y][next.x] == WATER) {
				continue;
			}
			else if (map_data[next.y][next.x][0] == 'E' || map_data[next.y][next.x] == "X") {
				string resDir = STAY;
				if (dir.y == -1 && dir.x == 0)
					resDir = UP;
				else if (dir.y == 1 && dir.x == 0)
					resDir = DOWN;
				else if (dir.y == 0 && dir.x == -1)
					resDir = LEFT;
				else if (dir.y == 0 && dir.x == 1)
					resDir = RIGHT;

				result.push_back({map_data[next.y][next.x], resDir});
			}
		}
	}

	return result;
}

string decrypt() {
	string result = "G ";
	for (int index = 0; index < codes->size(); index++) {
		const char original = codes[0][index];
		result += static_cast<char>(((original - 'A') + (('Z' - 'A' + 1) - ('S' - 'B'))) % ('Z' - 'A' + 1) + 'A');
	}

	return result;
}

int main() {
	wstring nickname = L"서울21_박진";
	string game_data = init(nickname);
	int count = 0;
	bool isFact = false;

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
			if (ally.first == MY_TANK) {
				cout << "A (내 탱크) - 체력: " << value[0] << ", 방향: " << value[1]
					<< ", 보유한 일반 포탄: " << value[2] << "개, 보유한 대전차 포탄: " << value[3] << "개\n";
			}
			else if (ally.first == MY_TOP) {
				cout << "H (아군 포탑) - 체력: " << value[0] << "\n";
			}
			else {
				cout << ally.first << " (아군 탱크) - 체력: " << value[0] << "\n";
			}
		}

		cout << "\n[적군 정보] (적군 수: " << num_of_enemies << ")\n";
		for (const auto& enemy : enemies) {
			string* value = enemy.second;
			if (enemy.first == ENEMY_TOP) {
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

		//=========================================================

		count = min(strtol(allies["A"][2].c_str(), nullptr, 10),
			strtol(allies["A"][3].c_str(), nullptr, 10));

		int requiredSmart = 0, requiredNormal = 0;

		for (const auto &enemy: enemies) {
			string *value = enemy.second;
			if (enemy.first != ENEMY_TOP) {
				requiredSmart += strtol(value[0].c_str(), nullptr, 10) / 10;
			}
		}
		requiredNormal = (strtol(enemies["X"][0].c_str(), nullptr, 10) / 10) + 4;
		int required = max(requiredSmart, requiredNormal);

		if (count >= required)
			isFact = true;
		else
			isFact = false;

		//=========================================================
		// U D L R -> 방향 전환 명령어
		// {dir} A -> 해당 방향으로 이동
		// {dir} F M -> 해당 방향으로 일반 미사일 발사
		// {dir} F S -> 해당 방향으로 대전차 미사일 발사
		// G *** -> 암호문 해독한 것 전송
		// {dir} S -> 방향 전환하고 아무것도 안하기

		string output = "S"; // 알고리즘 결괏값이 없을 경우를 대비하여 초기값을 S로 설정
		Pos myTankPos = findPosition(MY_TANK);
		Pos target = {map_height, map_width}; // Initialize with out-of-bound values
		Pos facilityPos = findClosestPosition(FACILITY);
		if (facilityPos.y == -1 && facilityPos.x == -1) {
			isFact = true;
		}
		int minDistance = map_height + map_width;

		// Track previous positions of enemies
		unordered_map<string, Pos> previousPositions;

		// Check for enemy tanks first, but move to facility if antitank ammo is insufficient
		for (const auto& enemy : enemies) {
			Pos enemyPos = findClosestPosition(enemy.first);
			int distance = abs(enemyPos.y - myTankPos.y) + abs(enemyPos.x - myTankPos.x);

			// Adjust target if enemy is moving
			if (enemy.first[0] == 'E' && previousPositions.find(enemy.first) != previousPositions.end()) {
				Pos prevPos = previousPositions[enemy.first];
				// Calculate the direction of movement
				Pos moveDir = {enemyPos.y - prevPos.y, enemyPos.x - prevPos.x};
				// Calculate the new target positioning 2 steps ahead in the movement direction
				enemyPos = {enemyPos.y + 2 * moveDir.y, enemyPos.x + 2 * moveDir.x};
			}

			if (enemy.first != ENEMY_TOP && distance < minDistance) {
				if (isFact) {
					target = enemyPos;
					minDistance = distance;
				}
				else {
					target = facilityPos;
					break;
				}
				previousPositions[enemy.first] = enemyPos; // Update previous position
			}
		}

		// Check for enemy top turret if no valid enemy tank found, but move to facility if normal ammo is insufficient
		if (target.y == map_height && target.x == map_width) {
			for (const auto& enemy : enemies) {
				Pos enemyPos = findClosestPosition(enemy.first);
				int distance = abs(enemyPos.y - myTankPos.y) + abs(enemyPos.x - myTankPos.x);
				if (enemy.first == ENEMY_TOP && distance < minDistance) {
					if (isFact) {
						target = enemyPos;
						minDistance = distance;
					}
					else {
						target = facilityPos;
						break;
					}
				}
			}
		}

		if (strtol(allies["A"][0].c_str(), nullptr, 10) <= 20 &&
			map_data[target.y][target.x][0] == 'E') {
			target = findClosestPosition("X");
		}

		// If no valid enemy target found
		if (target.y == map_height && target.x == map_width) {
			target = facilityPos;
		}

		if (target.y == -1 && target.x == -1) {
			submit("S");
			continue;
		}

		// Pretty-print the target, current tank position, and the path
		auto printMapWithPath = [&](const Pos& startPos, const Pos& endPos, const vector<Pos>& visited_path) {
			matrix<char> mapCopy(map_height, vector<char>(map_width, '.'));

			for (int y = 0; y < map_height; ++y) {
				for (int x = 0; x < map_width; ++x) {
					// Populate mapCopy with current positions
					mapCopy[y][x] = map_data[y][x][0]; // Assuming map_data has single-character elements
				}
			}
			mapCopy[startPos.y][startPos.x] = '#'; // Start
			mapCopy[endPos.y][endPos.x] = '@'; // Target

			// Mark the path using visited_path
			for (const auto& pos : visited_path) {
				if (!(pos.y == startPos.y && pos.x == startPos.x) && !(pos.y == endPos.y && pos.x == endPos.x)) {
					mapCopy[pos.y][pos.x] = '*'; // Path
				}
			}

			cout << "Map with Path:" << endl;
			for (const auto& row : mapCopy) {
				for (const char cell : row) {
					cout << cell << ' ';
				}
				cout << endl;
			}
		};

		string wayOut = nextMovement(target);
		vector<Target> allTarget = checkEnemy();
		printMapWithPath(myTankPos, target, visited_path); // Print map with path for debugging
		bool isFired = false;

		// 미사일 발사가 필요한 경우 확인
		if (strtol(allies["A"][0].c_str(), nullptr, 10) > 20) {
			for (const auto &value: allTarget) {
				if (value.type[0] == 'E' && allies["A"][3] > "0") {
					isFired = true;
					if (value.dir == UP) {
						output = "U F S";
					}
					else if (value.dir == DOWN) {
						output = "D F S";
					}
					else if (value.dir == LEFT) {
						output = "L F S";
					}
					else if (value.dir == RIGHT) {
						output = "R F S";
					}
					break;
				}
			}
		}
		for (const auto &value: allTarget) {
			if (value.type == ENEMY_TOP && allies["A"][2] > "0") {
				isFired = true;
				if (value.dir == UP) {
					output = "U F M";
				}
				else if (value.dir == DOWN) {
					output = "D F M";
				}
				else if (value.dir == LEFT) {
					output = "L F M";
				}
				else if (value.dir == RIGHT) {
					output = "R F M";
				}
				break;
			}
		}

		// 이동만 하는 경우
		if (!isFired) {
			if (wayOut == UP) {
				if (map_data[myTankPos.y - 1][myTankPos.x] == "T") {
					output = "U F M";
				}
				else {
					output = "U A";
				}
			}
			else if (wayOut == DOWN) {
				if (map_data[myTankPos.y + 1][myTankPos.x] == "T") {
					output = "D F M";
				}
				else {
					output = "D A";
				}
			}
			else if (wayOut == LEFT) {
				if (map_data[myTankPos.y][myTankPos.x - 1] == "T") {
					output = "L F M";
				}
				else {
					output = "L A";
				}
			}
			else if (wayOut == RIGHT) {
				if (map_data[myTankPos.y][myTankPos.x + 1] == "T") {
					output = "R F M";
				}
				else {
					output = "R A";
				}
			}
			else if (wayOut == STAY) {
				// 경로를 찾든 못찾든 일단 도망치기!!!
				if (isFact) {
					for (const auto &dir: MOVE) {
						Pos next = {myTankPos.y + dir.y, myTankPos.x + dir.x};
						if (0 <= next.y && next.y < map_height && 0 <= next.x && next.x < map_width) {
							if (map_data[next.y][next.x] == "G") {
								if (dir.y == -1 && dir.x == 0)
									output = "U A";
								else if (dir.y == 1 && dir.x == 0)
									output = "D A";
								else if (dir.y == 0 && dir.x == -1)
									output = "L A";
								else if (dir.y == 0 && dir.x == 1)
									output = "R A";
							}
						}
					}
				}
			}
		}

		if (output == "S" && !isFired && !isFact) {
			output = decrypt();
		}

		//=========================================================
		// while 문의 끝에는 다음 코드가 필수로 존재하여야 함
		// output에 담긴 값은 submit 함수를 통해 배틀싸피 메인 프로그램에 전달
		game_data = submit(output);
	}

	// 반복문을 빠져나왔을 때 배틀싸피 메인 프로그램과의 연결을 완전히 해제하기 위해 close 함수 호출
	free_memory();
	close();
	return 0;
}

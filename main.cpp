#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;



/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main()
{
    bool used_boost = false;

    // game loop
    while (1) {
        int x;
        int y;
        int nextCheckpointX; // x position of the next check point
        int nextCheckpointY; // y position of the next check point
        int nextCheckpointDist; // distance to the next checkpoint
        int nextCheckpointAngle; // angle between your pod orientation and the direction of the next checkpoint
        cin >> x >> y >> nextCheckpointX >> nextCheckpointY >> nextCheckpointDist >> nextCheckpointAngle; cin.ignore();
        int opponentX;
        int opponentY;
        cin >> opponentX >> opponentY; cin.ignore();

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        int thrust;
        
        if (nextCheckpointAngle > 90 || nextCheckpointAngle < -90) {
            thrust = 0;
        } else if (nextCheckpointAngle !=0 && nextCheckpointDist == 1000) {
            thrust = 0;
        } else if (nextCheckpointDist < 100) {
            thrust = 50;
        } else if (nextCheckpointDist < 50) {
            thrust = 0;
        } else {
            thrust = 100;
        }
        // You have to output the target position
        // followed by the power (0 <= thrust <= 100)
        // i.e.: "x y thrust"
        if (!used_boost && nextCheckpointDist > 7000 && nextCheckpointAngle == 0 ) {
            cout << nextCheckpointX << " " << nextCheckpointY << " BOOST" << endl;
            used_boost = true;
        } else {
            cout << nextCheckpointX << " " << nextCheckpointY << " " << thrust << endl;
        }
    }
}

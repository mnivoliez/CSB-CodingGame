#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

#define MAX_DEPTH 4
#define PI 3.14f

// Step:
//  * Populate sim world
//  * Compute MAX_DEPTH next turn
//      * Each turn will be represented by a Move and a resulting state
//      * Each turn will have a parent turn
//      * Each turn will have a score compute from the distance separating the pod from the next checkpoint
//  * We keep the best sequence of move based on the score
//  * We applied the first move of the sequence
//  * Rince and repeat.


struct FVec2 {
  FVec2() : X(0.f), Y(0.f){};

  float X;
  float Y;

  FVec2 operator-(const FVec2 &Other) const;
  float Length() const;
  float Length2() const;
  float Dot(const FVec2 &Other) const;
  float GetAngle(const FVec2 &Other) const;

  friend ostream& operator<<(ostream& os, const FVec2& Vec)
  {
    os << Vec.X << " " << Vec.Y;
    return os;
  }
};

struct FGameObject {
  FGameObject() : Radius(0.0f){};

  FVec2 Position;
  float Radius;
  string ID;

  friend ostream& operator<<(ostream& os, const FGameObject& GameObject)
  {
    os << GameObject.ID << endl
        << "\tPosition: " << GameObject.Position << endl;
    return os;
  }
};

// Instruction for a pod
struct FMove {
  bool bUseShield;
  float Angle;
  float Thrust;

  friend ostream& operator<<(ostream& os, const FMove& Move)
  {
    os << "Move:\n" 
        << "\tUse shield: " << (Move.bUseShield ? "Yes" : "No") << endl
        << "\tAngle: " << Move.Angle
        << "\tThrust: " << Move.Thrust << endl;
    return os;
  }
};

struct FPod : public FGameObject {
  FPod(const string &Id) : Timeout(100) {
    Radius = 400.f;
    ID = Id;
  }

  FPod(const FPod &Base)
      : Timeout(Base.Timeout), Velocity(Base.Velocity), Angle(Base.Angle) {
    ID = Base.ID;
    Radius = 400.f;
    Position = Base.Position;
  };

  FVec2 Velocity;
  float Angle;

  float Timeout;

  void Rotate(const FVec2 &Pos);
  void Boost(const int thrust);
  void Move();
  void End();

  float DiffAngle(const FVec2 &Pos) const;

  void ApplyMove(const FMove &Move);

  friend ostream& operator<<(ostream& os, const FPod& Pod)
  {
    os << (FGameObject)Pod 
        << "\tVelocity: "<< Pod.Velocity << endl;
    return os;
  }
};

struct FCheckPoint : public FGameObject {
  FCheckPoint() {
    ID = "CheckPoint";
    Radius = 600.f;
  }
};

// The idea is to predict the best course of action for our pod.
// To do that, just like in chess we can try to predict the next moves
// We can imagine the chain of move as a tree.
// For each branch of the tree we can either thrust, boost or shield.
// After predicting a certain number of move we can use a scoring function to
// select our best course of action.
struct FNode {
  FNode(FPod &Instance) : PodInstance(Instance){};

  FPod PodInstance;
  FMove Move;

  FNode *Parent;
};

struct FSimulation {
  // As given at the begining of the turn
  FPod *PlayerPod;
  FPod *EnemyPod;
  FCheckPoint NextCheckpoint;
  float CalculationTimeLeft;

  // We need to store each generation for each pod
  vector<FNode> CurrentGeneration;
  vector<FNode> PastGenerations;
  vector<FNode> SuccessfulGenerations;

  // Get original info into the simulation
  void Reset(FPod &Player, FPod &Enemy, FCheckPoint &CheckPoint);
  // Simulate the next X turn, X between 1 to MAX_DEPTH
  void Simulate();

  void SimulateTurn();
};

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main() {
  bool used_boost = false;
  FSimulation Sim;
  FPod Player("PLAYER");
  FPod Opponent("Enemy");

  // game loop
  while (1) {

    // the idea is to simulate some turn in advance with the cost of each
    // action. Lets first do that for only our pod.
    FCheckPoint NextCheckpoint;

    int nextCheckpointDist;  // distance to the next checkpoint
    int nextCheckpointAngle; // angle between your pod orientation and the
                             // direction of the next checkpoint
    cin >> Player.Position.X >> Player.Position.Y 
        >> NextCheckpoint.Position.X >> NextCheckpoint.Position.Y 
        >> nextCheckpointDist >> nextCheckpointAngle;
    cin.ignore();

    FVec2 FromPlayerToCheckpoint = NextCheckpoint.Position - Player.Position;
    float dist = FromPlayerToCheckpoint.Length();
    
    cin >> Opponent.Position.X >> Opponent.Position.Y;
    cin.ignore();
    
    // Write an action using cout. DON'T FORGET THE "<< endl"
    // To debug: cerr << "Debug messages..." << endl;
    cerr << Opponent << endl;
    cerr << Player << endl;
    cerr << NextCheckpoint << endl;
   
    int thrust;

    if (nextCheckpointAngle > 90 || nextCheckpointAngle < -90) {
      thrust = 0;
    } else if (nextCheckpointAngle != 0 && nextCheckpointDist == 1000) {
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
    if (!used_boost && nextCheckpointDist > 7000 && nextCheckpointAngle == 0) {
      cout << NextCheckpoint.Position << " BOOST" << endl;
      used_boost = true;
    } else {
      cout << NextCheckpoint.Position << " " << thrust
           << endl;
    }
  }
}

/// Vec 2 ===================================================
FVec2 FVec2::operator-(const FVec2 &Other) const {
  FVec2 Result;
  Result.X = X - Other.X;
  Result.Y = Y - Other.Y;

  return Result;
}

float FVec2::Length2() const { return X * X + Y * Y; }

float FVec2::Length() const { return sqrt(this->Length2()); }

float FVec2::Dot(const FVec2 &Other) const { return X * Other.X + Y * Other.Y; }

float FVec2::GetAngle(const FVec2 &Other) const {
  float dot = this->Dot(Other);

  // result in rad
  float a = acos(dot / (Length() * Other.Length()));

  // convert it to deg
  return a * 180 / PI;
}
/// Pod ======================================================
void FPod::Rotate(const FVec2 &Pos) {
  float a = DiffAngle(Pos);

  a = clamp(a, -18.f, 18.f);

  Angle += a;
  if (Angle >= 360.f) {
    Angle -= 360.f;
  } else if (Angle <= 0.f) {
    Angle += 360.f;
  }
}

void FPod::Boost(const int thrust) {
  // get the angle in rad
  float ra = Angle * PI / 180.f;

  Velocity.X += cos(ra) * thrust;
  Velocity.Y += sin(ra) * thrust;
}

void FPod::Move() {
  Position.X += Velocity.X;
  Position.Y += Velocity.Y;
}

void FPod::End() {}

void ApplyMove(const FMove& Move)
{

}

float FPod::DiffAngle(const FVec2 &Pos) const {
  float a = Position.GetAngle(Pos);

  float right = Angle <= a ? a - Angle : 360.0 - Angle + a;
  float left = Angle >= a ? Angle - a : Angle + 360.0 - a;

  if (right < left) {
    return right;
  } else {
    // We return a negative angle if we must rotate to left
    return -left;
  }
}

/// Node =====================================================

/// Simulation ===============================================
void FSimulation::Reset(FPod &Player, FPod &Enemy, FCheckPoint &Checkpoint) {
  CalculationTimeLeft = 75.f;
  PlayerPod = &Player;
  EnemyPod = &Enemy;
  NextCheckpoint = Checkpoint;
}

void FSimulation::Simulate() { SimulateTurn(); }

void FSimulation::SimulateTurn() {
  FMove NextMove;
  FPod EnemyFutureState(*EnemyPod);
  EnemyFutureState.Rotate(NextCheckpoint.Position);
  EnemyFutureState.Boost(100.0f);
  EnemyFutureState.Move();
  cerr << "Future Enemy pos: " << EnemyFutureState.Position.X << " "
       << EnemyFutureState.Position.Y << endl;
}

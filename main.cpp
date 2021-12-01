#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

#define MAX_DEPTH 4
#define MAX_ANGLE 5
#define MAX_THRUST 100
#define K 3
#define TAU 6.283185307179586232
#define PI TAU / 2

// Step:
//  * Populate sim world
//  * Compute MAX_DEPTH next turn
//      * Each turn will be represented by a Move and a resulting state
//      * Each turn will have a parent turn
//      * Each turn will have a score compute from the distance separating the
//      pod from the next checkpoint
//  * We keep the best sequence of move based on the score
//  * We applied the first move of the sequence
//  * Rince and repeat.

class FAngle {
public:
  FAngle() : Value(0.f){};

public:
  static FAngle FromDeg(float Deg) { return FAngle(Deg / 360.f * TAU); }
  static FAngle FromRad(float Rad) { return FAngle(Rad); }

  float ToRad() const { return Value; }
  float ToDeg() const { return Value / TAU * 360.f; }

  float Cos() const { return cos(Value); }
  float Sin() const { return sin(Value); }
  float Tan() const { return tan(Value); }

private:
  FAngle(float Val) : Value(Val){};

private:
  float Value;
};

struct FVec2 {
  FVec2() : X(0.f), Y(0.f){};

  float X;
  float Y;

  FVec2 operator-(const FVec2 &Other) const;
  FVec2 operator+(const FVec2 &Other) const;
  FVec2 operator*(const float Value) const;
  bool operator==(const FVec2 &Other) const {
    return X == Other.X && Y == Other.Y;
  }
  bool operator!=(const FVec2 &Other) const { return !(*this == Other); }
  float Length() const;
  float Length2() const;
  float Dot(const FVec2 &Other) const;
  FAngle GetAngle(const FVec2 &Other) const;
  FVec2 Rotate(const FAngle Angle) const;
  FVec2 Normalise() const;

  friend ostream &operator<<(ostream &os, const FVec2 &Vec) {
    os << Vec.X << " " << Vec.Y;
    return os;
  }
};

FVec2 Lerp(const FVec2 &A, const FVec2 &B, float Delta) {
  return A * Delta + B * (1 - Delta);
}

// Instruction for a pod
struct FMove {
  FMove() : bUseShield(false), Thrust(0), bUseBoost(false){};

  bool bUseShield;
  FVec2 Target;
  float Thrust;
  bool bUseBoost;

  friend ostream &operator<<(ostream &os, const FMove &Move) {
    os << "Move:\n"
       << "\tUse shield: " << (Move.bUseShield ? "Yes" : "No") << endl
       << "\tUseBoost: " << (Move.bUseBoost ? "Yes" : "No") << endl
       << "\tTarget: " << Move.Target << "\n\tThrust: " << Move.Thrust << endl;
    return os;
  }
};

class GameMap {
public:
  GameMap() : Laps(0){};

public:
  void UpdateCheckpoint(const FVec2 &Pos);

  bool IsNextCheckpointBestCandidateForBoost(const FVec2 &Pos) const;

  bool PassFirstLaps() const { return Laps >= 1; }

  FVec2 GetNextCheckpoint(const FVec2 &Pos) const;

private:
  int FindCheckpointIndex(const FVec2 &Pos) const;
  void ComputeBestCandidateForBoost();

private:
  int Laps;
  vector<FVec2> Checkpoints;
  FVec2 LastCheckpointPass;
  FVec2 AimedCheckpoint;
  FVec2 BestCandidateForBoost;
};

class ISolver {
public:
  virtual void Update(const FVec2 &PlayerPos, const FVec2 &EnemyPos,
                      const FVec2 &CPPos, const float CPAngle,
                      const float CPDist) = 0;
  virtual FMove Solve() = 0;
};

class FMapSolver : public ISolver {
public:
  virtual void Update(const FVec2 &PlayerPos, const FVec2 &EnemyPos,
                      const FVec2 &CPPos, const float CPAngle,
                      const float CPDist) override;
  virtual FMove Solve() override;

  FMapSolver(GameMap *GMap) : BoostUsed(false), Map(GMap){};

private:
  FMove Explore() const;
  FMove ComputeMoveFromMap();

protected:
  GameMap *Map;
  FVec2 PlayerPos;
  FVec2 EnemyPos;
  FVec2 CPPos;
  float CPAngle;
  float CPDist;
  bool BoostUsed;
};

/// Vec 2 ===================================================
FVec2 FVec2::operator-(const FVec2 &Other) const {
  FVec2 Result;
  Result.X = X - Other.X;
  Result.Y = Y - Other.Y;

  return Result;
}

FVec2 FVec2::operator+(const FVec2 &Other) const {
  FVec2 Result;
  Result.X = X + Other.X;
  Result.Y = Y + Other.Y;

  return Result;
}

FVec2 FVec2::operator*(const float Value) const {
  FVec2 Result;
  Result.X = X * Value;
  Result.Y = Y * Value;
  return Result;
}

float FVec2::Length2() const { return X * X + Y * Y; }

float FVec2::Length() const { return sqrt(this->Length2()); }

float FVec2::Dot(const FVec2 &Other) const { return X * Other.X + Y * Other.Y; }

FAngle FVec2::GetAngle(const FVec2 &Other) const {
  float dot = this->Dot(Other);

  float v1L = Length();
  float v2L = Other.Length2();
  float l = v1L * v2L;
  float dotOnLength = dot / l;

  // result in rad
  float a = acos(dotOnLength);

  return FAngle::FromRad(a);
}

FVec2 FVec2::Rotate(const FAngle Angle) const {
  FVec2 Result;

  Result.X = X * Angle.Cos() - Y * Angle.Sin();
  Result.Y = X * Angle.Sin() + Y * Angle.Cos();

  return Result;
}

FVec2 FVec2::Normalise() const {
  FVec2 Norm;
  float l = Length();
  cerr << *this << endl << l << endl;

  Norm.X = X / l;
  Norm.Y = Y / l;
  cerr << Norm << endl;
  return Norm;
}

/// CheckPointManager ========================================
void GameMap::UpdateCheckpoint(const FVec2 &Pos) {
  int index = FindCheckpointIndex(Pos);

  if (index == -1) {
    Checkpoints.emplace_back(Pos);
  } else if (Checkpoints.size() > 1 && Checkpoints[index] == AimedCheckpoint &&
             index == 0) {
    ++Laps;
    if (BestCandidateForBoost == FVec2()) {
      ComputeBestCandidateForBoost();
    }
  }

  if (AimedCheckpoint != Pos) {
    LastCheckpointPass = AimedCheckpoint;
    AimedCheckpoint = Pos;
  }
}

bool GameMap::IsNextCheckpointBestCandidateForBoost(const FVec2 &Pos) const {
  return Pos == BestCandidateForBoost;
}
int GameMap::FindCheckpointIndex(const FVec2 &Pos) const {
  int index = -1;
  for (int idx = 0; idx < Checkpoints.size(); ++idx) {
    if (Checkpoints[idx] == Pos) {
      index = idx;
      break;
    }
  }
  return index;
}

FVec2 GameMap::GetNextCheckpoint(const FVec2 &Pos) const {
  int index = FindCheckpointIndex(Pos);
  return Checkpoints[(index + 1) % Checkpoints.size()];
}

void GameMap::ComputeBestCandidateForBoost() {
  float GrtDist2 = 0.f;
  for (int i = 0; i < Checkpoints.size(); ++i) {
    int j = i + 1;
    if (j == Checkpoints.size()) {
      j = 0;
    }

    FVec2 Pos1 = Checkpoints[i];
    FVec2 Pos2 = Checkpoints[j];

    float dist2 = (Pos1 - Pos2).Length2();
    if (dist2 >= GrtDist2) {
      GrtDist2 = dist2;
      BestCandidateForBoost = Pos2;
    }
  }
}

/// BasicSolution ============================================
void FMapSolver::Update(const FVec2 &InPlayerPos, const FVec2 &InEnemyPos,
                        const FVec2 &InCPPos, const float InCPAngle,
                        const float InCPDist) {
  PlayerPos = InPlayerPos;
  EnemyPos = InEnemyPos;
  CPPos = InCPPos;
  CPDist = InCPDist;
  CPAngle = InCPAngle;

  Map->UpdateCheckpoint(CPPos);
};

FMove FMapSolver::Solve() {
  if (Map->PassFirstLaps()) {
    return ComputeMoveFromMap();
  }
  return Explore();
}

FMove FMapSolver::Explore() const {
  FMove Move;
  int thrust = 0;
  if (abs(CPAngle) > 90) {
    thrust = 0;
  } else {
    float angleCoef = 0;
    angleCoef = FAngle::FromDeg(CPAngle).Cos();
    float d = CPDist / 600.0f * K;
    float distCoef = clamp(d, 0.f, 1.f);
    thrust = MAX_THRUST * angleCoef * distCoef;
  }

  Move.Thrust = thrust;
  Move.Target = CPPos;
  cerr << Move << endl;
  return Move;
}
FMove FMapSolver::ComputeMoveFromMap() { 
    
    FMove Result;

    Result.Target = CPPos;
    Result.Thrust = 100.f;
    return Result; 
    
}

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main() {
  GameMap GMap;
  FMapSolver MapSolver(&GMap);
  // game loop
  while (1) {

    // the idea is to simulate some turn in advance with the cost of each
    // action. Lets first do that for only our pod.
    FVec2 CPPos;
    FVec2 PlayerPos;
    FVec2 EnemyPos;
    int nextCheckpointDist;  // distance to the next checkpoint
    int nextCheckpointAngle; // angle between your pod orientation and the
                             // direction of the next checkpoint
    cin >> PlayerPos.X >> PlayerPos.Y >> CPPos.X >> CPPos.Y >>
        nextCheckpointDist >> nextCheckpointAngle;
    cin.ignore();

    cin >> EnemyPos.X >> EnemyPos.Y;

    cin.ignore();

    // Write an action using cout. DON'T FORGET THE "<< endl"
    // To debug: cerr << "Debug messages..." << endl;

    MapSolver.Update(PlayerPos, EnemyPos, CPPos, nextCheckpointAngle,
                     nextCheckpointDist);
    FMove Move = MapSolver.Solve();
    if (Move.bUseBoost) {
      cout << (int)Move.Target.X << " " << (int)Move.Target.Y << " BOOST" << endl;
    } else {
      cout << (int)Move.Target.X << " " << (int)Move.Target.Y << " " << Move.Thrust << endl;
    }

    /*bool shouldUseBoost =
        !used_boost && CPManager.PassFirstLaps() && nextCheckpointDist > 7000 &&
        nextCheckpointAngle < MAX_ANGLE && nextCheckpointAngle > -MAX_ANGLE &&
        CPManager.IsNextCheckpointBestCandidateForBoost(CPPos);
    if (shouldUseBoost) {
        cout << Dir << " BOOST" << endl;
        used_boost = true;
    }*/
  }
}

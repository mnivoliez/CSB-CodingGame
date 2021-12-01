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
#define K 4
#define PI 3.14f

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

struct FVec2 {
  FVec2() : X(0.f), Y(0.f){};

  float X;
  float Y;

  FVec2 operator-(const FVec2 &Other) const;
  FVec2 operator+(const FVec2 &Other) const;
  FVec2 operator*(const float Value) const;
  float Length() const;
  float Length2() const;
  float Dot(const FVec2 &Other) const;
  float GetAngle(const FVec2 &Other) const;
  FVec2 Normalise() const;

  friend ostream &operator<<(ostream &os, const FVec2 &Vec) {
    os << Vec.X << " " << Vec.Y;
    return os;
  }
};

FVec2 Lerp(const FVec2 &A, const FVec2 &B, float Delta) {
  cerr << "A: " << A << "\nB: " << B << "\nA * Delta: " << A * Delta
       << "\nB * (1-Delta): " << B * (1 - Delta)
       << "\nA * Delta + B * (1-Delta): " << A * Delta + B * (1 - Delta)
       << endl;
  return A * Delta + B * (1 - Delta);
}

struct FGameObject {
  FGameObject() : Radius(0.0f){};

  FVec2 Position;
  float Radius;
  string ID;

  friend ostream &operator<<(ostream &os, const FGameObject &GameObject) {
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

  friend ostream &operator<<(ostream &os, const FMove &Move) {
    os << "Move:\n"
       << "\tUse shield: " << (Move.bUseShield ? "Yes" : "No") << endl
       << "\tAngle: " << Move.Angle << "\tThrust: " << Move.Thrust << endl;
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

  friend ostream &operator<<(ostream &os, const FPod &Pod) {
    os << (FGameObject)Pod << "\tVelocity: " << Pod.Velocity << endl;
    return os;
  }
};

struct FCheckPoint : public FGameObject {
  FCheckPoint() {
    ID = "CheckPoint";
    Radius = 600.f;
  }
  FCheckPoint(const string &Id, const FVec2 &Pos, const int Ord) : Order(Ord) {
    ID = Id;
    Position = Pos;
    Radius = 600.f;
  }

  int Order;
};

class FCheckpointsManager {
public:
  FCheckpointsManager()
      : Laps(0), LastCheckpointPass(""), AimedCheckpoint(""),
        BestCandidateForBoost(""){};

public:
  void UpdateCheckpoint(const FVec2 &Pos);

  bool IsNextCheckpointBestCandidateForBoost(const FVec2 &Pos) const;

  bool PassFirstLaps() const { return Laps >= 1; }

  FVec2 GetNextCheckpoint(const FVec2 &Pos) const;

private:
  string GetCheckpointUniqueID(const FVec2 &Pos) const;
  void ComputeBestCandidateForBoost();

private:
  int Laps;
  map<string, FCheckPoint> Checkpoints;
  vector<string> ChecpointOrder;
  string LastCheckpointPass;
  string AimedCheckpoint;
  string BestCandidateForBoost;
};

class ISolution {
public:
  virtual void Update(const FVec2 &PlayerPos, const FVec2 &EnemyPos,
                      const FVec2 &CPPos, const float CPAngle,
                      const float CPDist) = 0;
  virtual FMove Solve() = 0;
};

class FBasicSolution : public ISolution {
public:
  virtual void Update(const FVec2 &PlayerPos, const FVec2 &EnemyPos,
                      const FVec2 &CPPos, const float CPAngle,
                      const float CPDist) override;
  virtual FMove Solve() override;

private:
  FVec2 PlayerPos;
  FVec2 CPPos;
  float CPAngle;
  float CPDist;
};

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main() {
  bool used_boost = false;
  FPod Player("PLAYER");
  FPod Opponent("Enemy");

  FCheckpointsManager CPManager;
  FBasicSolution BasicSol;

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
    CPManager.UpdateCheckpoint(CPPos);
    BasicSol.Update(PlayerPos, EnemyPos, CPPos, nextCheckpointAngle, nextCheckpointDist);
    FMove Move = BasicSol.Solve();
    cout << CPPos << " " << Move.Thrust << endl;
    

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

float FVec2::GetAngle(const FVec2 &Other) const {
  float dot = this->Dot(Other);

  float v1L = Length();
  float v2L = Other.Length2();
  float l = v1L * v2L;
  float dotOnLength = dot / l;

  // result in rad
  float a = acos(dotOnLength);
  // convert it to deg
  float adeg = a * 180 / PI;

  return adeg;
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
void FCheckpointsManager::UpdateCheckpoint(const FVec2 &Pos) {
  string Id = GetCheckpointUniqueID(Pos);

  auto CP = Checkpoints.find(Id);
  if (CP == Checkpoints.end()) {
    Checkpoints.emplace(Id, FCheckPoint(Id, Pos, Checkpoints.size()));
    ChecpointOrder.push_back(Id);
  } else if (Id != AimedCheckpoint && CP->second.Order == 0) {
    ++Laps;
    if (BestCandidateForBoost == "") {
      ComputeBestCandidateForBoost();
    }
  }

  if (AimedCheckpoint.compare(Id) != 0) {
    LastCheckpointPass = AimedCheckpoint;
    AimedCheckpoint = Id;
  }
}

bool FCheckpointsManager::IsNextCheckpointBestCandidateForBoost(
    const FVec2 &Pos) const {
  return GetCheckpointUniqueID(Pos) == BestCandidateForBoost;
}

FVec2 FCheckpointsManager::GetNextCheckpoint(const FVec2 &Pos) const {
  string Id = GetCheckpointUniqueID(Pos);
  string NextId = "";
  for (int i = 0; i < ChecpointOrder.size(); ++i) {
    int j = i + 1;
    if (j == ChecpointOrder.size())
      j = 0;

    NextId = ChecpointOrder[j];
  }

  auto CP = Checkpoints.find(NextId);
  if (CP != Checkpoints.end()) {
    return CP->second.Position;
  }
  return FVec2();
}

string FCheckpointsManager::GetCheckpointUniqueID(const FVec2 &Pos) const {
  return to_string((int)Pos.X) + " " + to_string((int)Pos.Y);
}

void FCheckpointsManager::ComputeBestCandidateForBoost() {
  float GrtDist2 = 0.f;
  for (int i = 0; i < ChecpointOrder.size(); ++i) {
    int j = i + 1;
    if (j == ChecpointOrder.size()) {
      j = 0;
    }

    string Id1 = ChecpointOrder[i];
    string Id2 = ChecpointOrder[j];

    float dist2 =
        (Checkpoints[Id2].Position - Checkpoints[Id1].Position).Length2();
    if (dist2 >= GrtDist2) {
      GrtDist2 = dist2;
      BestCandidateForBoost = Id2;
    }
  }
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

void ApplyMove(const FMove &Move) {}

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

void FBasicSolution::Update(const FVec2 &InPlayerPos, const FVec2 &InEnemyPos,
                            const FVec2 &InCPPos, const float InCPAngle,
                            const float InCPDist){
    PlayerPos = InPlayerPos;
    CPPos = InCPPos;
    CPDist = InCPDist;
    CPAngle = InCPAngle;

};
FMove FBasicSolution::Solve() {

  int thrust = 0;
  if (CPAngle > 90 || CPAngle < -90) {
    thrust = 0;
  } else {
    float angleCoef = clamp(1 - CPAngle / 180.f, 0.f, 1.f);
    float d = 1;
    if (CPDist < 600.f * K) {
      d = CPDist / (600.f * K);
    }
    float distCoef = clamp(d, 0.f, 1.f);
    thrust = MAX_THRUST * angleCoef * distCoef;
  }

  FMove Move;
  Move.Thrust = thrust;
  return Move;
}

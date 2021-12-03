#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

using namespace std;

#define MAX_THRUST 100
#define TAU 6.283185307179586232
#define PI TAU / 2

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
  FVec2 operator/(const float Value) const;
  bool operator==(const FVec2 &Other) const {
    return X == Other.X && Y == Other.Y;
  }
  bool operator!=(const FVec2 &Other) const { return !(*this == Other); }
  float Length() const;
  float Length2() const;
  float Dot(const FVec2 &Other) const;
  FAngle GetAngle(const FVec2 &Other) const;
  FAngle GetAngle() const;
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

  vector<FVec2> GetCheckpoints() const { return Checkpoints; }

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

struct FGene {
  FGene(FVec2 Pt, FAngle Angle, float Dist2, bool Virtual = true)
      : Point(Pt), AngleToNextPoint(Angle), Dist2ToNextPoint(Dist2),
        bVirtual(Virtual){};

  FVec2 Point;
  FAngle AngleToNextPoint;
  float Dist2ToNextPoint;
  bool bVirtual;
};
typedef vector<FGene> FGenome;
typedef vector<FGenome> FGeneticPool;
typedef vector<FGeneticPool> FGeneticGenerations;

class FMapSolver : public ISolver {
public:
  virtual void Update(const FVec2 &PlayerPos, const FVec2 &EnemyPos,
                      const FVec2 &CPPos, const float CPAngle,
                      const float CPDist) override;
  virtual FMove Solve() override;

  FMapSolver(GameMap *GMap)
      : bBoostUsed(false), Map(GMap), bGeneticPoolSeeded(false){};

private:
  FMove Explore();
  FMove ComputeMoveFromMap();

  bool ShouldUseShield() const;
  bool ShouldUseBoost() const;

  void SeedGeneticMap();
  void ComputeGeneticMap();

protected:
  GameMap *Map;
  FVec2 PlayerPos;
  FVec2 PrevPlayerPos;
  FVec2 EnemyPos;
  FVec2 PrevEnemyPos;
  FVec2 CPPos;
  float CPAngle;
  float CPDist;
  bool bBoostUsed;

  bool bGeneticPoolSeeded;
  FGenome GeneticMap;
  FGenome GeneticSeed;
  FGeneticGenerations Genenerations;
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
FVec2 FVec2::operator/(const float Value) const {
  FVec2 Result;
  Result.X = X / Value;
  Result.Y = Y / Value;
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

FAngle FVec2::GetAngle() const { return FAngle::FromRad(atan2(Y, X)); }

FVec2 FVec2::Rotate(const FAngle Angle) const {
  FVec2 Result;

  Result.X = X * Angle.Cos() - Y * Angle.Sin();
  Result.Y = X * Angle.Sin() + Y * Angle.Cos();

  return Result;
}

FVec2 FVec2::Normalise() const {
  FVec2 Norm;
  float l = Length();
  Norm.X = X / l;
  Norm.Y = Y / l;
  return Norm;
}

/// GameMap ==============================================
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
  PrevPlayerPos = PlayerPos;
  PlayerPos = InPlayerPos;
  PrevEnemyPos = EnemyPos;
  EnemyPos = InEnemyPos;
  CPPos = InCPPos;
  CPDist = InCPDist;
  CPAngle = InCPAngle;

  Map->UpdateCheckpoint(CPPos);
};

FMove FMapSolver::Solve() {
  if (Map->PassFirstLaps()) {
    ComputeGeneticMap();
  }

  return Explore();
}

FMove FMapSolver::Explore() {
  FMove Move;

  Move.bUseShield = ShouldUseShield();

  const float kMaxSteeringAngle = 18.f;
  if (abs(CPAngle) >= kMaxSteeringAngle) {
    FVec2 DesiredDirection = (CPPos - PlayerPos).Normalise();
    FVec2 CurrentDir =
        DesiredDirection.Rotate(FAngle::FromDeg(-CPAngle)).Normalise();
    FAngle SteeringAngle =
        FAngle::FromDeg(clamp(CPAngle, -kMaxSteeringAngle, kMaxSteeringAngle));
    FVec2 TargetDirection = CurrentDir.Rotate(SteeringAngle).Normalise();
    Move.Target = PlayerPos + TargetDirection * 100;
  } else {
    Move.Target = CPPos;
  }
  if (ShouldUseBoost()) {
    Move.bUseBoost = true;
    bBoostUsed = true;
  }

  const int KMaxThrust = 100;
  if (abs(CPAngle) < 1) {
    Move.Thrust = KMaxThrust;
  } else if (abs(CPAngle) >= 90.f) {
    Move.Thrust = 0;
  } else {
    float DistCoef = 1.0f; // clamp(CPDist / (600.f *4), 0.f, 1.f);
    float AngleCoef = 1.f - clamp(CPAngle / 90.f, 0.f, 1.f);
    Move.Thrust = round(KMaxThrust * DistCoef * AngleCoef);
  }

  cerr << Move << endl;
  return Move;
}
FMove FMapSolver::ComputeMoveFromMap() {

  FMove Result;
  Result.Target = CPPos;
  Result.Thrust = 100.f;
  return Result;
}

bool FMapSolver::ShouldUseShield() const {
  if (PrevEnemyPos != FVec2()) {
    FVec2 EnemyVel = EnemyPos - PrevEnemyPos;
    FVec2 FutureEnemyPos = EnemyPos + EnemyVel * 0.85f;

    FVec2 PlayerVel = PlayerPos - PrevPlayerPos;
    FVec2 FuturePlayerPos = PlayerPos + PlayerVel * 0.85;

    const float kPodCollBox = 400.0f * 2; // We seek coll between bbox
    FVec2 PlayerToEnemy = FutureEnemyPos - FuturePlayerPos;
    float Dist2PtoE = PlayerToEnemy.Length2();
    bool bIsInRange = Dist2PtoE <= (kPodCollBox * kPodCollBox);

    return bIsInRange;
  }
  return false;
}
bool FMapSolver::ShouldUseBoost() const {
  const float kMaxAngle = 2.0f;
  return !bBoostUsed && Map->PassFirstLaps() && CPDist > 7000 &&
         abs(CPAngle) < kMaxAngle &&
         Map->IsNextCheckpointBestCandidateForBoost(CPPos);
}

void FMapSolver::SeedGeneticMap() {
  if (bGeneticPoolSeeded)
    return;
  auto &&CPs = Map->GetCheckpoints();
  for (int i = 0; i < CPs.size(); ++i) {
    int j = i + 1;
    if (j >= CPs.size())
      j = 0;
    int k = i - 1;
    if (k < 0)
      k = CPs.size() - 1;
    FVec2 PrevSeg = CPs[i] - CPs[k];
    FVec2 NextSeg = CPs[j] - CPs[i];
    GeneticSeed.emplace_back(FGene(CPs[i], PrevSeg.GetAngle(NextSeg),
                                   (CPs[j] - CPs[i]).Length2(), false));
  }
  // Generate 6 candidates from the seed
  const int kCandidates = 6;
  for (int cidx = 0; cidx < kCandidates; ++cidx) {
    FGenome Candidate;
    for (int i = 0; i < GeneticSeed.size(); ++i) {
      Candidate.emplace_back(GeneticSeed[i]);
      if (GeneticSeed[i].Dist2ToNextPoint < 800.f * 800.f)
        continue;
      int j = i + 1;
      if (j >= CPs.size())
        j = 0;

      int k = j + 1;
      if (k >= GeneticSeed.size())
        k = 0;

      FVec2 CurrSeg = GeneticSeed[j].Point - GeneticSeed[i].Point;
      FVec2 NextSeg = GeneticSeed[k].Point - GeneticSeed[j].Point;

      // cut segment in two
      FVec2 NewSeg = CurrSeg / 2.0f;
      // rotate new segment between [-18;18]
      int A = 0;
      while (A == 0)
        A = (rand() % 36) - 18;
      FAngle Angle = FAngle::FromDeg(A);

      NewSeg.Rotate(Angle);
      FVec2 NewPoint = GeneticSeed[i].Point + NewSeg;
      FVec2 NNewSeg = GeneticSeed[j].Point - NewPoint;

      Candidate.emplace_back(
          FGene(NewPoint, NNewSeg.GetAngle(NextSeg), NNewSeg.Length2()));
      Candidate[i].AngleToNextPoint = Angle;
    }
    //Pool.emplace_back(Candidate);
  }
  bGeneticPoolSeeded = true;
}

void FMapSolver::ComputeGeneticMap() {

  cerr << "Compute genetic map\n";
  SeedGeneticMap();
  // Mutate Genetic poool;
  
}

void PrintMove(const FMove &Move) {
  cout << (int)Move.Target.X << " " << (int)Move.Target.Y << " ";
  if (Move.bUseBoost) {
    cout << "BOOST" << endl;
  } else if (Move.bUseShield) {
    cout << "SHIELD" << endl;
  } else {
    cout << Move.Thrust << endl;
  }
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

    PrintMove(MapSolver.Solve());
  }
}

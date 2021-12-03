# CSB-CodingGame

Code for the Coder Strike Back on cding game.

## Codingame
https://www.codingame.com/profile/be513aec591129dc3c40c9762bf58bc8809969
Username: mnivoliez
Silver 2947 / 29157

Currently playing: commit e306b08c818a0ebef12b899e95f126f99f57fc27, https://github.com/mnivoliez/CSB-CodingGame/commit/e306b08c818a0ebef12b899e95f126f99f57fc27

## Development

Total time spent: ~16h

### Creation of base math structures

FVec2 : 1h
FAngle: 30 min

### Prediction

Trying to anticipate the checkpoint to reduce speed according to distance and angle: 3h (including fine tuning)
Trying to anticipate collision to activate shield: 2h

### Exploration

Keep a list of all checkpoint: 2h
Trying to tune speed and angle according to the n and n+1 checkpoints : 2h

### Genetic map

I tried to determine an optimal course after the first lap using an genetic algorythm, without success : 3h
It is visible in the genetic-map branch of the repo: https://github.com/mnivoliez/CSB-CodingGame/tree/genetic-map

### Other
Trying to counter inertia: 1h
Refactor: 1h

## Current state

The pod will try to adjust it course to aime for the next checkpoint. If it should meet the enemy's pod next turn, we use shield. 
We use boost after first turn on the longest segment.

## Difficulties

As I do not know how the speed is calculated from the thrust, I do not know how to determine my next position.
I tried using a genetic algorythm as it is suggested by the challenge presentation page. I have never done that before and as it seems like the right call, it is difficult to learn to do it quickly.

15 hours is really short.
